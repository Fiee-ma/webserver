#include "coroutine.h"
#include "assert.h"
#include "fileconfig.h"
#include <atomic>
#include "coroutinescheduler.h"

namespace server_name {
static std::atomic<uint64_t> s_coroutine_id {0};
static std::atomic<uint64_t> s_coroutine_count {0};

// 指向当前运行的协程
static thread_local Coroutine *t_coroutine = nullptr;
// 指向主协程
static thread_local Coroutine::ptr t_threadcoroutine = nullptr;

static Logger::ptr g_logger = WEBSERVER_LOG_NAME("system");

static ConfigVar<uint32_t>::ptr g_coroutine_stack_size =
    Config::Lookup<uint32_t>("coroutine.stack_size", 1024 * 1024, "coroutine stack size");

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

Coroutine::Coroutine() {
    m_state = EXEC;
    SetThis(this);

    if(getcontext(&m_ctx)) {
        WEBSERVER_ASSERT2(false, "getcontext");
    }

    ++s_coroutine_count;

    WEBSERVER_LOG_DEBUG(g_logger) << "Coroutine() construct main";
}

Coroutine::Coroutine(std::function<void()> cb, size_t stacksize, bool use_caller)
    :m_id(++s_coroutine_id)
    ,m_cb(cb) {
    ++s_coroutine_count;
    m_stacksize = stacksize ? stacksize : g_coroutine_stack_size->getValue();
    m_stack = StackAllocator::Alloc(m_stacksize);
    if(getcontext(&m_ctx)) {
        WEBSERVER_ASSERT2(false, "getcontext");
    }
    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;

    if(!use_caller) {
        makecontext(&m_ctx, &Coroutine::MainFunc, 0);
    } else {
        makecontext(&m_ctx, &Coroutine::CallMainFunc, 0);
    }

    WEBSERVER_LOG_DEBUG(g_logger) << "Coroutine(cb) construct main id= " << m_id;
}

Coroutine::~Coroutine() {
    --s_coroutine_count;
    if(m_stack) {
        WEBSERVER_ASSERT(m_state == TERM
                || m_state == INIT
                || m_state == EXECPT);
        StackAllocator::Dealloc(m_stack, m_stacksize);
    } else {
        WEBSERVER_ASSERT(!m_cb);
        WEBSERVER_ASSERT(m_state == EXEC);

        Coroutine * cur = t_coroutine;
        if(cur == this) {
            SetThis(nullptr);
        }
    }
    WEBSERVER_LOG_DEBUG(g_logger) << "~Coroutine() id= " << m_id;
}

//重置协程函数，并重置其状态
//INIT, TERM
void Coroutine::reset(std::function<void()> cb) {
    WEBSERVER_ASSERT(!m_cb);
    WEBSERVER_ASSERT(m_state == INIT
            || m_state == TERM
            || m_state == EXECPT);
    m_cb = cb;
    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_size = m_stacksize;
    m_ctx.uc_stack.ss_sp = m_stack;

    makecontext(&m_ctx, &Coroutine::MainFunc, 0);
    m_state = INIT;
}

uint64_t Coroutine::GetCoroutineId() {
    if(t_coroutine){
        return t_coroutine->getId();
    }
    return 0;
}

void Coroutine::back() {
    SetThis(t_threadcoroutine.get());
    if(swapcontext(&m_ctx, &t_threadcoroutine->m_ctx)) {
        WEBSERVER_ASSERT2(false, "swapcontext");
    }
}

void Coroutine::call() {
    SetThis(this);
    m_state = EXEC;
    if(swapcontext(&t_threadcoroutine->m_ctx, &m_ctx)) {
        WEBSERVER_ASSERT2(false, "swapcontext");
    }
}

//从当前主协程切换到当前协程去执行
void Coroutine::swapIn() {
    SetThis(this);
    WEBSERVER_ASSERT(m_state != EXEC);
    m_state = EXEC;
    if(swapcontext(&Scheduler::GetMainCoroutine()->m_ctx, &m_ctx)) {
        WEBSERVER_ASSERT2(false, "swapcontext");
    }
}

//当前协程切换到当前的主协程
void Coroutine::swapOut() {
    // 让t_Coroutine指向主协程
    SetThis(Scheduler::GetMainCoroutine());

    if(swapcontext(&m_ctx, &Scheduler::GetMainCoroutine()->m_ctx)) {
        WEBSERVER_ASSERT2(false, "swapcontext");
    }
    //std::weak_ptr<Coroutine> cur = GetThis();
    //std::cout << "swapOut cur_shared_ptr count= " << cur.use_count() << std::endl;
}

//设置t_Coroutine指向当前的协程
void Coroutine::SetThis(Coroutine *f) {
    t_coroutine = f;
}
//返回当前协程
Coroutine::ptr Coroutine::GetThis() {
    // 记住每一个线程的t_Coroutine都会重新初始化
   if(t_coroutine) {
       return t_coroutine->shared_from_this();
   }
   Coroutine::ptr main_coroutine(new Coroutine);
   WEBSERVER_ASSERT(t_coroutine == main_coroutine.get());
   t_threadcoroutine = main_coroutine;
   return t_coroutine->shared_from_this();
}

//协程切换到后台，并设置为Ready状态
void Coroutine::YieldToReady() {
    Coroutine::ptr cur = GetThis();
    cur->m_state = READY;
    cur->swapOut();
}

//协程切换到后台，并设置为Hold状态
void Coroutine::YieldToHold() {
    Coroutine::ptr cur = GetThis();
    //cur->m_state = HOLD;
    cur->swapOut();
//    t_Coroutine->m_state = HOLD;
//    t_Coroutine->swapOut();
}

//返回总的协程数
uint64_t Coroutine::TotalToHold() {
    return s_coroutine_count;
}

void Coroutine::MainFunc() {
    Coroutine::ptr cur = GetThis();
    //std::weak_ptr<Coroutine> cur = GetThis();
    WEBSERVER_ASSERT(cur);
    try{
        cur->m_cb();
        cur->m_cb = nullptr;
        cur->m_state = TERM;
    } catch(std::exception &ex) {
        cur->m_state = EXECPT;
        WEBSERVER_LOG_ERROR(g_logger) << "Coroutine Exception: " << ex.what();
    } catch(...) {
        cur->m_state = EXECPT;
        WEBSERVER_LOG_ERROR(g_logger) << "Coroutine Exception: ";
    }
    //std::weak_ptr<Coroutine> raw_ptr = cur;
    auto raw_ptr = cur.get();  //获取原始对象
    cur.reset();
    raw_ptr->swapOut();

    WEBSERVER_ASSERT2(false, "nerver reach");
}

void Coroutine::CallMainFunc() {
    Coroutine::ptr cur = GetThis();
    //std::weak_ptr<Coroutine> cur = GetThis();
    WEBSERVER_LOG_INFO(g_logger) << "callmainfunc";
    WEBSERVER_ASSERT(cur);
    try{
        cur->m_cb();
        cur->m_cb = nullptr;
        cur->m_state = TERM;
    } catch(std::exception &ex) {
        cur->m_state = EXECPT;
        WEBSERVER_LOG_ERROR(g_logger) << "Coroutine Exception: " << ex.what();
    } catch(...) {
        cur->m_state = EXECPT;
        WEBSERVER_LOG_ERROR(g_logger) << "Coroutine Exception: ";
    }
    //std::weak_ptr<Coroutine> raw_ptr = cur;
    auto raw_ptr = cur.get();  //获取原始对象
    cur.reset();
    raw_ptr->back();

    WEBSERVER_ASSERT2(false, "nerver reach");
}


}
