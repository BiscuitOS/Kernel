/*
 * interrupt 46: hard disk interrrupt
 *
 * (C) 2017.10 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>

/* trigger interrupt 46: invoke 'int $0x2E' */
#define INT46_SOFTINT         0x01

/*
 * trigger interrupt 46: invoke 'int $0x2E'
 * Note! whatever interrupt is enable or disable, this routine
 * will trigger interrupt 46.
 */
#ifdef INT46_SOFTINT
void trigger_interrupt46(void)
{
    printk("Test interrupt 46: hard disk interrupt "
                             "[invoke 'int $0x2E']\n");
    __asm__ ("int $0x2E");
}
#endif
