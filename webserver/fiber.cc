#include "fiber.h"
#include "macro.h"
#include "config.h"
#include <atomic>
#include "scheduler.h"

namespace server_name {
static std::atomic<uint64_t> s_fiber_id {0};
static std::atomic<uint64_t> s_fiber_count {0};

// 指向当前运行的协程
static thread_local Fiber *t_fiber = nullptr;
// 指向主协程
static thread_local Fiber::ptr t_threadFiber = nullptr;

static Logger::ptr g_logger = SYLAR_LOG_NAME("system");

static ConfigVar<uint32_t>::ptr g_fiber_stack_size =
    Config::Lookup<uint32_t>("fiber.stack_size", 1024 * 1024, "fiber stack size");

class MallocStackAllocator{
public:
    static void *Alloc(size_t size) {
        return malloc(size);
    }

    static void Dealloc(void *vp, size_t size) {
        free(vp);
    }
};

typedef MallocStackAllocator StackAllocator;

Fiber::Fiber() {
    m_state = EXEC;
    SetThis(this);

    if(getcontext(&m_ctx)) {
        SYLAR_ASSERT2(false, "getcontext");
    }

    ++s_fiber_count;

    SYLAR_LOG_DEBUG(g_logger) << "Fiber() construct main";
}

Fiber::Fiber(std::function<void()> cb, size_t stacksize, bool use_caller)
    :m_id(++s_fiber_id)
    ,m_cb(cb) {
    ++s_fiber_count;
    m_stacksize = stacksize ? stacksize : g_fiber_stack_size->getValue();
    m_stack = StackAllocator::Alloc(m_stacksize);
    if(getcontext(&m_ctx)) {
        SYLAR_ASSERT2(false, "getcontext");
    }
    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;

    if(!use_caller) {
        makecontext(&m_ctx, &Fiber::MainFunc, 0);
    } else {
        makecontext(&m_ctx, &Fiber::CallMainFunc, 0);
    }

    SYLAR_LOG_DEBUG(g_logger) << "Fiber(cb) construct main id= " << m_id;
}

Fiber::~Fiber() {
    --s_fiber_count;
    if(m_stack) {
        SYLAR_ASSERT(m_state == TERM
                || m_state == INIT
                || m_state == EXECPT);
        StackAllocator::Dealloc(m_stack, m_stacksize);
    } else {
        SYLAR_ASSERT(!m_cb);
        SYLAR_ASSERT(m_state == EXEC);

        Fiber * cur = t_fiber;
        if(cur == this) {
            SetThis(nullptr);
        }
    }
    SYLAR_LOG_DEBUG(g_logger) << "~Fiber() id= " << m_id;
}

//重置协程函数，并重置其状态
//INIT, TERM
void Fiber::reset(std::function<void()> cb) {
    SYLAR_ASSERT(!m_cb);
    SYLAR_ASSERT(m_state == INIT
            || m_state == TERM
            || m_state == EXECPT);
    m_cb = cb;
    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_size = m_stacksize;
    m_ctx.uc_stack.ss_sp = m_stack;

    makecontext(&m_ctx, &Fiber::MainFunc, 0);
    m_state = INIT;
}

uint64_t Fiber::GetFiberId() {
    if(t_fiber){
        return t_fiber->getId();
    }
    return 0;
}

void Fiber::back() {
    SetThis(t_threadFiber.get());
    if(swapcontext(&m_ctx, &t_threadFiber->m_ctx)) {
        SYLAR_ASSERT2(false, "swapcontext");
    }
}

void Fiber::call() {
    SetThis(this);
    m_state = EXEC;
    if(swapcontext(&t_threadFiber->m_ctx, &m_ctx)) {
        SYLAR_ASSERT2(false, "swapcontext");
    }
}

//从当前主协程切换到当前协程去执行
void Fiber::swapIn() {
    SetThis(this);
    SYLAR_ASSERT(m_state != EXEC);
    m_state = EXEC;
    if(swapcontext(&Scheduler::GetMainFiber()->m_ctx, &m_ctx)) {
        SYLAR_ASSERT2(false, "swapcontext");
    }
}

//当前协程切换到当前的主协程
void Fiber::swapOut() {
    // 让t_fiber指向主协程
    SetThis(Scheduler::GetMainFiber());

    if(swapcontext(&m_ctx, &Scheduler::GetMainFiber()->m_ctx)) {
        SYLAR_ASSERT2(false, "swapcontext");
    }
    //std::weak_ptr<Fiber> cur = GetThis();
    //std::cout << "swapOut cur_shared_ptr count= " << cur.use_count() << std::endl;
}

//设置t_fiber指向当前的协程
void Fiber::SetThis(Fiber *f) {
    t_fiber = f;
}
//返回当前协程
Fiber::ptr Fiber::GetThis() {
    // 记住每一个线程的t_fiber都会重新初始化
   if(t_fiber) {
       return t_fiber->shared_from_this();
   }
   Fiber::ptr main_fiber(new Fiber);
   SYLAR_ASSERT(t_fiber == main_fiber.get());
   t_threadFiber = main_fiber;
   return t_fiber->shared_from_this();
}

//协程切换到后台，并设置为Ready状态
void Fiber::YieldToReady() {
    Fiber::ptr cur = GetThis();
    cur->m_state = READY;
    cur->swapOut();
}

//协程切换到后台，并设置为Hold状态
void Fiber::YieldToHold() {
    Fiber::ptr cur = GetThis();
    //cur->m_state = HOLD;
    cur->swapOut();
//    t_fiber->m_state = HOLD;
//    t_fiber->swapOut();
}

//返回总的协程数
uint64_t Fiber::TotalToHold() {
    return s_fiber_count;
}

void Fiber::MainFunc() {
    Fiber::ptr cur = GetThis();
    //std::weak_ptr<Fiber> cur = GetThis();
    SYLAR_ASSERT(cur);
    try{
        cur->m_cb();
        cur->m_cb = nullptr;
        cur->m_state = TERM;
    } catch(std::exception &ex) {
        cur->m_state = EXECPT;
        SYLAR_LOG_ERROR(g_logger) << "Fiber Exception: " << ex.what();
    } catch(...) {
        cur->m_state = EXECPT;
        SYLAR_LOG_ERROR(g_logger) << "Fiber Exception: ";
    }
    //std::weak_ptr<Fiber> raw_ptr = cur;
    auto raw_ptr = cur.get();  //获取原始对象
    cur.reset();
    raw_ptr->swapOut();

    SYLAR_ASSERT2(false, "nerver reach");
}

void Fiber::CallMainFunc() {
    Fiber::ptr cur = GetThis();
    //std::weak_ptr<Fiber> cur = GetThis();
    SYLAR_LOG_INFO(g_logger) << "callmainfunc";
    SYLAR_ASSERT(cur);
    try{
        cur->m_cb();
        cur->m_cb = nullptr;
        cur->m_state = TERM;
    } catch(std::exception &ex) {
        cur->m_state = EXECPT;
        SYLAR_LOG_ERROR(g_logger) << "Fiber Exception: " << ex.what();
    } catch(...) {
        cur->m_state = EXECPT;
        SYLAR_LOG_ERROR(g_logger) << "Fiber Exception: ";
    }
    //std::weak_ptr<Fiber> raw_ptr = cur;
    auto raw_ptr = cur.get();  //获取原始对象
    cur.reset();
    raw_ptr->back();

    SYLAR_ASSERT2(false, "nerver reach");
}


}
