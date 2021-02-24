#include "../webserver/webserver.h"
#include <unistd.h>

server_name::Logger::ptr g_logger = WEBSERVER_LOG_ROOT();
int count = 0;
server_name::RWMutex s_mutex;

void fun1() {
    WEBSERVER_LOG_INFO(g_logger) << "name: " << server_name::Thread::GetName()
                             << " this.name: " << server_name::Thread::GetThis()->getName()
                             << " id: " << server_name::GetThreadId()
                             << " this.id: " << server_name::Thread::GetThis()->getId();
    for(int i = 0; i < 1000000; ++i){
        server_name::RWMutex::ReadLock lock(s_mutex);
        ++count;
    }
}

void fun2() {
    while(true) {
        WEBSERVER_LOG_INFO(g_logger) << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
    }
}

void fun3() {
    while(true) {
        WEBSERVER_LOG_INFO(g_logger) << "1111111111111111111111111111";
    }
}

int main(int argc, char **argv) {
    WEBSERVER_LOG_INFO(g_logger) << "thread test begin";
//    YAML::Node root = YAML::LoadFile("/home/marulong/webserver/bin/conf/log2.yml");
//    server_name::Config::LoadFromYaml(root);
    std::vector<server_name::Thread::ptr> thrs;
    for(int i = 0; i < 2; ++i) {
        server_name::Thread::ptr thr(new server_name::Thread(&fun2, "name_" + std::to_string(i)));
        server_name::Thread::ptr thr1(new server_name::Thread(&fun3, "name_" + std::to_string(i)));
        thrs.push_back(thr);
        thrs.push_back(thr1);
    }

    WEBSERVER_LOG_DEBUG(g_logger) << "thrs[i]->join before";
    for(size_t i = 0; i < thrs.size(); ++i) {
        thrs[i]->join();
    WEBSERVER_LOG_DEBUG(g_logger) << "thrs[i]->join after";
    }
    WEBSERVER_LOG_INFO(g_logger) << "thread test begin";
    WEBSERVER_LOG_INFO(g_logger) << "count = "<< count;
    return 0;
}
