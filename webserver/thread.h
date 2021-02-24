#ifndef __WEBSERVER_THREAD_H__
#define __WEBSERVER_THREAD_H__

#include <thread>
#include <functional>
#include <memory>
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <atomic>

#include "../webserver/noncopyable.h"

namespace server_name {

class Semaphore : Noncopyable{
public:
    Semaphore(uint32_t count = 0);
    ~Semaphore();

    void wait();
    void notify();
private:
    sem_t m_semaphore;
};

template<class T>
struct ScopeLockImpl {
public:
    ScopeLockImpl(T &mutex)
        :m_mutex(mutex) {
            m_mutex.lock();
            m_locked = true;
        }

    ~ScopeLockImpl() {
        unlock();
    }

    void lock() {
        if(!m_locked) {
            m_mutex.lock();
            m_locked = true;
        }
    }

    void unlock() {
        if(m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }

private:
    T &m_mutex;
    bool m_locked = false;
};

template<class T>
struct ReadScopeLockImpl {
public:
    ReadScopeLockImpl(T &mutex)
        :m_mutex(mutex) {
            m_mutex.rdlock();
            m_locked = true;
        }

    ~ReadScopeLockImpl() {
        unlock();
    }

    void lock() {
        if(!m_locked) {
            m_mutex.rdlock();
            m_locked = true;
        }
    }

    void unlock() {
        if(m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }

private:
    T &m_mutex;
    bool m_locked = false;
};

template<class T>
struct WriteScopeLockImpl {
public:
    WriteScopeLockImpl(T &mutex)
        :m_mutex(mutex) {
            m_mutex.wrlock();
            m_locked = true;
        }

    ~WriteScopeLockImpl() {
        unlock();
    }

    void lock() {
        if(!m_locked) {
            m_mutex.wrlock();
            m_locked = true;
        }
    }

    void unlock() {
        if(m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }

private:
    T &m_mutex;
    bool m_locked = false;
};



class Mutex : Noncopyable {
public:
    typedef ScopeLockImpl<Mutex> Lock;
    Mutex() {
        pthread_mutex_init(&m_lock, nullptr);
    }

    ~Mutex() {
        pthread_mutex_destroy(&m_lock);
    }

    void lock() {
        pthread_mutex_lock(&m_lock);
    }

    void unlock() {
        pthread_mutex_unlock(&m_lock);
    }

private:
    pthread_mutex_t m_lock;
};

class NullMutex {
public:
    typedef ScopeLockImpl<NullMutex> Lock;
    NullMutex() {}
    ~NullMutex() {}
    void lock() {}
    void unlock() {}
};

class RWMutex : Noncopyable {
public:
    typedef ReadScopeLockImpl<RWMutex> ReadLock;
    typedef WriteScopeLockImpl<RWMutex> WriteLock;

    RWMutex() {
        pthread_rwlock_init(&m_lock, nullptr);
    }

    ~RWMutex() {
        pthread_rwlock_destroy(&m_lock);
    }
    // 上读锁
    void rdlock() {
        pthread_rwlock_rdlock(&m_lock);
    }
    // 上写锁
    void wrlock() {
        pthread_rwlock_wrlock(&m_lock);
    }

    //解锁
    void unlock() {
        pthread_rwlock_unlock(&m_lock);
    }

private:
    pthread_rwlock_t m_lock;
};

class NullWRMutex : Noncopyable{
public:
    typedef ReadScopeLockImpl<NullWRMutex> ReadLock;
    typedef WriteScopeLockImpl<NullWRMutex> WriteLock;
    NullWRMutex() {}
    ~NullWRMutex() {}
    void rdlock() {}
    void wrlock() {}
    void unlock() {}
};

class SpinLock : Noncopyable {
public:
    typedef ScopeLockImpl<SpinLock> Lock;

    SpinLock() {
        pthread_spin_init(&m_mutex, 0);
    }

    ~SpinLock() {
        pthread_spin_destroy(&m_mutex);
    }

    void lock() {
        pthread_spin_lock(&m_mutex);
    }

    void unlock() {
        pthread_spin_unlock(&m_mutex);
    }


private:
    pthread_spinlock_t m_mutex;
};

class CASLock : Noncopyable {
public:
    typedef ScopeLockImpl<CASLock> Lock;

    CASLock() {
        m_mutex.clear();
    }

    ~CASLock() {

    }

    void lock() {
        while(std::atomic_flag_test_and_set_explicit(&m_mutex, std::memory_order_acquire));
    }

    void unlock() {
        std::atomic_flag_clear_explicit(&m_mutex, std::memory_order_release);
    }
private:
    volatile std::atomic_flag m_mutex;

};

class Thread : Noncopyable{
public:
    typedef std::shared_ptr<Thread> ptr;
    Thread(std::function<void()> cb, const std::string &name);
    ~Thread();

    pid_t getId() const {return m_id;}
    const std::string &getName() const { return m_name;}

    void join();

    static Thread *GetThis();
    static const std::string &GetName();
    static void SetName(const std::string &name);


private:
    /**
     * 线程执行函数
     */
    static void *run(void *arg);

private:
    /// 线程id
    pid_t m_id = -1;
    /// 线程结构
    pthread_t m_thread = 0;
    /// 线程执行函数
    std::function<void()> m_cb;
    /// 线程名称
    std::string m_name;
    /// 信号量
    Semaphore m_semaphore;

};

}

#endif
