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

#include <test/mm.h>

/* common memory entry */
int test_mmu(void)
{
    printk("Testcase MMU(Memory Manager Unit.)\n");

    return 0;
}
