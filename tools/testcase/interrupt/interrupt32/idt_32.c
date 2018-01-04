/*
 * interrupt 32: timer interrupt
 *
 * (C) 2017.10 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>

/* trigger interrupt 32: invoke 'int $0x20' */
#define INT32_SOFTINT         0x01

/*
 * trigger interrupt 32: invoke 'int $0x20'
 * Note! whatever interrupt is enable or disable, this routine
 * will trigger interrupt 32.
 */
#ifdef INT32_SOFTINT
void trigger_interrupt32(void)
{
    printk("Test interrupt 32: timer interrupt [invoke 'int $0x20']\n");
    __asm__ ("int $0x20");
}
#endif
