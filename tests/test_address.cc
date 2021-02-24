#include "../webserver/sockaddr.h"
#include "../webserver/log.h"

server_name::Logger::ptr g_logger = WEBSERVER_LOG_ROOT();

void test_adress() {
    std::vector<server_name::Address::ptr> result;
    bool v = server_name::Address::Lookup(result, "www.baidu.com:http", AF_UNSPEC);

    if(!v) {
        WEBSERVER_LOG_INFO(g_logger) << "lookup fail";
        return;
    }
    for(size_t i = 0; i < result.size(); ++i) {
        WEBSERVER_LOG_INFO(g_logger) << i << " - " << result[i]->toString();
    }
}

void test_iface() {
    std::multimap<std::string, std::pair<server_name::Address::ptr, uint32_t> > results;

    bool v = server_name::Address::GetInterfaceAddresses(results);
    if(!v) {
        WEBSERVER_LOG_ERROR(g_logger) << "GetInterfaceAddresses fail";
        return;
    }

    for(auto &i : results) {
        WEBSERVER_LOG_INFO(g_logger) << i.first << " - " << i.second.first->toString() << "-"
            << i.second.second;
    }
}

void test_ipv4() {
    auto addr = server_name::IPv4Address::Create("127.0.0.8");
//    auto addr1 = server_name::IPv4Address("127.0.0.8");
    if(addr) {
        WEBSERVER_LOG_INFO(g_logger) << addr->toString();
    }
}
int main() {
    test_adress();
    //test_iface();
//    test_ipv4();

    return 0;
}
