#ifndef __WEBSERVER_TIMER_R_H__
#define __WEBSERVER_TIMER_R_H__

#include <memory>
#include "thread.h"
#include <set>
#include <vector>

namespace server_name {

class TimerManager;
class Timer : public std::enable_shared_from_this<Timer> {
friend class TimerManager;
public:
    typedef std::shared_ptr<Timer> ptr;
    /**
     * 取消定时器
     */
    bool cancel();
    /**
     * 刷新设置定时器的执行时间
     */
    bool refresh();
    /**
     * 重置定时器时间
     * ms为定时器执行间隔时间（毫秒）
     * from_now是否从当前时间开始计算
     */
    bool reset(uint64_t ms, bool from_now);
private:
    Timer(uint64_t ms, std::function<void()> cb,
            bool recurring, TimerManager *manager);
    Timer(uint64_t next);
private:
    bool m_recurring = false;  // 是否安装定时器
    uint64_t m_ms = 0;         // 执行周期
    uint64_t m_next = 0;       // 精确执行时间
    std::function<void()> m_cb;//定时器的回调函数
    TimerManager * m_manager = nullptr;
private:
    struct Comparator {
        bool operator()(const Timer::ptr & lhs, const Timer::ptr &rhs) const;
    };
};

class TimerManager {
friend class Timer;
public:
    typedef RWMutex RWMutexType;

    TimerManager();
    virtual ~TimerManager();

    Timer::ptr addTimer(uint64_t ms, std::function<void()> cb
            , bool recurring = false);
    Timer::ptr addConditionTimer(uint64_t ms, std::function<void()> cb
            ,std::weak_ptr<void> weak_cond
            , bool recurring = false);
    uint64_t getNextTimer();
    void listExpiredCb(std::vector<std::function<void()> > &cbs);

protected:
    virtual void onTimerInsertedAtFront();
    void addTimer(Timer::ptr val, RWMutexType::WriteLock &lock);

private:
    bool detectClockRollover(uint64_t now_ms);

private:
    RWMutexType m_mutex;
    std::set<Timer::ptr, Timer::Comparator> m_timers;
    /// 是否触发onTimerInsertAtFront
    bool m_tickled = false;
    /// 上次执行的时间
    uint64_t m_previouseTime = 0;
};

}


#endif
