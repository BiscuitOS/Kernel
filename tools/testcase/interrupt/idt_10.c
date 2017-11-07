/*
 * interrupt 10: invalid TSS segment
 *
 * (C) 2017.10 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>

/* trigger interrupt 10: invoke 'int $0x10' */
#define INT10_SOFTINT         0x01

/*
 * trigger interrupt 10: invoke 'int $0xA'
 * Note! whatever interrupt is enable or disable, this routine
 * will trigger interrupt 10.
 */
#ifdef INT10_SOFTINT
void trigger_interrupt10(void)
{
    printk("Test interrupt 10: invalid TSS segment "
                               "[invoke 'int $0xA']\n");
    __asm__ ("int $0xA");
}
#endif
