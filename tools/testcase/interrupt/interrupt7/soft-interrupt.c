/*
 * Interrupt 7: Trigger Device Not Available Exception by soft-interrupt
 *
 * (C) 2018.01 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>

/*
 * trigger interrupt 7: invoke 'int $0x7'
 *   Note! whatever interrupt is enable or disable, this routine
 *   will trigger interrupt 7.
 */
void trigger_interrupt7(void)
{
    printk("Trigger interrupt 7: invoke 'int $0x7'\n");
    __asm__ ("int $7");
}
