#include "../sylar/sylar.h"

sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void run_in_fiber() {
    SYLAR_LOG_INFO(g_logger) << "run_in_fiber begin";
    sylar::Fiber::YieldToHold();
    SYLAR_LOG_INFO(g_logger) << "run_in_fiber_hold";
    sylar::Fiber::YieldToHold();
    SYLAR_LOG_INFO(g_logger) << "run_in_fiber_hold_end";
    sylar::Fiber::YieldToHold();
}

void test_fiber() {
    {
        sylar::Fiber::GetThis();
        SYLAR_LOG_INFO(g_logger) << "main begin";
        sylar::Fiber::ptr fiber(new sylar::Fiber(run_in_fiber));
        fiber->swapIn();
        SYLAR_LOG_INFO(g_logger) << "main_run_in_fiber_hold1_return";
        fiber->swapIn();
        SYLAR_LOG_INFO(g_logger) << "main_run_in_fiber_hold2_return";
        fiber->swapIn();
        SYLAR_LOG_INFO(g_logger) << "main return";
        fiber->swapIn();
        SYLAR_LOG_INFO(g_logger) << "hahahahahahaha";
        fiber->swapIn();
    }
    SYLAR_LOG_INFO(g_logger) << "main return2";
}

int main() {
    std::vector<sylar::Thread::ptr> vec;
    for(int i = 0; i < 3; ++i) {
        vec.push_back(sylar::Thread::ptr(
                    new sylar::Thread(&test_fiber, "name_" + std::to_string(i))));
    }

    for(auto &i : vec) {
        i->join();
    }
    return 0;
}
