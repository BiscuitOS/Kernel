/*
 * Interrupt 46: hard disk interrupt trigger by soft-interrupt
 *
 * (C) 2018.01 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>

/*
 * Trigger interrupt 46: invoke 'int $0x2E'
 *   Note! whatever interrupt is enable or disable, this routine
 *   will trigger interrupt 46.
 */
void trigger_interrupt46(void)
{
    printk("Trigger interrupt 46: hard disk interrupt "
                             "[invoke 'int $0x2E']\n");
    __asm__ ("int $0x2E");
}
