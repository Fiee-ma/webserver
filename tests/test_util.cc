#include "../webserver/sylar.h"
#include <assert.h>

server_name::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void test_assert() {
    SYLAR_LOG_INFO(g_logger) << server_name::BacktraceToString(10, 0, "   ");
    SYLAR_ASSERT(false);
    SYLAR_ASSERT2(0==1, "marulong");
}


int main(int argc, char **argv) {
    test_assert();

    return 0;
}
