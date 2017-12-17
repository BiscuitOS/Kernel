/*
 * Test Common Timer and CMOS clock
 *
 * (C) 2017.12 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>

#include <test/debug.h>

/* common timer interface */
int test_common_timer(void)
{
    printk("Test common Timer.\n");

#ifdef CONFIG_TESTCASE_CMOS_CLK
    debug_cmos_clk_common();
#endif

#ifdef CONFIG_TESTCASE_8253
    debug_8253_common();
#endif

    return 0;
}

