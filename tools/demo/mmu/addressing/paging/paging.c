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

#ifdef CONFIG_DEBUG_PAGING_MODE
static int __unused paging_mode(void)
{
    unsigned int __unused mode;
    unsigned int __unused CR0;
    unsigned int __unused CR4;
    unsigned int __unused IA32_EFER;

    /* Obtain CR0 Register */
    __asm__ ("mov %%cr0, %0" : "=r" (CR0));

    /* Obtain CR4 Register */
    __asm__ ("mov %%cr4, %0" : "=r" (CR4));

    /* Obtain IA32_EFER, not support IA32e */
    IA32_EFER = 0;

#ifdef CONFIG_DEBUG_PAGING_MODE_DETECT
    /*
     * 32-bit Paging mode
     *   If CR0.PG = 1 and CR4.PAE = 0, 32-bit paging is used.
     */
    if (((CR0 >> 31) & 0x1) && !((CR4 >> 5) & 0x1)) {
        printk("32-bit Paging Modes.\n");
    } else if (((CR0 >> 31) & 0x1) && ((CR4 >> 5) & 0x1) && 
              !((IA32_EFER >> 8) & 0x1)) {
        /*
         * PAE Paing mode
         *   If CR0.PG = 1, CR4.PAE = 1, and IA32_EFER.LME = 0, PAE paging is
         *   used.
         */
        printk("PAE Paging Mode.\n");
    } else if (((CR0 >> 31) & 0x1) && ((CR4 >> 5) & 0x1) &&
              ((IA32_EFER >> 8) & 0x1)) {
        /*
         * 4-level Paging mode.
         *   If CR0.PG = 1, CR4.PAE = 1, and IA32_EFER.LME = 1, 4-level paging
         *   is used.
         */
        printk("4-level Paging Mode.\n");
    } else
        printk("Unknow Paging Mode.\n");

#endif

    return 0;
}
late_debugcall(paging_mode);
#endif
