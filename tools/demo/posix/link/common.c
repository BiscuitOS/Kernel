/*
 * Common System Call: link
 *
 * (C) 2018.04 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <test/debug.h>

int debug_syscall_link_common_userland(void)
{
#ifdef CONFIG_DEBUG_SYSCALL_LINK0
    debug_syscall_link0();
#endif
    return 0;
}
