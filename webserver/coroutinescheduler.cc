#include "coroutinescheduler.h"
#include "log.h"
#include "assert.h"
#include "thread.h"
#include <vector>
#include <functional>
#include "hook.h"

namespace server_name {

static server_name::Logger::ptr g_logger = WEBSERVER_LOG_NAME("system");

/// t_scheduler指向调度器　调度器只有一个
static thread_local Scheduler* t_scheduler = nullptr;
static thread_local Coroutine* t_scheduler_coroutine = nullptr;

Scheduler::Scheduler(size_t threads, bool use_caller, const std::string& name)
    :m_name(name) {
    WEBSERVER_ASSERT(threads > 0);

    if(use_caller) {
        server_name::Coroutine::GetThis();
        --threads;

        WEBSERVER_ASSERT(GetThis() == nullptr);
        t_scheduler = this;

        //这个时候　跑run方法的这个协程变成了调度函数的主协程
        m_rootcoroutine.reset(new Coroutine(std::bind(&Scheduler::run, this), 0, true));
        server_name::Thread::SetName(m_name);

        t_scheduler_coroutine = m_rootcoroutine.get();
        m_rootThread = server_name::GetThreadId();
        m_threadIds.push_back(m_rootThread);
    } else {
        m_rootThread = -1;
    }
    m_threadCount = threads;
}

Scheduler::~Scheduler() {
    WEBSERVER_ASSERT(m_stopping);
    if(GetThis() == this) {
        t_scheduler = nullptr;
    }
}

Scheduler* Scheduler::GetThis() {
    return t_scheduler;
}

Coroutine* Scheduler::GetMainCoroutine() {
    return t_scheduler_coroutine;
}

void Scheduler::start() {
    MutexType::Lock lock(m_mutex);
    if(!m_stopping) {
        return;
    }
    m_stopping = false;
    WEBSERVER_ASSERT(m_threads.empty());
//    std::cout << "thread create before" << " m_threadCount = " << m_threadCount << std::endl;
    m_threads.resize(m_threadCount);
    for(size_t i = 0; i < m_threadCount; ++i) {
        m_threads[i].reset(new Thread(std::bind(&Scheduler::run, this)
                    , m_name + "_" + std::to_string(i)));
        m_threadIds.push_back(m_threads[i]->getId());
    }
//    lock.unlock();
    //if(m_rootCoroutine) {
    //    m_rootCoroutine->call();
    //    std::cout << "m_rootCoroutine->swapIn() after" << std::endl;
    //}
}

void Scheduler::stop() {
    m_autoStop = true;
    if(m_rootcoroutine
        && m_threadCount == 0
        && (m_rootcoroutine->getState() == Coroutine::TERM
            || m_rootcoroutine->getState() == Coroutine::INIT)) {
            WEBSERVER_LOG_INFO(g_logger) << this << "stopped";
            m_stopping = true;

        if(stopping()) {
            return;
        }
    }

    if(m_rootThread != -1) {
        WEBSERVER_ASSERT(GetThis() == this);

    } else {
        WEBSERVER_ASSERT(GetThis() != this);
    }

    m_stopping = true;
    for(size_t i = 0; i < m_threadCount; ++i) {
        tickle();
    }

    if(m_rootcoroutine) {
        tickle();
    }

    if(m_rootcoroutine) {
        if(!stopping()) {
            m_rootcoroutine->call();
        }
    }

    std::vector<Thread::ptr> thrs;
    {
        MutexType::Lock lock(m_mutex);
        thrs.swap(m_threads);
    }

    for(auto &i : thrs) {
        i->join();
    }

}

void Scheduler::setThis() {
    t_scheduler = this;
}

void Scheduler::run() {
    set_hook_enable(true);
    setThis();
    // 这个判断必须有 GetThis()会创建一个主协程，这点非常关键
    if(server_name::GetThreadId() != m_rootThread) {
        t_scheduler_coroutine = Coroutine::GetThis().get();
    }

    // 当我们的调度线程都完成后,我们开始做idle
    Coroutine::ptr idle_coroutine(new Coroutine(std::bind(&Scheduler::idle, this)));
    Coroutine::ptr cb_coroutine;

    CoroutineAndThread ft;
    while(true) {
        ft.reset();
        bool tickle_me = false;
        bool is_active = false;
        {
            MutexType::Lock lock(m_mutex);
            auto it = m_coroutines.begin();
            while(it != m_coroutines.end()) {
                // 我们指定了一个线程id,当这个线程线程id不等于我们指定的线程id时，我们不做这个任务
                // 我们要通知其他线程，让它唤醒去做
                // 太棒了吧！赞
                if(it->thread != -1 && it->thread != server_name::GetThreadId()) {
                    ++it;
                    tickle_me = true;
                    continue;
                }

                WEBSERVER_ASSERT(it->coroutine || it->cb);
                if(it->coroutine && it->coroutine->getState() == Coroutine::EXEC) {
                    ++it;
                    continue;
                }

                ft = *it;
                m_coroutines.erase(it++);
                ++m_activeThreadCount;
                is_active = true;
                break;
            }
        }
        if(tickle_me) {
            std::cout << "<3>tickle_me = true" << std::endl;
            tickle();
        }

        if(ft.coroutine && (ft.coroutine->getState() != Coroutine::TERM
                || ft.coroutine->getState() != Coroutine::EXECPT)) {
            ft.coroutine->swapIn();
            --m_activeThreadCount;

            if(ft.coroutine->getState() == Coroutine::READY) {
                schedule(ft.coroutine);
            } else if(ft.coroutine->getState() != Coroutine::TERM
                    && ft.coroutine->getState() != Coroutine::EXECPT) {
                ft.coroutine->m_state = Coroutine::HOLD;
            }
            ft.reset();
        } else if(ft.cb) {
            if(cb_coroutine) {
                cb_coroutine->reset(ft.cb);
            } else {
                cb_coroutine.reset(new Coroutine(ft.cb));
            }
            ft.reset();
            cb_coroutine->swapIn();
            --m_activeThreadCount;

            if(cb_coroutine->getState() == Coroutine::READY) {
                schedule(cb_coroutine);
                cb_coroutine.reset();
            } else if(cb_coroutine->getState() == Coroutine::TERM
                    || cb_coroutine->getState() == Coroutine::EXECPT) {
                cb_coroutine->reset(nullptr);
            } else {
                cb_coroutine->m_state = Coroutine::HOLD;
                cb_coroutine.reset();
            }
        } else {
            if(is_active) {
                --m_activeThreadCount;
                continue;
            }
            if(idle_coroutine->getState() == Coroutine::TERM) {
                WEBSERVER_LOG_INFO(g_logger) << "idle_coroutine term";
                break;
            }
            ++m_idleThreadCount;
            idle_coroutine->swapIn();
            --m_idleThreadCount;
            if(idle_coroutine->getState() != Coroutine::TERM
                    && idle_coroutine->getState() != Coroutine::EXECPT) {
                idle_coroutine->m_state = Coroutine::HOLD;
            }
        }
    }
}

void Scheduler::tickle() {
    WEBSERVER_LOG_INFO(g_logger) << "tickle";
}

bool Scheduler::stopping() {
    MutexType::Lock lock(m_mutex);
    return m_autoStop && m_stopping
        && m_coroutines.empty() && m_activeThreadCount == 0;
}

void Scheduler::idle() {
    WEBSERVER_LOG_INFO(g_logger) << "idle";
    while(!stopping()) {
        server_name::Coroutine::YieldToHold();
    }
}



}
