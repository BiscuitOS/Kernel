/*
 * CR3: Control Register 3
 *
 * (C) 2018.11.13 BuddyZhang1 <buddy.zhang@aliyun.com>
 * (C) 2018.11.13 BiscuitOS   <buddy.biscuitos@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <demo/debug.h>

#ifdef CONFIG_DEBUG_CR3_BITMAP
static int __unused cr3_bitmap(void)
{
    unsigned int __unused CR3;
    unsigned int __unused base;

    /* Obtain CR3 Register */
    __asm__ ("movl %%cr3, %0" : "=r" (CR3));

#ifdef CONFIG_DEBUG_CR3_BASE
    /*
     * Page-Directory Base field
     *
     * The physcial address of the base of the paging-structure hierarchy.
     */
    base = CR3 >> 12;
    base <<= 12;
    printk("Paing-Structure Base Physical address: %#x\n", base);
#endif

#ifdef CONFIG_DEBUG_CR3_PCD
    /*
     * CR3.PCD
     *
     * Page-level Cache Disable (bit 4 of CR3) -- Control the memory type used
     * to access the first paging structure of current paging-structure
     * hierarchy.
     */
    if ((CR3 >> 4) & 0x1) 
        printk("Disable Page-level Cache.\n");
    else
        printk("Enable Page-level Cache.\n");
#endif

#ifdef CONFIG_DEBUG_CR3_PWT
    /*
     * CR3.PWT
     *
     * Page-level Write-Through (bit 3 of the CR3) -- Control the memory type
     * used to access the first paging structure of the current paging-structure
     * hierarchy.
     */
    if ((CR3 >> 3) & 0x1)
        printk("Enable Page-level Write-Through.\n");
    else
        printk("Disable Page-level Write-Through.\n");
#endif

    printk("Hello World\n");

    return 0;
}
late_debugcall(cr3_bitmap);
#endif
