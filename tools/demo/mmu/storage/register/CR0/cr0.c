/*
 * CR0: Control Register 0
 *
 * (C) 2018.11.11 BuddyZhang1 <buddy.zhang@aliyun.com>
 * (C) 2018.11.11 BiscuitOS   <buddy.biscuitos@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <demo/debug.h>

#ifdef CONFIG_DEBUG_CR0_BITMAP
static int __unused cr0_bitmap(void)
{
    unsigned int __unused CR0;

    /* Obtain CR0 Register */
    __asm__ ("mov %%cr0, %0" : "=r" (CR0));

#ifdef CONFIG_DEBUG_CR0_PG
    /*
     * CR0.PG
     *
     * Paging(bit 31 of CR0) -- Enables paging when set; disables paging when
     * clear. When paging is disabled, all linear address are treated as 
     * physical address. The PG flag has no effect if the PE flag (bit 0 of
     * register CR0) is not also set; setting the PG flag when the PE flag is
     * clear causes a general-protection exception (#GP).
     */
    if ((CR0 >> 31) & 0x1) {
        if (CR0 & 0x1) {
            printk("Paing is enable and PE flag has set.\n");
            printk("Then, clear PE flag and trigger #GP.\n");
            CR0 &= 0xFFFFFFFE;
            __asm__ ("mov %0, %%cr0" :: "r" (CR0));
        } else 
            printk("Paing is enable and PE flag clear and cause #GP.\n");
    } else
        printk("Paing is disable\n");
#endif

#ifdef CONFIG_DEBUG_CR0_CD
    /*
     * CR0.CD
     *
     * Cache Disable(bit 30 of CR0) -- When the CD and NW flag are clear, 
     * caching of memory locations for the whole of physical memory in the
     * processor's internal (and external) caches is enabled. When the CD flag
     * is set, caching is restricted as described in Table. To prevent the
     * processor from accessing and updating its caches, the CD flag must be
     * set and the caches must be invalidated so that no cache hits can occur.
     */
    if ((CR0 >> 30) & 0x1)
        printk("Cache is disable.\n");
    else
        printk("Cache is enable.\n");
#endif

#ifdef CONFIG_DEBUG_CR0_NW
    /*
     * CR0.NW
     *
     * Not Write-through(bit 29 of CR0) -- When the NW and CD flags are clear,
     * write-back or write-through is enabled for writes that hit the cache and
     * invalidation cycles are enabled.
     */
    if ((CR0 >> 29) & 0x1)
        printk("Not Write-through.\n");
    else {
        if (!((CR0 >> 30) & 0x1))
            printk("write-back or write-through is enable.\n");
    }
#endif

#ifdef CONFIG_DEBUG_CR0_AM
    /*
     * CR0.AM
     *
     * Alignment Mask(bit 18 of CR0) -- Enables automatic alignment checking
     * when set; disables alignment checking when clear. Alignment checking is
     * performed only when the AM flag is set, the AC flag in the EFLAGS
     * register is set, CPL is 3, and the processor is operating in either
     * protected or virtual-8086 mode.
     */
    if ((CR0 >> 18) & 0x1)
        printk("Enables automatic alignment checking.\n");
    else
        printk("Disable automatic alignment checking.\n");
#endif

#ifdef CONFIG_DEBUG_CR0_WP
    /*
     * CR0.WP
     *
     * Write Protect(bit 16 of CR0) -- When set, inhibits supervisor-level
     * procedure from writing into read-only pages; when clear, allows
     * supervisor-level procedures to write into read-only pages (regardless
     * of the U/S bit setting). The flag facilitiates implementation of the
     * copy-on-write method of creating a new process (forking) used by
     * operating systems such as UNIX.
     */
    if ((CR0 >> 16) & 0x1)
        printk("Enable write protect\n");
    else
        printk("Disable write protect.\n");
#endif

#ifdef CONFIG_DEBUG_CR0_PE
    /*
     * CR0.PE
     *
     * Protection Enable(bit 0 of the CR0) -- Enables protected mode when set;
     * enables real-address mode when clear. This flag does not enable paging
     * directly. It only enables segment-level protection. To enable paging,
     * both the PE and PG flags must be set.
     */
    if (CR0 & 0x1)
        printk("Enable segment protection.\n");
    else
        printk("Disable segment protection.\n");
#endif

    return 0;
}
late_debugcall(cr0_bitmap);
#endif
