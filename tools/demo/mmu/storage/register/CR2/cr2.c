/*
 * CR2: Control Register 2
 *
 * (C) 2018.11.20 BuddyZhang1 <buddy.zhang@aliyun.com>
 * (C) 2018.11.20 BiscuitOS   <buddy.biscuitos@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <demo/debug.h>

static int __unused cr2_entence(void)
{
    unsigned long __unused CR2;

    __asm__ ("movl %%cr2, %0" : "=r" (CR2));
    printk("CR2 page-fault linear address: %#lx\n", CR2);

    return 0;
}
late_debugcall(cr2_entence);
