#ifndef __WEBSERVER_COROUTINESCHEDULER_H__
#define __WEBSERVER_COROUTINESCHEDULER_H__

#include <memory>
#include "thread.h"
#include "coroutine.h"
#include "thread.h"
#include <vector>
#include <list>

namespace server_name {
/**
 * @brief协程调度器
 */
class Scheduler {
public:
    typedef std::shared_ptr<Scheduler> ptr;
    typedef Mutex MutexType;

    /**
     * threads表示线程数量
     * use_caller表示是否使用当前调度线程
     * name表示表示协程调度名称
     */
    Scheduler(size_t threads = 1, bool use_caller = true, const std::string &name = "");
    virtual ~Scheduler();
    /**
     * 返回协程调度器名称
     */
    const std::string &getName() const { return m_name;}
    /**
     * 返回当前的协程调度器
     */
    static Scheduler *GetThis();
    /**
     * 返回当前调度器的主协程
     */
    static Coroutine *GetMainCoroutine();
    /**
     * 启动当前的协程调度器
     */
    void start();
    /**
     * @brief暂停当前的协程调度器
     */
    void stop();
    /**
     * @brief 协程调度
     * fc表示协程或者函数
     * thread表示指定用什么协程去执行，-1表示任意线程
     */
    template<class CoroutineOrCb>
    void schedule(CoroutineOrCb fc, int thread = -1) {
        bool need_tickle = false;
        {
            MutexType::Lock lock(m_mutex);
            need_tickle = scheduleNoLock(fc, thread);
            if(need_tickle) {
                tickle();
            }
        }
    }

    template<class InputIterator>
    void schedule(InputIterator begin, InputIterator end) {
        bool need_tickle = false;
        {
            MutexType::Lock lock(m_mutex);
            while(begin != end){
                need_tickle = scheduleNoLock(&*begin, -1) || need_tickle;
                ++begin;
            }
            if(need_tickle){
                tickle();
            }
        }
    }

protected:
    /// 通知协程调度器有任务了
    virtual void tickle();
    /// 真正执行协程调度函数，调度的流程都在这个函数中执行
    void run();
    /// 返回是否可以停止
    virtual bool stopping();
    /// 设置当前协程调度器
    void setThis();
    /// 协程无任务了调度时调用idle函数
    virtual void idle();

    bool hasIdleThreads() const { return m_idleThreadCount > 0;}

private:
    template<class CoroutineOrCb>
    bool scheduleNoLock(CoroutineOrCb fc, int thread) {
        bool need_tickle = m_coroutines.empty();
        CoroutineAndThread ft(fc, thread);
        if(ft.coroutine || ft.cb) {
            m_coroutines.push_back(ft);
        }

        return need_tickle;
    }
private:
    struct CoroutineAndThread {
        Coroutine::ptr coroutine;
        std::function<void()> cb;
        int thread;

        CoroutineAndThread(Coroutine::ptr f, int thr)
            :coroutine(f), thread(thr){
        }
        CoroutineAndThread(Coroutine::ptr *f, int thr)
            :thread(thr){
                coroutine.swap(*f);
        }
        CoroutineAndThread(std::function<void()> f, int thr)
            :cb(f), thread(thr){
        }
        CoroutineAndThread(std::function<void()> *f, int thr)
            :thread(thr){
                cb.swap(*f);
        }

        CoroutineAndThread()
        :thread(-1){
        }

        void reset() {
            coroutine = nullptr;
            cb = nullptr;
            thread = -1;
        }

    };
private:
    MutexType m_mutex;
    /// 线程池
    std::vector<Thread::ptr> m_threads;
    /// 执行的协程队列
    std::list<CoroutineAndThread> m_coroutines;
    /// use_caller为true时有效，调度协程
    Coroutine::ptr m_rootcoroutine;
    /// 协程调度器名称
    std::string m_name;

protected:
    /// 协程下的线程id数组
    std::vector<int> m_threadIds;
    /// 线程数量
    size_t m_threadCount = 0;
    /// 工作线程数量
    std::atomic<size_t> m_activeThreadCount = {0};
    /// 空闲线程数量
    std::atomic<size_t> m_idleThreadCount = {0};
    /// 是否停止正在运行的协程调度器
    bool m_stopping = true;
    /// 是否自动停止正在运行的协程调度器
    bool m_autoStop = false;
    /// 主线程id(use_caller)
    int m_rootThread = 0;
};

}
#endif
