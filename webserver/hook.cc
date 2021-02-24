#include <dlfcn.h>
#include <errno.h>
#include "fd_manager.h"
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "fileconfig.h"
#include "log.h"
#include "hook.h"
#include "coroutine.h"
#include "iomanager.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>


server_name::Logger::ptr g_logger = WEBSERVER_LOG_NAME("system");
namespace server_name {

static server_name::ConfigVar<int>::ptr g_tcp_connect_timeout =
    server_name::Config::Lookup("tcp.connect.timeout", 5000, "tcp connect timeout");

static thread_local bool t_hook_enable = false;

#define HOOK_FUN(XX) \
    XX(sleep) \
    XX(usleep) \
    XX(nanosleep) \
    XX(socket) \
    XX(connect) \
    XX(accept) \
    XX(read) \
    XX(readv) \
    XX(recv) \
    XX(recvfrom) \
    XX(recvmsg) \
    XX(write) \
    XX(writev) \
    XX(send) \
    XX(sendto) \
    XX(sendmsg) \
    XX(close) \
    XX(fcntl) \
    XX(ioctl) \
    XX(getsockopt) \
    XX(setsockopt)

void hook_init() {
    static bool is_inited = false;
    if(is_inited) {
        return;
    }
#define XX(name) name ## _f = (name ## _fun)dlsym(RTLD_NEXT, #name);
    HOOK_FUN(XX);
#undef XX
}

static uint64_t s_connect_timeout = -1;
struct _HookIniter {
    _HookIniter() {
        hook_init();
        s_connect_timeout = g_tcp_connect_timeout->getValue();

        g_tcp_connect_timeout->addListener([](const int &old_value, const int &new_value){
            WEBSERVER_LOG_INFO(g_logger) << "tcp connect timeout changed from " <<
                           old_value <<" " << new_value;
            s_connect_timeout = new_value;
        });
    }
};

static _HookIniter s_hook_initer;

bool is_hook_enable() {
    return t_hook_enable;
}

void set_hook_enable(bool flag) {
    t_hook_enable = flag;
}

}

struct timer_info {
    int canceled = 0;
};

template<typename OriginFun, typename ... Args>
static ssize_t do_io(int fd, OriginFun fun, const char *hook_fun_name,
        uint32_t event, int timeout_so, Args&&... args) {
    if(!server_name::t_hook_enable) {
        return fun(fd, std::forward<Args>(args)...);
    }

    server_name::FdCtx::ptr ctx = server_name::FdMgr::GetInstance()->get(fd);
    if(!ctx) {
        return fun(fd, std::forward<Args>(args)...);
    }

    if(ctx->isClose()) {
        errno = EBADF;
        return -1;
    }

    if(!ctx->isSocket() || ctx->getUserNonblock()) {
        return fun(fd, std::forward<Args>(args)...);
    }

    uint64_t to = ctx->getTimeout(timeout_so);
    std::shared_ptr<timer_info> tinfo(new timer_info);

retry:
    ssize_t n = fun(fd, std::forward<Args>(args)...);
    while(n == -1 && errno == EINTR) {
       n = fun(fd, std::forward<Args>(args)...);
    }

    if(n == -1 && errno == EAGAIN) {
        server_name::IOManager *iom = server_name::IOManager::GetThis();
        server_name::Timer::ptr timer;
        std::weak_ptr<timer_info> winfo(tinfo);

        if(to != (uint64_t)-1) {
                timer = iom->addConditionTimer(to, [winfo, fd, iom, event](){
                        auto t = winfo.lock();
                        if(!t || t->canceled) {
                            return;
                        }
                        t->canceled = ETIMEDOUT;
                        iom->cancelEvent(fd, (server_name::IOManager::Event)(event));
                }, winfo);
        }

        int rt = iom->addEvent(fd, (server_name::IOManager::Event)(event));
        if(rt) {
            WEBSERVER_LOG_ERROR(g_logger) << hook_fun_name << " addEvent("
                << fd << ", " << event << ")";
            if(timer) {
                timer->cancel();
            }
            return -1;
        } else {
            server_name::Coroutine::YieldToHold();   // 如果事件触发，或者定时器超时，都会从这返回
            if(timer) {
                timer->cancel();
            }
            if(tinfo->canceled) {
                errno = tinfo->canceled;
                return -1;
            }

            goto retry;
        }
    }
    return n;
}

extern "C" {
#define XX(name) name ## _fun name ## _f = nullptr;
    HOOK_FUN(XX)
#undef XX

unsigned int sleep(unsigned int seconds) {
    if(!server_name::t_hook_enable) {
        return sleep_f(seconds);
    }

    server_name::Coroutine::ptr coroutine = server_name::Coroutine::GetThis();
    server_name::IOManager *iom = server_name::IOManager::GetThis();
    iom->addTimer(seconds * 1000, std::bind((void(server_name::Scheduler::*)
                (server_name::Coroutine::ptr, int thread))&server_name::IOManager::schedule
                ,iom, coroutine, -1));

    server_name::Coroutine::YieldToHold();
    return 0;
}

int usleep(useconds_t usec) {
    if(!server_name::t_hook_enable) {
        return usleep_f(usec);
    }

    server_name::Coroutine::ptr coroutine = server_name::Coroutine::GetThis();
    server_name::IOManager *iom = server_name::IOManager::GetThis();
    iom->addTimer(usec / 1000, std::bind((void(server_name::Scheduler::*)
                (server_name::Coroutine::ptr, int thread))&server_name::IOManager::schedule
                ,iom, coroutine, -1));

    server_name::Coroutine::YieldToHold();
    return 0;
}

int nanosleep(const struct timespec *req, struct timespec *rem) {
    if(!server_name::t_hook_enable) {
        return nanosleep_f(req, rem);
    }

    int timeout_ms = req->tv_sec + req->tv_nsec / 1000 / 1000;
    server_name::Coroutine::ptr coroutine = server_name::Coroutine::GetThis();
    server_name::IOManager *iom = server_name::IOManager::GetThis();
    iom->addTimer(timeout_ms, std::bind((void(server_name::Scheduler::*)
                (server_name::Coroutine::ptr, int thread))&server_name::IOManager::schedule
                ,iom, coroutine, -1));

    server_name::Coroutine::YieldToHold();
    return 0;
}

int socket(int domain, int type, int protocol) {
    if(!server_name::t_hook_enable) {
        return socket_f(domain, type, protocol);
    }
    int fd = socket_f(domain, type, protocol);
    if(fd == -1) {
        return fd;
    }
    server_name::FdMgr::GetInstance()->get(fd, true); // 这一步把fd加入到FdMgr中
    return fd;
}

int connect_with_timeout(int sockfd, const struct sockaddr *addr, socklen_t addrlen, uint64_t timeout_ms) {
    if(!server_name::t_hook_enable) {
        return connect_f(sockfd, addr, addrlen);
    }
    server_name::FdCtx::ptr ctx = server_name::FdMgr::GetInstance()->get(sockfd);
    if(!ctx || ctx->isClose()) {
        errno = EBADF;
        return -1;
    }

    if(!ctx->isSocket()) {
        return connect_f(sockfd, addr, addrlen);
    }

    if(ctx->getUserNonblock()) {
        return connect_f(sockfd, addr, addrlen);
    }

    sockaddr_in addr1;
    memset(&addr1, 0, sizeof(addr));
    addr1.sin_family = AF_INET;
    addr1.sin_port = htons(80);
    //addr.sin_addr.s_addr = inet_addr("110.242.68.3");
    inet_pton(AF_INET, "110.242.68.3", &addr1.sin_addr.s_addr);

    //int n = connect_f(sockfd, (sockaddr*)&addr1, sizeof(addr1));
    int n = connect_f(sockfd, addr, addrlen);
    if(n == 0) {
        return 0;
    } else if(n != -1 || errno != EINPROGRESS) {
        return n;
    }

    WEBSERVER_LOG_DEBUG(g_logger) << "IOManager::GetThis() before";
    server_name::IOManager *iom = server_name::IOManager::GetThis();
    server_name::Timer::ptr timer;
    std::shared_ptr<timer_info> tinfo(new timer_info);
    std::weak_ptr<timer_info> winfo(tinfo);

    if(timeout_ms != (uint64_t)-1) {
        timer = iom->addConditionTimer(timeout_ms, [winfo, sockfd, iom](){
            auto t = winfo.lock();
            if(!t || t->canceled) {
                return;
            }
            t->canceled = ETIMEDOUT;
            iom->cancelEvent(sockfd, server_name::IOManager::WRITE);
        }, winfo);
    }

    int rt = iom->addEvent(sockfd, server_name::IOManager::WRITE);
    if(rt == 0) {
        server_name::Coroutine::YieldToHold();
        if(timer) {
            timer->cancel();
        }
        if(tinfo->canceled) {
            errno = tinfo->canceled;
            return -1;
        }
    } else {
        if(timer) {
            timer->cancel();
        }
        WEBSERVER_LOG_ERROR(g_logger) << "connect addEvent(" << sockfd  << ", WRITE) error";
    }

    int error = 0;
    socklen_t len = sizeof(int);
    if(getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len) == -1) {
        return -1;
    }
    if(!error) {
        return 0;
    } else {
        errno = error;
        return -1;
    }
}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    return connect_with_timeout(sockfd, addr, addrlen, server_name::s_connect_timeout);
}

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
    int fd = do_io(sockfd, accept_f, "accept", server_name::IOManager::READ, SO_RCVTIMEO, addr, addrlen);
    if(fd >= 0) {
        server_name::FdMgr::GetInstance()->get(fd, true);
    }
    return fd;
}

/// read
ssize_t read(int fd, void *buf, size_t count) {
    return do_io(fd, read_f, "read", server_name::IOManager::READ, SO_RCVTIMEO, buf, count);
}

ssize_t readv(int fd, const struct iovec *iov, int iovcnt) {
    return do_io(fd, readv_f, "readv", server_name::IOManager::READ, SO_RCVTIMEO,iov, iovcnt);
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags) {
    return do_io(sockfd, recv_f, "recv", server_name::IOManager::READ, SO_RCVTIMEO, buf, len, flags);
}

ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags,
                        struct sockaddr *src_addr, socklen_t *addrlen) {
    return do_io(sockfd, recvfrom_f, "recvfrom", server_name::IOManager::READ,
            SO_RCVTIMEO, buf, len, flags, src_addr, addrlen);
}

ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags) {
    return do_io(sockfd, recvmsg_f, "recvmsg", server_name::IOManager::READ,
            SO_RCVTIMEO, msg, flags);
}

/// write
ssize_t write(int fd, const void *buf, size_t count) {
    return do_io(fd, write_f, "write", server_name::IOManager::WRITE,
            SO_RCVTIMEO, buf, count);
}

ssize_t writev(int fd, const struct iovec *iov, int iovcnt) {
    return do_io(fd, writev_f, "writev", server_name::IOManager::WRITE,
            SO_RCVTIMEO, iov, iovcnt);
}

ssize_t send(int sockfd, const void *buf, size_t len, int flags) {
    return do_io(sockfd, send_f, "send", server_name::IOManager::WRITE,
            SO_RCVTIMEO, buf, len, flags);
}

ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
                       const struct sockaddr *dest_addr, socklen_t addrlen) {
    return do_io(sockfd, sendto_f, "sendto", server_name::IOManager::WRITE,
            SO_RCVTIMEO, buf, len, flags, dest_addr, addrlen);
}

ssize_t sendmsg(int sockfd, const struct msghdr *msg, int flags) {
    return do_io(sockfd, sendmsg_f, "sendmsg", server_name::IOManager::WRITE,
            SO_RCVTIMEO, msg, flags);
}

/// close
int close(int fd) {
    if(!server_name::t_hook_enable) {
        return close_f(fd);
    }

    server_name::FdCtx::ptr ctx = server_name::FdMgr::GetInstance()->get(fd);
    if(ctx) {
        auto iom = server_name::IOManager::GetThis();
        if(iom) {
            iom->cancelAll(fd);
        }
        server_name::FdMgr::GetInstance()->del(fd);
    }
    return close_f(fd);
}

int fcntl(int fd, int cmd, ... /* arg */ ) {
    va_list va;
    va_start(va, cmd);
    switch(cmd) {
        case F_SETFL:
            {
                int arg = va_arg(va, int);
                va_end(va);
                server_name::FdCtx::ptr ctx = server_name::FdMgr::GetInstance()->get(fd);
                if(!ctx || ctx->isClose() || !ctx->isSocket()) {
                    return fcntl_f(fd, cmd, arg);
                }
                ctx->setUserNonblock(arg & O_NONBLOCK);
                if(ctx->getSysNonblock()) {
                    arg |= O_NONBLOCK;
                } else {
                    arg &= ~O_NONBLOCK;
                }
                return fcntl_f(fd, cmd, arg);
            }
            break;
        case F_GETFL:
            {
                va_end(va);
                int arg = fcntl_f(fd, cmd);
                server_name::FdCtx::ptr ctx = server_name::FdMgr::GetInstance()->get(fd);
                if(!ctx || ctx->isClose() || !ctx->isSocket()) {
                    return arg;
                }
                if(ctx->getUserNonblock()) {
                    return arg | O_NONBLOCK;
                } else {
                    return arg & ~O_NONBLOCK;
                }
            }
            break;
        case F_DUPFD:
        case F_DUPFD_CLOEXEC:
        case F_SETFD:
        case F_SETOWN:
        case F_SETSIG:
        case F_SETLEASE:
        case F_NOTIFY:
#ifdef F_SETPIPE_SZ
        case F_SETPIPE_SZ:
#endif
            {
                int arg = va_arg(va, int);
                va_end(va);
                return fcntl_f(fd, cmd, arg);
            }
            break;
        case F_GETFD:
        case F_GETOWN:
        case F_GETSIG:
        case F_GETLEASE:
#ifdef F_GETPIPE_SZ
        case F_GETPIPE_SZ:
#endif
            {
                va_end(va);
                return fcntl_f(fd, cmd);
            }
            break;
        case F_SETLK:
        case F_SETLKW:
        case F_GETLK:
            {
                struct flock* arg = va_arg(va, struct flock*);
                va_end(va);
                return fcntl_f(fd, cmd, arg);
            }
            break;
        case F_GETOWN_EX:
        case F_SETOWN_EX:
            {
                struct f_owner_exlock* arg = va_arg(va, struct f_owner_exlock*);
                va_end(va);
                return fcntl_f(fd, cmd, arg);
            }
            break;
        default:
            va_end(va);
            return fcntl_f(fd, cmd);
    }
}

int ioctl(int fd, unsigned long request, ...) {
    va_list va;
    va_start(va, request);
    void *arg = va_arg(va, void*);
    va_end(va);

    if(request == FIONBIO) {
        bool user_nonblock = !!*(int *)arg;
        server_name::FdCtx::ptr ctx = server_name::FdMgr::GetInstance()->get(fd);
        if(!ctx || ctx->isClose() || !ctx->isSocket()) {
            return ioctl_f(fd, request, arg);
        }
        ctx->setUserNonblock(user_nonblock);
    }
    return ioctl_f(fd, request, arg);
}

int getsockopt(int sockfd, int level, int optname,
                      void *optval, socklen_t *optlen) {
    return getsockopt_f(sockfd, level, optname, optval, optlen);
}

int setsockopt(int sockfd, int level, int optname,
                      const void *optval, socklen_t optlen) {
    if(!server_name::t_hook_enable) {
        return setsockopt_f(sockfd, level, optname, optval, optlen);
    }
    if(level == SOL_SOCKET) {
        if(optname == SO_RCVTIMEO || optname == SO_SNDTIMEO) {
            server_name::FdCtx::ptr ctx = server_name::FdMgr::GetInstance()->get(sockfd);
            if(ctx) {
                const timeval *v = (const timeval *)optval;
                ctx->setTimeout(optname, v->tv_sec * 1000 + v->tv_usec /1000);
            }
        }
    }
    return setsockopt_f(sockfd, level, optname, optval, optlen);
}




}
















