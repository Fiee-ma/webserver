#include "../sylar/sylar.h"

sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void test_scheduler() {
    static int s_count = 5;
    SYLAR_LOG_INFO(g_logger) << "test_scheduler " << s_count;
    if(--s_count >= 0) {
        sleep(1);
        sylar::Scheduler::GetThis()->schedule(&test_scheduler, sylar::GetThreadId());
    }
}

int main() {
    sylar::Scheduler sc(3, false, "test");
    sc.start();
    sc.schedule(&test_scheduler);
    sc.stop();

    return 0;
}
