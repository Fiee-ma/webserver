#include "../webserver/http/http_connection.h"
#include "../webserver/log.h"
#include "../webserver/iomanager.h"
#include <iostream>

static server_name::Logger::ptr g_logger = WEBSERVER_LOG_ROOT();

void run() {
    server_name::Address::ptr addr = server_name::Address::LookupAnyIPAdress("www.baidu.com:80");
    if(!addr) {
        WEBSERVER_LOG_INFO(g_logger) << "get addr error";
        return;
    }
    server_name::Socket::ptr sock = server_name::Socket::CreateTCP(addr);
    bool rt = sock->connect(addr);
    if(!rt) {
        WEBSERVER_LOG_INFO(g_logger) << "connect" << *(addr.get()) << "failed";
        return;
    }

    server_name::http::HttpConnection::ptr conn(new server_name::http::HttpConnection(sock));

    server_name::http::HttpRequest::ptr req(new server_name::http::HttpRequest);
    WEBSERVER_LOG_INFO(g_logger) << "req:" << std::endl
        << *(req.get());

    conn->sendRequest(req);
    auto rsp = conn->recvResponse();
    if(!rsp) {
        WEBSERVER_LOG_WARN(g_logger) << "recv response error";
        return;
    }
    WEBSERVER_LOG_INFO(g_logger) << "rsq:" << std::endl
        << *(rsp.get());

}

int main() {
    server_name::IOManager iom(2);
    iom.schedule(run);

    return 0;
}
