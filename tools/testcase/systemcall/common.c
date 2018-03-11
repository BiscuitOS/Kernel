/*
 * Common System Call
 *
 * (C) 2018.03 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/*
 * Note! Broken the number of system call on linux 0.11, we should modify
 * 'nr_system_calls' on kernel/system_call.c. Default value is 72, so the 
 * number of system call over it will directly return -1.
 */
#define __LIBRARY__
#include <unistd.h>
#include <stdarg.h>
#include <fcntl.h>

#include <linux/kernel.h>
#include <test/debug.h>

static char printbuf[1024];
extern int vsprintf(char *buf, const char *fmt, va_list args);

int debug_syscall_common(void)
{
#ifdef CONFIG_DEBUG_SYSCALL_ROUTINE
    system_call_rountine();
#endif
    return 0;
}

/******* Userland *********/
int d_printf(const char *fmt, ...)
{
    va_list args;
    int i;

    va_start(args, fmt);
    write(1, printbuf, i = vsprintf(printbuf, fmt, args));
    va_end(args);
    return i;
}

/* Debug on userland */
int debug_syscall_common_userland(void)
{
#ifdef CONFIG_DEBUG_SYSCALL_OPEN
    debug_syscall_open_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_CLOSE
    debug_syscall_close_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_READ
    debug_syscall_read_common_userland();
#endif

    return 0;
}
