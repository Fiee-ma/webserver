#include "../webserver/tcp_server.h"
#include "../webserver/iomanager.h"
#include "../webserver/log.h"

server_name::Logger::ptr g_logger = WEBSERVER_LOG_ROOT();

void run() {
    auto addr = server_name::Address::LookupAny("0.0.0.0:8033", AF_INET);
//    auto addr2 = server_name::UnixAddress::ptr(new server_name::UnixAddress("/tmp/unix_addr"));
    std::vector<server_name::Address::ptr> addrs;
    addrs.push_back(addr);
//    addrs.push_back(addr2);

    server_name::TcpServer::ptr tcp_server(new server_name::TcpServer);
    std::vector<server_name::Address::ptr> fails;
    while(!tcp_server->bind(addrs, fails)) {
        sleep(2);
    }
    tcp_server->start();
}

int main() {
    server_name::IOManager iom(2, true, "test");
    iom.schedule(run);
    return 0;
}
