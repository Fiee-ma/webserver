
#include "../webserver/sylar.h"

server_name::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void run_in_fiber() {
    SYLAR_LOG_INFO(g_logger) << "run_in_fiber begin";
    server_name::Fiber::YieldToHold();
    SYLAR_LOG_INFO(g_logger) << "run_in_fiber end";
    server_name::Fiber::YieldToHold();
}

void test_fiber() {
    SYLAR_LOG_INFO(g_logger) << "main begin -1";
    {
        server_name::Fiber::GetThis();
        SYLAR_LOG_INFO(g_logger) << "main begin";
        server_name::Fiber::ptr fiber(new server_name::Fiber(run_in_fiber));
        fiber->swapIn();
        SYLAR_LOG_INFO(g_logger) << "main after swapIn";
        fiber->swapIn();
        SYLAR_LOG_INFO(g_logger) << "main after end";
        fiber->swapIn();
    }
    SYLAR_LOG_INFO(g_logger) << "main after end2";
}

int main(int argc, char** argv) {
    server_name::Thread::SetName("main");

    std::vector<server_name::Thread::ptr> thrs;
    for(int i = 0; i < 3; ++i) {
        thrs.push_back(server_name::Thread::ptr(
                    new server_name::Thread(&test_fiber, "name_" + std::to_string(i))));
    }
    for(auto i : thrs) {
        i->join();
    }
    return 0;
}
