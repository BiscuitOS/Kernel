/*
 * Interrupt 10: Invalid TSS Exception (#TS)
 *
 * (C) 2018.01 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>

/*
 * Trigger interrupt 10: invoke 'int $0xA'
 *   Note! whatever interrupt is enable or disable, this routine
 *   will trigger interrupt 10.
 */
void trigger_interrupt10(void)
{
    printk("Trigger interrupt 10: invalid TSS segment "
                               "[invoke 'int $0xA']\n");
    __asm__ ("int $0xA");
}
