/*
 * Caching address-translation into TLB
 *
 * (C) 2018.01 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/head.h>
#include <linux/sched.h>

#include <test/debug.h>

/* common access right entry */
int debug_TLB_common(void)
{
    return 0;
}
