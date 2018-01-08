/*
 * Interrupt 13: General Protection Exception (#GP) by soft-interrupt
 *
 * (C) 2018.01 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>

/*
 * Trigger interrupt 13: invoke 'int $0xD'
 *   Note! whatever interrupt is enable or disable, this routine
 *   will trigger interrupt 13.
 */
void trigger_interrupt13(void)
{
    printk("Trigger interrupt 13: General protection "
                              "[invoke 'int $0xD']\n");
    __asm__ ("int $0xD");
}
