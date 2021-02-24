/**
 * 常用宏的封装
 */

#ifndef __WEBSERVER_ASSERT_H__
#define __WEBSERVER_ASSERT_H__

#include <string.h>
#include <assert.h>
#include "util.h"
#include "log.h"

#if defined __GNUC__ || defined __llvm__
/// LIKELY 宏的封装，告诉编译器优化，条件大概率成立
# define WEBSERVER_LIKELY(x)  __builtin_expect(!!(x), 1)
/// LIKELY 宏的封装，告诉编译器优化，条件大概率不成立
# define WEBSERVER_UNLIKELY(x)  __builtin_expect(!!(x), 0)
# else
# define WEBSERVER_LIKELY(x)  (x)
#define  WEBSERVER_UNLIKELY(x)  (x)
#endif

#define WEBSERVER_ASSERT(x) \
    if(WEBSERVER_UNLIKELY(!(x))) { \
        WEBSERVER_LOG_ERROR(WEBSERVER_LOG_ROOT()) << "ASSERT: " #x \
            << "\nbacktrance\n" \
            << server_name::BacktraceToString(100, 2, "    "); \
        assert(x); \
    }

#define WEBSERVER_ASSERT2(x, prefix) \
    if(WEBSERVER_UNLIKELY(!(x))) { \
        WEBSERVER_LOG_ERROR(WEBSERVER_LOG_ROOT()) << "ASSERT: " #x \
            << "\n" << prefix \
            << "\nbacktrance\n" \
            << server_name::BacktraceToString(100, 2, "    "); \
        assert(x); \
    }




#endif
