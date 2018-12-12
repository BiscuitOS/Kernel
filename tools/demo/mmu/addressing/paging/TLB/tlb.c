/*
 * TLB caching mechanism
 *
 * (C) 2018.11.26 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>

#include <demo/debug.h>

static int __unused tlb_entence(void)
{
    printk("Hello World\n");

    return 0;
}
late_debugcall(tlb_entence);

static int __unused PCIDs_entence(void)
{
    unsigned long __unused CR4;

    /* Obtain CR4.PCIDE bit */
    __asm__ ("movl %%cr4, %0" : "=r" (CR4));

    if ((CR4 >> 17) & 0x1) 
        printk("The current PCID is the value of bits of bits 11:0 of CR3\n");
    else
        printk("The current PCID is always 000H\n");

    return 0;
}
late_debugcall(PCIDs_entence);
