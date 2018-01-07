/*
 * Interrupt 8: Double Fault Exception (#DF) by soft-interrupt
 *
 * (C) 2018.01 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>

/*
 * trigger interrupt 8: invoke 'int $0x8'
 *   Note! This routine will trigger interrupt 8 whatever interrupt is
 *   enable or disable. 
 */
void trigger_interrupt8(void)
{
    printk("Trigger interrupt 8: duble fault.\n");
    __asm__ ("int $8");
}
