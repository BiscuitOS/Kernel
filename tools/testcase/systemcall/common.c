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

int common_system_call_entry(void)
{
    printk("Common system call\n");

#ifdef CONFIG_TESTCASE_SYSCALL_ROUTINE
    common_system_call_rountine();
#endif

    return 0;
}
