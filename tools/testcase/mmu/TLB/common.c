/*
 * TLB Mechanism on MMU
 *
 * (C) 2018.01 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <test/debug.h>

/* common TLB entry */
int debug_mmu_TLB_common(void)
{
#ifdef CONFIG_DEBUG_TLB
    debug_TLB_common();
#endif

    return 0;
}
