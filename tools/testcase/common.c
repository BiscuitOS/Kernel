/*
 * Testcae main entry
 *
 * (C) BiscuitOS 2017.10 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>

#include <test/debug.h>

void testcase_init(void)
{
    printk("BiscuitOS auto-TestCase.\n");
    /* function entry */

#ifdef CONFIG_TESTCASE_IDT
    /* Interrupt test */
    interrupt_main();
#endif

#ifdef CONFIG_TESTCASE_SCHED
    /* Scheduler test */
    test_task_scheduler();
#endif

#ifdef CONFIG_TESTCASE_MMU
    /* Memory Mamager Unit test */
    test_mmu();
#endif
}
