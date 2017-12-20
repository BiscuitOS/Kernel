/*
 * Test XT, AT and PS/2 I/O port addresses
 *
 * (C) 2017.12 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>

#include <test/debug.h>

/* common ioports interface */
int test_common_ioports(void)
{
    printk("Test common ioports.\n");

#ifdef CONFIG_TESTCASE_PORT_0X70
    debug_cmos_ram_common();
#endif
    return 0;
}

