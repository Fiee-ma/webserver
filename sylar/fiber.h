#ifndef __SYLAR_FIBER_H__
#define __SYLAR_FIBER_H__

#include <memory>
#include <ucontext.h>
#include <functional>
#include "thread.h"

namespace sylar {

class Scheduler;
class Fiber : public std::enable_shared_from_this<Fiber> {
friend class Scheduler;
public:
    typedef std::shared_ptr<Fiber> ptr;

    enum State {
        INIT,
        HOLD,
        EXEC,
        TERM,
        READY,
        EXECPT
    };

private:
    Fiber();

public:
    Fiber(std::function<void()> cb, size_t stacksize = 0, bool use_caller = false);
    ~Fiber();
    //重置协程函数，并重置其状态
    //INIT, TERM
    void reset(std::function<void()> cb);
    //切换到当前协程去执行
    void swapIn();
    //切换到后台执行
    void swapOut();
    uint64_t getId() const {return m_id;}
    State getState() const { return m_state;}
    void back();
    void call();

public:
    //设置当前协程
    static void SetThis(Fiber *f);
    //返回当前协程
    static Fiber::ptr GetThis();
    //协程切换到后台，并设置为Ready状态
    static void YieldToReady();
    //协程切换到后台，并设置为Hold状态
    static void YieldToHold();
    //返回总的协程数
    static uint64_t TotalToHold();

    //协程调用函数
    static void MainFunc();
    static void CallMainFunc();
    static uint64_t GetFiberId();

private:
    //协程id
    uint64_t m_id = 0;
    //协程栈的大小
    uint32_t m_stacksize = 0;
    //协程的状态
    State m_state = INIT;

    //ucontext_t是一个协程的结构体
        /*typedef struct ucontext_t
        {
            unsigned long int __ctx(uc_flags);
            struct ucontext_t *uc_link; //后继上下文
            stack_t uc_stack;           //该上下文中使用的栈
            mcontext_t uc_mcontext;
            sigset_t uc_sigmask;
            ostruct _libc_fpstate __fpregs_mem;
        } ucontext_t;*/
    ucontext_t m_ctx;
    //栈的内存空间
    void *m_stack = nullptr;

    //协程执行的方法
    std::function<void()> m_cb;

};


}


#endif
