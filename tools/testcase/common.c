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

#ifdef CONFIG_DEBUG_KERNEL_LATER
/*
 * debug on kernel last before userland launch.
 */
void debug_on_kernel_later(void)
#endif
#ifdef CONFIG_DEBUG_KERNEL_EARLY
/*
 * debug on kernel early
 */
void debug_on_kernel_early(void)
#endif
#ifdef CONFIG_DEBUG_USERLAND_EARLY
/*
 * debug on userland launch on early stage
 */
void debug_on_userland_early(void)
#endif
#ifdef CONFIG_DEBUG_USERLAND_SHELL
/*
 * debug on shell stage
 */
void debug_on_shell_stage(void)
#endif
{
    printk("BiscuitOS auto-TestCase.\n");
    /* function entry */

#ifdef CONFIG_TESTCASE_INTERRUPT
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

#ifdef CONFIG_TESTCASE_TIMER
    /* Timer test*/
    test_common_timer();
#endif
}

