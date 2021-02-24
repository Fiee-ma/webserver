#include "util.h"
#include "log.h"
#include <execinfo.h>
#include <vector>
#include <sstream>
#include <string.h>
#include <unistd.h>
#include "coroutine.h"
#include <sys/time.h>

namespace server_name {

static server_name::Logger::ptr g_logger = WEBSERVER_LOG_NAME("system");

pid_t GetThreadId(){
   return syscall(SYS_gettid);
}

uint64_t GetCoroutineId(){
    return Coroutine::GetCoroutineId();
}

void Backtrace(std::vector<std::string> &bt, int size, int skip) {
    void **array = (void **)malloc((sizeof(void *) * size));
    size_t s = ::backtrace(array, size);

    char **strings = backtrace_symbols(array, s);
    if(strings == NULL) {
        WEBSERVER_LOG_ERROR(g_logger) << "backtrace_symbols error";
        return;
    }

    for(size_t i = skip; i < s; ++i) {
        bt.push_back(strings[i]);
    }

    free(strings);
    free(array);
}

std::string BacktraceToString(int size, int skip, const std::string &prefix) {
    std::vector<std::string> bt;
    Backtrace(bt, size, skip);
    std::stringstream ss;
    for(auto &i : bt) {
        ss << prefix << i << std::endl;
    }

    return ss.str();
}

uint64_t GetCurrentMS() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}
uint64_t GetCurrentUS() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec;
}


}
