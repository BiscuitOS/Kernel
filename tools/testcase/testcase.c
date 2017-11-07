/*
 * Testcae main entry
 *
 * (C) 2017.10 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>

#include <test/testcase.h>

void testcase_init(void)
{
    printk("BiscuitOS auto-TestCase.\n");
    /* function entry */
#ifdef CONFIG_TESTCASE_IDT
    interrupt_main();
#endif
}
