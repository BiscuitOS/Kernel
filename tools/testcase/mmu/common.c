/*
 * MMU(Memory Manager Unit)
 *
 * (C) 2017.12 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/mm.h>

#include <test/debug.h>

/* common memory entry */
int debug_mmu_common(void)
{
#ifdef CONFIG_DEBUG_MMU_LINEAR
    debug_mmu_linear_common();
#endif

#ifdef CONFIG_DEBUG_MMU_PHYSIC
    debug_mmu_physic_common();
#endif

#ifdef CONFIG_DEBUG_MMU_VIRTUAL
    debug_mmu_virtual_common();
#endif

#ifdef CONFIG_DEBUG_MMU_PGTABLE
    debug_mmu_pgtable_common();
#endif

#ifdef CONFIG_DEBUG_MMU_SEGMENTATION
    debug_mmu_segmentation_common();
#endif

#ifdef CONFIG_DEBUG_MMU_LOGIC
    debug_mmu_logical_common();
#endif

#ifdef CONFIG_DEBUG_MMU_PAGING
    debug_mmu_paging_common();
#endif

#ifdef CONFIG_DEBUG_MMU_ACCESS
    debug_mmu_access_right_common();
#endif

#ifdef CONFIG_DEBUG_MMU_PAGE_FAULT
    debug_mmu_page_fault_common();
#endif

#ifdef CONFIG_DEBUG_MMU_TLB
    debug_mmu_TLB_common();
#endif

#ifdef CONFIG_DEBUG_MMU_KMALLOC
    debug_mmu_kmalloc_common();
#endif

    return 0;
}

int debug_mmu_common_userland(void)
{
#ifdef CONFIG_DEBUG_MMU_VIRTUAL
    debug_mmu_virtual_common_userland();
#endif
    return 0;
}
