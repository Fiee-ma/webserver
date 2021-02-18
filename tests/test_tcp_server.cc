#include "../webserver/tcp_server.h"
#include "../webserver/iomanager.h"
#include "../webserver/log.h"

server_name::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void run() {
    auto addr = server_name::Address::LookupAny("0.0.0.0:8033", AF_INET);
    auto addr2 = server_name::UnixAddress::ptr(new server_name::UnixAddress("/tmp/unix_addr"));
    std::vector<server_name::Address::ptr> addrs;
    addrs.push_back(addr);
    addrs.push_back(addr2);
}

int main() {
    server_name::IOManager iom(2, true, "test");
    iom.schedule(run);
    return 0;
}
