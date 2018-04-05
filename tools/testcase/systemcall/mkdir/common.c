/*
 * Common System Call: mkdir
 *
 * (C) 2018.04 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <test/debug.h>

int debug_syscall_mkdir_common_userland(void)
{
#ifdef CONFIG_DEBUG_SYSCALL_MKDIR0
    debug_syscall_mkdir0();
#endif
    return 0;
}
