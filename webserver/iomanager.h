#ifndef __SYLAR_IOMANAGER_H__
#define __SYLAR_IOMANAGER_H__

#include "scheduler.h"
#include "timer.h"

namespace server_name {

class IOManager : public Scheduler, public TimerManager {
public:
    typedef std::shared_ptr<IOManager> ptr;
    typedef RWMutex RWMutexType;

    enum Event {
        NONE   = 0x0,
        READ   = 0x1,   //EPOLLIN
        WRITE  = 0x4,   //EPOLLOUT
    };

private:
    struct FdContext {
        typedef Mutex MutexType;
        struct EventContext {
            Scheduler *scheduler = nullptr;  // 事件执行的scheduler
            Fiber::ptr fiber;                // 事件协程
            std::function<void()> cb;        // 事件回调函数
        };

        EventContext read;  // 读事件
        EventContext write; // 写事件
        int fd = 0;             //事件相关句柄
        Event events = NONE;  // 已经注册的事件
        MutexType mutex;

        /// 获取事件上下文类 event表示事件类型
        EventContext &getContext(Event event);
        /** 重置事件的上下文
         * ctx表示待重置上下文的类
         */
        void resetContext(EventContext & ctx);
        /** 触发事件
         * event表示事件类型
         */
        void triggerEvent(Event event);
    };

public:
    IOManager(size_t threads = 1, bool use_caller = true, const std::string &name = "");
    ~IOManager();

    //1 sucess , 0 retry, -1 error
    int addEvent(int fd, Event event, std::function<void()> cb = nullptr);
    bool delEvent(int fd, Event event);
    bool cancelEvent(int fd, Event event);

    bool cancelAll(int fd);

    static IOManager *GetThis();

protected:
    void tickle() override;
    bool stopping() override;
    void idle() override;
    void onTimerInsertedAtFront() override;

    /**
     * 重置socket描述符上下文容器的大小
     */
    void contextResize(size_t size);
    bool stopping(uint64_t &timeout);

private:
    /// epoll的文件描述符
    int m_epfd = 0;
    /// pipi的文件描述符
    int m_tickleFds[2];

    /// 当前等待执行的事件数量
    std::atomic<size_t> m_pendingEventCount = {0};
    /// IOManager的Mutex
    RWMutexType m_mutex;
    /// socket事件的上下文容器
    std::vector<FdContext *> m_fdContexts;
};


}

#endif
