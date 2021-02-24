#include "../webserver/socket.h"
#include "../webserver/webserver.h"
#include "../webserver/iomanager.h"

static server_name::Logger::ptr g_logger = WEBSERVER_LOG_ROOT();

void test_socket() {
    //std::vector<server_name::Address::ptr> addrs;
    //server_name::Address::Lookup(addrs, "www.baidu.com", AF_INET);
    //server_name::IPAddress::ptr addr;
    //for(auto &i : addrs) {
    //    addr = std::dynamic_pointer_cast<server_name::IPAddress>(i);
    //    if(addr) {
    //        break;
    //    }
    //}
    server_name::IPAddress::ptr addr = server_name::Address::LookupAnyIPAdress("www.baidu.com");
    if(addr) {
        WEBSERVER_LOG_INFO(g_logger) << "get address:" << addr->toString();
    } else {
        WEBSERVER_LOG_ERROR(g_logger) << "get address fail";
        return;
    }

    server_name::Socket::ptr sock = server_name::Socket::CreateTCP(addr);
    addr->setPort(80);
    if(!sock->connect(addr)) {
        WEBSERVER_LOG_ERROR(g_logger) << "connect " << addr->toString() << " fail";
        return;
    } else {
        WEBSERVER_LOG_INFO(g_logger) << "connect " << addr->toString() << " connected";
    }

    const char buff[] = "GET / HTTP/1.0\r\n\r\n";
    int rt = sock->send(buff, sizeof(buff));
    if(rt <=0) {
        WEBSERVER_LOG_ERROR(g_logger) << "send fail rt=" << rt;
        return;
    }

    std::string buffs;
    buffs.resize(4096);
    rt = sock->recv(&buffs[0], buffs.size());

    if(rt <= 0) {
        WEBSERVER_LOG_ERROR(g_logger) << "recv fail rt=" << rt;
        return;
    }

    buffs.resize(rt);
    WEBSERVER_LOG_INFO(g_logger) << buffs;
}

int main() {
    server_name::IOManager iom;
    iom.schedule(test_socket);
    return 0;
}
