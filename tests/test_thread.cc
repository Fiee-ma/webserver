#include "../sylar/sylar.h"
#include <unistd.h>

sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();
int count = 0;
sylar::RWMutex s_mutex;

void fun1() {
    SYLAR_LOG_INFO(g_logger) << "name: " << sylar::Thread::GetName()
                             << " this.name: " << sylar::Thread::GetThis()->getName()
                             << " id: " << sylar::GetThreadId()
                             << " this.id: " << sylar::Thread::GetThis()->getId();
    for(int i = 0; i < 1000000; ++i){
        sylar::RWMutex::ReadLock lock(s_mutex);
        ++count;
    }
}

void fun2() {
    while(true) {
        SYLAR_LOG_INFO(g_logger) << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
    }
}

void fun3() {
    while(true) {
        SYLAR_LOG_INFO(g_logger) << "1111111111111111111111111111";
    }
}

int main(int argc, char **argv) {
    SYLAR_LOG_INFO(g_logger) << "thread test begin";
    YAML::Node root = YAML::LoadFile("/home/marulong/sylar/bin/conf/log2.yml");
    sylar::Config::LoadFromYaml(root);
    std::vector<sylar::Thread::ptr> thrs;
    for(int i = 0; i < 2; ++i) {
        sylar::Thread::ptr thr(new sylar::Thread(&fun2, "name_" + std::to_string(i)));
        sylar::Thread::ptr thr1(new sylar::Thread(&fun3, "name_" + std::to_string(i)));
        thrs.push_back(thr);
        thrs.push_back(thr1);
    }

    for(size_t i = 0; i < thrs.size(); ++i) {
        thrs[i]->join();
    }
    SYLAR_LOG_INFO(g_logger) << "thread test begin";
    SYLAR_LOG_INFO(g_logger) << "count = "<< count;
    return 0;
}
