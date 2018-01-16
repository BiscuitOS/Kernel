/*
 * Common System Call
 *
 * (C) 2017.12 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <test/debug.h>

int debug_syscall_common(void)
{
#ifdef CONFIG_DEBUG_SYSCALL_ROUTINE
    system_call_rountine();
#endif

    return 0;
}
