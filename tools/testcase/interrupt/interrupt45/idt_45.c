/*
 * interrupt 45: irq13
 *
 * (C) 2017.10 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>

/* trigger interrupt 45: invoke 'int $0x2D' */
#define INT45_SOFTINT         0x01

/*
 * trigger interrupt 45: invoke 'int $0x2D'
 * Note! whatever interrupt is enable or disable, this routine
 * will trigger interrupt 45.
 */
#ifdef INT45_SOFTINT
void trigger_interrupt45(void)
{
    printk("Test interrupt 45: irq13 [invoke 'int $0x2D']\n");
    __asm__ ("int $0x2D");
}
#endif
