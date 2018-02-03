/*
 * Paging mechanism on X86 Architecture.
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
int debug_mmu_paging_common(void)
{
#ifdef CONFIG_DEBUG_PAGING_REGISTER
    debug_paging_register_common();
#endif
    return 0;
}
