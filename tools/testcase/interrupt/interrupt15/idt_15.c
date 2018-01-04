/*
 * interrupt 15: Intel reserved
 *
 * (C) 2017.10 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>

/* trigger interrupt 15: invoke 'int $0xF' */
#define INT15_SOFTINT         0x01

/*
 * trigger interrupt 15: invoke 'int $0xF'
 * Note! whatever interrupt is enable or disable, this routine
 * will trigger interrupt 15.
 */
#ifdef INT15_SOFTINT
void trigger_interrupt15(void)
{
    printk("Test interrupt 15: Intel reserved "
                              "[invoke 'int $0xF']\n");
    __asm__ ("int $0xF");
}
#endif
