/*
 * System Call: Open
 *
 * (C) 2018.03 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#define __LIBRARY__
#include <unistd.h>
#include <stdarg.h>

int d_open(const char *filename, int flag, ...)
{
    register int res;
    va_list arg;

    va_start(arg, flag);
    __asm__ ("int $0x80"
             : "=a" (res)
             : "0"  (__NR_d_open), "b" (filename), "c" (flag),
               "d"  (va_arg(arg, int)));
    if (res >= 0)
        return res;
    errno = -res;
    return -1;
}
