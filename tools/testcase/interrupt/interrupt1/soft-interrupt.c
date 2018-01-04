/*
 * Interrupt 1: trigger by soft-interrupt
 *
 * (C) 2018.1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <test/debug.h>

/*
 * trigger interrupt 1 by 'int 0x1'
 * The interrupt 1 will be general when invoke soft-interrupt 'int $0x1'
 * Note! whatever interrupt is enable or disable, 'int $0x1' must trigger
 * interrupt 1.
 */
void trigger_interrupt1(void)
{
    printk("Test interrupt 1: debug ['int $0x1']\n");
    __asm__("int $0x1");

}
