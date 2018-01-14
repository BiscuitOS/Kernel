/*
 * Two-Level Page-Table Machanism on MMU
 *
 * (C) 2017.12 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <test/debug.h>

/* common linear address entry */
int debug_mmu_pgtable_common(void)
{

#ifdef CONFIG_DEBUG_PGDIR
    debug_pgdir_common();
#endif

#ifdef CONFIG_DEBUG_PGTABLE
    debug_page_table_common();
#endif

    return 0;
}
