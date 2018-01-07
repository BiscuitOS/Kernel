/*
 * Interrupt 11: Segment Not Present (#NP)
 *
 * (C) 2018.01 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>

/*
 * Trigger interrupt 11: invoke 'int $0xB'
 *   Note! whatever interrupt is enable or disable, this routine
 *   will trigger interrupt 11.
 */
void trigger_interrupt11(void)
{
    printk("Trigger interrupt 11: Segment not present "
                              "[invoke 'int $0xB']\n");
    __asm__ ("int $0xB");
}
