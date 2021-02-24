#include "../webserver/webserver.h"
#include <assert.h>

server_name::Logger::ptr g_logger = WEBSERVER_LOG_ROOT();

void test_assert() {
    WEBSERVER_LOG_INFO(g_logger) << server_name::BacktraceToString(10, 0, "   ");
    WEBSERVER_ASSERT(false);
    WEBSERVER_ASSERT2(0==1, "marulong");
}


int main(int argc, char **argv) {
    test_assert();

    return 0;
}
