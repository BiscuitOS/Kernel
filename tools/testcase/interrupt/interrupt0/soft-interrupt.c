/*
 * Interrupt 0: trigger by soft-interrupt
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
 * Trigger interrupt 0: invoke 'int $0'
 * The system will trigger interrupt 0 when execute 'int 0'. 
 * Note! for this case, whatever interrupt is enable or disable,
 * 'int $0x0' will trigger interrupter 0.
 */
void trigger_interrupt0(void)
{
    printk("Test interrupt 0: divide zero [invoke 'int $0x0']\n");
    __asm__ ("int $0");
}
