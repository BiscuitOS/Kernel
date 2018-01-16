/*
 * Control Register (CR0 and CR4) on Paging Mode.
 *
 * (C) 2017.12 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>

/*
 * Detect system whether support paging.
 */
static inline int detect_system_paging(void)
{
    unsigned long cr0;

    __asm__ ("movl %%cr0, %0" : "=r" (cr0));
    return (cr0 >> 31) & 0x1;
}

/*
 * Detect paging whether support protection.
 */
static inline int detect_paging_protection(void)
{
    unsigned long cr0;

    __asm__ ("movl %%cr0, %0" : "=r" (cr0));
    return (cr0 & 0x1);
}

/*
 * Detect Paging bit-width
 *
 *   @return: 0 - 32bit paging
 *            1 - 36bit paging
 */
static inline int detect_paging_bit_width(void)
{
    unsigned long cr0, cr4;

    __asm__ ("movl %%cr0, %0\n\r"
             "movl %%cr4, %1" 
             : "=r" (cr0), "=r" (cr4));
    /* If CR0.PG = 1 and CR4.PAE = 0, 32-bit paging is used. 32-bit paging
     * is detailed in Section `32-bit paging`. */
    if (cr0 >> 31) {
        if ((cr4 >> 5) & 0x1) {
            printk("36-bit Paging Mode.\n");
            return 0;
        } else {
            printk("32-bit Paging Mode.\n");
            return 1;
        }
    }
    return -1;
}

/* obtain CR3 contents */
static inline unsigned long obtain_cr3(void)
{
    unsigned long cr3;

    __asm__ ("movl %%cr3, %0" : "=r" (cr3));
    return cr3;
}

/* common linear address entry */
int debug_paging_cr0_common(void)
{
    if (1) {
        printk("CR3 contents: %#x\n", obtain_cr3());
    } else {
        if (detect_system_paging()) {
            printk("System support Paging Mode.\n");
            if (detect_paging_protection())
                printk("System support Protection on Paging.\n");
        } else
            printk("System doesn't support Paging mode.\n");
        detect_paging_bit_width();
        obtain_cr3();
    }
    return 0;
}
