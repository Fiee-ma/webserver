
#include "../webserver/webserver.h"

server_name::Logger::ptr g_logger = WEBSERVER_LOG_ROOT();

void run_in_coroutine() {
    WEBSERVER_LOG_INFO(g_logger) << "run_in_coroutine begin";
    server_name::Coroutine::YieldToHold();
    WEBSERVER_LOG_INFO(g_logger) << "run_in_coroutine end";
    server_name::Coroutine::YieldToHold();
}

void test_coroutine() {
    WEBSERVER_LOG_INFO(g_logger) << "main begin -1";
    {
        server_name::Coroutine::GetThis();
        WEBSERVER_LOG_INFO(g_logger) << "main begin";
        server_name::Coroutine::ptr coroutine(new server_name::Coroutine(run_in_coroutine));
        coroutine->swapIn();
        WEBSERVER_LOG_INFO(g_logger) << "main after swapIn";
        coroutine->swapIn();
        WEBSERVER_LOG_INFO(g_logger) << "main after end";
        coroutine->swapIn();
    }
    WEBSERVER_LOG_INFO(g_logger) << "main after end2";
}

int main(int argc, char** argv) {
    server_name::Thread::SetName("main");

    std::vector<server_name::Thread::ptr> thrs;
    for(int i = 0; i < 3; ++i) {
        thrs.push_back(server_name::Thread::ptr(
                    new server_name::Thread(&test_coroutine, "name_" + std::to_string(i))));
    }
    for(auto i : thrs) {
        i->join();
    }
    return 0;
}
