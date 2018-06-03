/*
 * Virtual Address Machanism on MMU
 *
 * (C) 2017.12 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <test/debug.h>

/* common virtual address entry */
int debug_mmu_virtual_common(void)
{
    return 0;
}

/* common virtual address entry on userland */
int debug_mmu_virtual_common_userland(void)
{
#ifdef CONFIG_DEBUG_MMU_VIRTUAL_TABLE
    debug_virtual_address_common_userland();
#endif

    return 0;
}
