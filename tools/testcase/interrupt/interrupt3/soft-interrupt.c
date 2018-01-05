/*
 * Interrupt 3: Trigger Breakpoint by soft-interrupt
 *
 * (C) 2018.01 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>

/*
 * Trigger interrupt 3: invoke 'int $0x03'
 * The interrupt 3 must be triggered by invoking 'int $0x03'.
 * Note! This routine will be trigger whatever interrupt is 
 * enable or disable.
 */
void trigger_interrupt3(void)
{
    printk("Trigger interrupt 3: invoke 'int $0x3'\n");
    __asm__ ("int $0x03");
}
