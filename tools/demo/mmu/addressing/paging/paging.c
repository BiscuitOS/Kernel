/*
 * Paging mechanism
 *
 * (C) 2018.11.10 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>

#include <demo/debug.h>

#ifdef CONFIG_DEBUG_PR_CR0
static int __unused paging_CR0(void)
{
    printk("Hello World\n");

    return 0;
}
late_debugcall(paging_CR0);
#endif
