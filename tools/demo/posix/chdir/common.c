/*
 * Common System Call: chdir
 *
 * (C) 2018.03 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <test/debug.h>

int debug_syscall_chdir_common_userland(void)
{
#ifdef CONFIG_DEBUG_SYSCALL_CHDIR0
    debug_syscall_chdir0();
#endif
    return 0;
}
