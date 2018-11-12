/*
 * CR4: Control Register 4
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

#ifdef CONFIG_DEBUG_CR4_BITMAP
static int __unused cr4_bitmap(void)
{
    unsigned int __unused CR4;

    /* Obtain CR4 register */
    __asm__ ("mov %%cr4, %0" : "=r" (CR4));

#ifdef CONFIG_DEBUG_CR4_PSE
    /*
     * CR4.PSE
     *
     * Page Size Extensions (bit4 0f CR4) -- Enables 4-MByte pages with 32-bit
     * paging when set; restricts 32-bit paging to pages of 4 KBytes when clear.
     */
    if ((CR4 >> 4) & 0x1)
        printk("Enable 4-MByte pages with 32-bit paging.\n");
    else
        printk("Disable 4-KByte page with 32-bit paging.\n");
#endif

#ifdef CONFIG_DEBUG_CR4_PAE
    /*
     * CR4.PAE
     *
     * Physical Address Extension (bit 5 of CR4) -- When set, enables paging to
     * produce physical addresses with more than 32 bits. When clear, restricts
     * physcial addresses to 32-bits. PAE must be set before entering IA-32e
     * mode.
     */
    if ((CR4 >> 5) & 0x1)
        printk("Enable paging physical addresses with more than 32-bits");
    else
        printk("Disable paging physical addresses with more than 32-bits");
#endif

#ifdef CONFIG_DEBUG_CR4_PGE
    /*
     * CR4.PGE
     *
     * Page Global Enable (bit 7 of CR4) -- Enables the global page feature
     * when set; disables the global page feature when clear. The global page
     * feature allows frequently used or shared pages to be marked as global
     * to all users (done with the global flag, bit 8, in a page-director or
     * page-table entry). Global pages are not flushed from the
     * translation-lookaside buffer (TLB) on a task switch or a write to
     * register CR3.
     *
     * When enabling the global page feature, paging must be enabled (by
     * setting the PG flag in control register CR0) before the PEG flag is set.
     * Reversing the sequence may affect program correctness, and processor
     * performance will be impacted.
     */
    if ((CR4 >> 7) & 0x1)
        printk("Enable Page Global.\n");
    else
        printk("Disable Page Global.\n");
#endif

#ifdef CONFIG_DEBUG_CR4_PCID
    /*
     * CR4.PCIDE
     *
     * PCID-Enable Bit (bit 17 of CR4) -- Enables processor-context identifiers
     * (PCIDs) when set.
     */
    if ((CR4 >> 17) & 0x1)
        printk("Enable PCID\n");
    else
        printk("Disable PCID\n");
#endif

#ifdef CONFIG_DEBUG_CR4_SMEP
    /*
     * CR4.SMEP
     *
     * SMEP-Enable Bit (bit 20 of CR4) -- Enables supervisor-mode execution
     * prevention (SMEP) when set.
     */
    if ((CR4 >> 20) & 0x1)
        printk("Enables supervisor-mode execution prevention.\n");
    else
        printk("Disable supervisor-mode execution prevention.\n");
#endif

#ifdef CONFIG_DEBUG_CR4_SMAP
    /*
     * CR4.SMAP
     *
     * SMAP-Enable Bit (bit 21 of CR4) -- Enables supervisor-mode access
     * prevention (SMAP) when set.
     */
    if ((CR4 >> 21) & 0x1)
        printk("Enables supervisor-mode access prevention.\n");
    else
        printk("Disable supervisor-mode access prevention.\n");
#endif

#ifdef CONFIG_DEBUG_CR4_PKE
    /*
     * CR4.PKE
     *
     * Protection-Key-Enable Bit (bit 22 of CR4) -- Enables 4-level paging to
     * associate each linear address with a protection key. The PKRU register
     * specifies, for each protection key, whether user-mode linear addresses
     * with that protection key can be read or written. This bit also enables
     * access to the PKRU register using the RDPKRU and WRPKRU instructions.
     */
    if ((CR4 >> 22) & 0x1) 
        printk("Enable 4-level paging to associate each linear address.\n");
    else
        printk("Disable 4-level paging to associate each linear address.\n");
#endif

    return 0;
}
late_debugcall(cr4_bitmap);
#endif
