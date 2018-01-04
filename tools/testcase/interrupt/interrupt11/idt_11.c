/*
 * interrupt 11: Segment not present
 *
 * (C) 2017.10 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>

/* trigger interrupt 11: invoke 'int $0xB' */
#define INT11_SOFTINT         0x01

/*
 * trigger interrupt 11: invoke 'int $0xB'
 * Note! whatever interrupt is enable or disable, this routine
 * will trigger interrupt 11.
 */
#ifdef INT11_SOFTINT
void trigger_interrupt11(void)
{
    printk("Test interrupt 11: Segment not present "
                              "[invoke 'int $0xB']\n");
    __asm__ ("int $0xB");
}
#endif
