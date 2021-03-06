#include "../webserver/webserver.h"

server_name::Logger::ptr g_logger = WEBSERVER_LOG_ROOT();

void test_scheduler() {
    static int s_count = 5;
    WEBSERVER_LOG_INFO(g_logger) << "test_scheduler " << s_count;
    if(--s_count >= 0) {
        sleep(1);
        server_name::Scheduler::GetThis()->schedule(&test_scheduler, server_name::GetThreadId());
    }
}

int main() {
    server_name::Scheduler sc(3, false, "test");
    sc.start();
    sc.schedule(&test_scheduler);
    sc.stop();

    return 0;
}
