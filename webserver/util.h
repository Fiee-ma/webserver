#ifndef __WEBSERVER_UTIL_H__
#define __WEBSERVER_UTIL_H__

#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <string>

namespace server_name {

/**
 * @返回当前线程id
 */
pid_t GetThreadId();

/**
 * @返回当前线程id
 */
uint64_t GetCoroutineId();

void Backtrace(std::vector<std::string> &bt, int size = 64, int skip = 1);
std::string BacktraceToString(int size = 64, int skip = 2, const std::string &prefix = "");

uint64_t GetCurrentMS();
uint64_t GetCurrentUS();
}


#endif
