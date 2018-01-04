/*
 * interrupt 14: page fault
 *
 * (C) 2017.10 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>

/* trigger interrupt 14: invoke 'int $0xE' */
#define INT14_SOFTINT         0x01

/*
 * trigger interrupt 14: invoke 'int $0xE'
 * Note! whatever interrupt is enable or disable, this routine
 * will trigger interrupt 14.
 */
#ifdef INT14_SOFTINT
void trigger_interrupt14(void)
{
    printk("Test interrupt 14: page fault "
                             "[invoke 'int $0xE']\n");
    __asm__ ("int $0xE");
}
#endif
