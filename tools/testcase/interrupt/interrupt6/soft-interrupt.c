/*
 * Interrupt 5: trigger Invalid operand (#UD) by soft-interrupt
 *
 * (C) 2018.01 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>

/*
 * trigger interrupt 6: invoke 'int $0x6'
 *   Note! whatever interrupt is enable or disable, this routine
 *   will trigger interrupt 6.
 */
void trigger_interrupt6(void)
{
    printk("Trigger interrupt 6: invoke 'int $0x6'\n");
    __asm__ ("int $6");
}
