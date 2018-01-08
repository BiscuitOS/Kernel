/*
 * Interrupt 10: irq13
 *
 * (C) 2018.01 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>

/*
 * Trigger interrupt 45: invoke 'int $0x2D'
 *   Note! whatever interrupt is enable or disable, this routine
 *   will trigger interrupt 45.
 */
void trigger_interrupt45(void)
{
    printk("Trigger interrupt 45: irq13 [invoke 'int $0x2D']\n");
    __asm__ ("int $0x2D");
}
