/*
 * Interrupt 14: Page-Fault Exception (#PF) by soft-interrupt
 *
 * (C) 2018.01 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>

/*
 * Trigger interrupt 14: invoke 'int $0xE'
 *   Note! whatever interrupt is enable or disable, this routine
 *   will trigger interrupt 14.
 */
void trigger_interrupt14(void)
{
    printk("Trigger interrupt 14: page fault "
                             "[invoke 'int $0xE']\n");
    __asm__ ("int $0xE");
}
