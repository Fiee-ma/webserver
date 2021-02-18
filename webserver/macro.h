/**
 * 常用宏的封装
 */

#ifndef __SYLAR_MACRO_H__
#define __SYLAR_MACRO_H__

#include <string.h>
#include <assert.h>
#include "util.h"
#include "log.h"

#if defined __GNUC__ || defined __llvm__
/// LIKELY 宏的封装，告诉编译器优化，条件大概率成立
# define SYLAR_LIKELY(x)  __builtin_expect(!!(x), 1)
/// LIKELY 宏的封装，告诉编译器优化，条件大概率不成立
# define SYLAR_UNLIKELY(x)  __builtin_expect(!!(x), 0)
# else
# define SYLAR_LIKELY(x)  (x)
#define  SYLAR_UNLIKELY(x)  (x)
#endif

#define SYLAR_ASSERT(x) \
    if(SYLAR_UNLIKELY(!(x))) { \
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "ASSERT: " #x \
            << "\nbacktrance\n" \
            << server_name::BacktraceToString(100, 2, "    "); \
        assert(x); \
    }

#define SYLAR_ASSERT2(x, prefix) \
    if(SYLAR_UNLIKELY(!(x))) { \
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "ASSERT: " #x \
            << "\n" << prefix \
            << "\nbacktrance\n" \
            << server_name::BacktraceToString(100, 2, "    "); \
        assert(x); \
    }




#endif
