/*
 * Interrupt 39: Parallel Interrupt by soft-interrupt
 *
 * (C) 2018.01 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>

/*
 * Trigger interrupt 39: invoke 'int $0x27'
 *   Note! whatever interrupt is enable or disable, this routine
 *   will trigger interrupt 39.
 */
void trigger_interrupt39(void)
{
    printk("Trigger interrupt 39: parallel interrupt "
                             "[invoke 'int $0x27']\n");
    __asm__ ("int $0x27");
}
