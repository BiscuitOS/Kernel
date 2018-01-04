/*
 * Testcase for Interrupt
 *
 * (C) 2018.1 <buddy.zhang@aliyun.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <test/debug.h>

int debug_interrupt_common(void)
{
#ifdef CONFIG_DEBUG_INT_USAGE
    interrupt_useage_common();
#endif

#ifdef CONFIG_DEBUG_INTERRUPT0
    common_interrupt0();
#endif
    return 0;
}
