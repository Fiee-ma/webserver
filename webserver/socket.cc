#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <iostream>
#include "socket.h"
#include "fd_manager.h"
#include "hook.h"
#include "log.h"
#include "assert.h"
#include "fd_manager.h"
#include "iomanager.h"

namespace server_name {

static Logger::ptr g_logger = WEBSERVER_LOG_NAME("system");

Socket::ptr Socket::CreateTCP(server_name::Address::ptr address) {
    Socket::ptr sock(new Socket(address->getFamily(), TCP, 0));
    return sock;
}

Socket::ptr Socket::CreateUDP(server_name::Address::ptr address) {
    Socket::ptr sock(new Socket(address->getFamily(), UDP, 0));
    sock->newSock();
    sock->m_isConnected = true;
    return sock;
}

Socket::ptr Socket::CreateTCPSocket() {
    Socket::ptr sock(new Socket(IPv4, TCP, 0));
    return sock;
}

Socket::ptr Socket::CreateUDPSocket() {
    Socket::ptr sock(new Socket(IPv4, UDP, 0));
    sock->newSock();
    sock->m_isConnected = true;
    return sock;
}

Socket::ptr Socket::CreateTCPSocket6() {
    Socket::ptr sock(new Socket(IPv6, TCP, 0));
    return sock;
}

Socket::ptr Socket::CreateUDPSocket6() {
    Socket::ptr sock(new Socket(IPv6, UDP, 0));
    sock->newSock();
    sock->m_isConnected = true;
    return sock;
}

Socket::ptr Socket::CreateUnixTCPSocket() {
    Socket::ptr sock(new Socket(UNIX, TCP, 0));
    return sock;
}

Socket::ptr Socket::CreateUnixUDPSocket() {
    Socket::ptr sock(new Socket(UNIX, UDP, 0));
    sock->newSock();
    sock->m_isConnected = true;
    return sock;
}

Socket::Socket(int family, int type, int protocol)
    :m_sock(-1)
    ,m_family(family)
    ,m_type(type)
    ,m_protocol(protocol)
    ,m_isConnected(false){
}

Socket::~Socket() {
    close();
}

int64_t Socket::getSendTimeout() {
    FdCtx::ptr ctx = FdMgr::GetInstance()->get(m_sock);
    if(ctx) {
        return ctx->getTimeout(SO_SNDTIMEO);
    }
    return -1;
}

void Socket::setSendTimeout(int64_t v) {
    struct timeval tv{int(v / 1000), int(v % 1000 * 1000)};
    setOption(SOL_SOCKET, SO_SNDTIMEO, tv);
}

int Socket::getRecvTimeout() {
    FdCtx::ptr ctx = FdMgr::GetInstance()->get(m_sock);
    if(ctx) {
        return ctx->getTimeout(SO_RCVTIMEO);
    }
    return -1;
}

void Socket::setRecvTimeout(int64_t v) {
    struct timeval tv{int(v / 1000), int(v % 1000 * 1000)};
    setOption(SOL_SOCKET, SO_RCVTIMEO, tv);
}

bool Socket::getOption(int level, int option, void *result, socklen_t *len) {
    int rt = getsockopt(m_sock, level, option, result, (socklen_t*)len);
    if(rt) {
        WEBSERVER_LOG_DEBUG(g_logger) << "getOption sock=" << m_sock
            << " level=" << level << " option=" << option
            << " errno=" << errno << " errstr=" << strerror(errno);
        return false;
    }
    return true;
}

bool Socket::setOption(int level, int option, void *result, socklen_t len) {
    int rt = setsockopt(m_sock, level, option, result, (socklen_t)len);
    if(rt) {
        WEBSERVER_LOG_DEBUG(g_logger) << "setOption sock=" << m_sock
            << " level=" << level << " option=" << option
            << " errno=" << errno << " errstr=" << strerror(errno);
        return false;
    }
    return true;
}

Socket::ptr Socket::accept() {
    Socket::ptr sock(new Socket(m_family, m_type, m_protocol));
    int newsock = ::accept(m_sock, nullptr, nullptr);
    if(newsock == -1) {
        WEBSERVER_LOG_ERROR(g_logger) << "accept(" << m_sock << ") errno="
            << errno << " strerror=" << strerror(errno);
        return nullptr;
    }
    if(sock->init(newsock)) {
        return sock;
    }
    return nullptr;
}

bool Socket::init(int sock) {
    FdCtx::ptr ctx = FdMgr::GetInstance()->get(sock);
    if(ctx && ctx->isSocket() && !ctx->isClose()) {
        m_sock = sock;
        m_isConnected = true;
        initSock();
        getLocalAddress();
        getRemoteAddress();
        return true;
    }
    return false;
}

bool Socket::bind(const Address::ptr addr) {
    if(!isValid()) {
        newSock();
        if(WEBSERVER_UNLIKELY(!isValid())) {
            return false;
        }
    }

    if(WEBSERVER_UNLIKELY(addr->getFamily() != m_family)) {
        WEBSERVER_LOG_ERROR(g_logger) << "bind sock.family("
            << m_family << ") addr.family(" << addr->getFamily()
            << ") not equal, addr=" << addr->toString();
        return false;
    }

    if(::bind(m_sock, addr->getAddr(), addr->getAddrLen())) {
        WEBSERVER_LOG_ERROR(g_logger) << "bind error errno=" << errno
            << " strerr=" << strerror(errno);
        return false;
    }
    getLocalAddress();
    return true;
}

bool Socket::connect(const Address::ptr addr, uint64_t timeout_ms) {
    m_remoteAddress = addr;
    if(!isValid()) {
        newSock();
        if(WEBSERVER_UNLIKELY(!isValid())) {
            return false;
        }
    }

    if(WEBSERVER_UNLIKELY(addr->getFamily() != m_family)) {
        WEBSERVER_LOG_ERROR(g_logger) << "bind sock.family("
            << m_family << ") addr.family(" << addr->getFamily()
            << ") not equal, addr=" << addr->toString();
        return false;
    }

    if(timeout_ms == (uint64_t)-1) {
        if(::connect(m_sock, addr->getAddr(), addr->getAddrLen())) {
            WEBSERVER_LOG_ERROR(g_logger) << "sock=" << m_sock << "connect("
                << addr->toString() <<") timeout=" << timeout_ms << " errno="
                << errno << " strerror=" << strerror(errno);
            close();
            return false;
        }
    }else {
            if(::connect_with_timeout(m_sock, addr->getAddr(), addr->getAddrLen(), timeout_ms)) {
                WEBSERVER_LOG_ERROR(g_logger) << "sock=" << m_sock << "connect("
                    << addr->toString() <<") timeout=" << timeout_ms << " errno="
                    << errno << " strerror=" << strerror(errno);
                close();
                return false;
            }
        }

    m_isConnected = true;
    getRemoteAddress();
    getLocalAddress();
    return true;
}

bool Socket::listen(int backlog) {
    if(!isValid()) {
        WEBSERVER_LOG_ERROR(g_logger) << "listen error sock = -1";
        return false;
    }
    if(::listen(m_sock, backlog)) {
        WEBSERVER_LOG_ERROR(g_logger) << "listen error errno=" << errno
            << " errstr=" << strerror(errno);
        return false;
    }
    return true;
}

bool Socket::close() {
    if(!m_isConnected && m_sock == -1) {
        return -1;
    }
    m_isConnected = false;
    if(m_sock != -1) {
        ::close(m_sock);
        m_sock = -1;
    }
    return false;
}

int Socket::send(const void *buffer, size_t length, int flags) {
    if(isConnected()) {
        return ::send(m_sock, buffer, length, flags);
    }
    return -1;
}

int Socket::send(const iovec *buffers, size_t length, int flags) {
    if(isConnected()) {
        struct msghdr msg;
        memset(&msg, 0, sizeof(msg));
        msg.msg_iov = (iovec*)buffers;
        msg.msg_iovlen = length;
        return ::sendmsg(m_sock, &msg, flags);
    }
    return -1;
}

int Socket::sendTo(const void *buffer, size_t length, const Address::ptr to, int flags) {
    if(isConnected()) {
        return ::sendto(m_sock, buffer, length, flags, to->getAddr(), to->getAddrLen());
    }
    return -1;
}

int Socket::sendTo(const iovec *buffers, size_t length, const Address::ptr to, int flags) {
    if(isConnected()) {
        struct msghdr msg;
        memset(&msg, 0, sizeof(msg));
        msg.msg_iov = (iovec*)buffers;
        msg.msg_iovlen = length;
        msg.msg_name = to->getAddr();
        msg.msg_namelen = to->getAddrLen();
        return ::sendmsg(m_sock, &msg, flags);
    }
    return -1;
}

int Socket::recv(void *buffer, size_t length, int flags) {
    if(isConnected()) {
        return ::recv(m_sock, buffer, length, flags);
    }
    return -1;
}

int Socket::recv(const iovec *buffers, size_t length, int flags) {
    if(isConnected()) {
        struct msghdr msg;
        memset(&msg, 0, sizeof(msg));
        msg.msg_iov = (iovec*)buffers;
        msg.msg_iovlen = length;
        return ::recvmsg(m_sock, &msg, flags);
    }
    return -1;
}

int Socket::recvFrom(void *buffer, size_t length, Address::ptr from, int flags) {
    if(isConnected()) {
        socklen_t len = from->getAddrLen();
        return ::recvfrom(m_sock, buffer, length, flags, from->getAddr(), &len);
    }
    return -1;
}

int Socket::recvFrom(iovec *buffers, size_t length, const Address::ptr from, int flags) {
    if(isConnected()) {
        struct msghdr msg;
        memset(&msg, 0, sizeof(msg));
        msg.msg_iov = (iovec*)buffers;
        msg.msg_iovlen = length;
        msg.msg_name = from->getAddr();
        msg.msg_namelen = from->getAddrLen();
        return ::recvmsg(m_sock, &msg, flags);
    }
    return -1;

}

Address::ptr Socket::getRemoteAddress() {
    if(m_remoteAddress) {
        return m_remoteAddress;
    }

    Address::ptr result;
    switch(m_family) {
        case AF_INET:
            result.reset(new IPv4Address());
            break;
        case AF_INET6:
            result.reset(new IPv6Address());
            break;
        case AF_UNIX:
            result.reset(new UnixAddress());
            break;
        default:
            result.reset(new UnknownAddress(m_family));
            break;
    }
    socklen_t addrlen = result->getAddrLen();
    if(getpeername(m_sock, result->getAddr(), &addrlen)) {
        WEBSERVER_LOG_ERROR(g_logger) << "getpeername error sock=" << m_sock
            << " errno=" << errno << " strerror=" << strerror(errno);
        return Address::ptr(new UnknownAddress(m_family));
    }
    if(m_family == AF_UNIX) {
        UnixAddress::ptr addr = std::dynamic_pointer_cast<UnixAddress>(result);
        addr->setAddrLen(addrlen);
    }

    return m_remoteAddress = result;
}

Address::ptr Socket::getLocalAddress() {
    if(m_localAddress) {
        return m_localAddress;
    }

    Address::ptr result;
    switch(m_family) {
        case AF_INET:
            result.reset(new IPv4Address());
            break;
        case AF_INET6:
            result.reset(new IPv6Address());
            break;
        case AF_UNIX:
            result.reset(new UnixAddress());
            break;
        default:
            result.reset(new UnknownAddress(m_family));
            break;
    }
    socklen_t addrlen = result->getAddrLen();
    if(getsockname(m_sock, result->getAddr(), &addrlen)) {
        WEBSERVER_LOG_ERROR(g_logger) << "getsockname error sock=" << m_sock
            << " errno=" << errno << " strerror=" << strerror(errno);
        return Address::ptr(new UnknownAddress(m_family));
    }
    if(m_family == AF_UNIX) {
        UnixAddress::ptr addr = std::dynamic_pointer_cast<UnixAddress>(result);
        addr->setAddrLen(addrlen);
    }

    return m_remoteAddress = result;
}

bool Socket::isValid() const {
    return m_sock != -1;
}

int Socket::getError() {
    int error = 0;
    socklen_t len = sizeof(error);
    if(!getOption(SOL_SOCKET, SO_ERROR, &error, &len)) {
        return -1;
    }
    return error;
}

std::ostream &Socket::dump(std::ostream &os) const {
    os << "[Socket sock=" << m_sock
        << " is_connected=" << m_isConnected
        << " family=" << m_family
        << " type=" << m_type
        << "protocol=" << m_protocol;
    if(m_localAddress) {
        os << " local_address" << m_localAddress->toString();
    }
    if(m_remoteAddress) {
        os << " local_address" << m_remoteAddress->toString();
    }
    os << "]";

    return os;
}

std::string Socket::toString() const {
    std::stringstream ss;
    dump(ss);

    return ss.str();
}

bool Socket::cancelRead() {
    return IOManager::GetThis()->cancelEvent(m_sock, server_name::IOManager::READ);
}

bool Socket::cancelWrite() {
    return IOManager::GetThis()->cancelEvent(m_sock, server_name::IOManager::WRITE);
}

bool Socket::cancelAccept() {
    return IOManager::GetThis()->cancelEvent(m_sock, server_name::IOManager::READ);
}

bool Socket::cancelAll() {
    return IOManager::GetThis()->cancelAll(m_sock);
}

void Socket::initSock() {
    int val = 1;
    setOption(SOL_SOCKET, SO_REUSEADDR, val);
    if(m_type == SOCK_STREAM) {
        setOption(IPPROTO_TCP, TCP_NODELAY, val);
    }
}

void Socket::newSock() {
    m_sock = socket(m_family, m_type, m_protocol);
    if(WEBSERVER_LIKELY(m_sock != -1)) {
        initSock();
    } else {
        WEBSERVER_LOG_ERROR(g_logger) << "socket(" << m_family << ","
            << m_type << "," << m_protocol << ")" << " errno="
            << errno << " strerror=" << strerror(errno);
    }
}

std::ostream &operator<<(std::ostream &os, const Socket &sock) {
    return sock.dump(os);
}

}
