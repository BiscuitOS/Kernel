/*
 * interrupt 17: Intel Reserved
 *
 * (C) 2017.10 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>

/* trigger interrupt 17: invoke 'int $0x11' */
#define INT17_SOFTINT         0x01

/*
 * trigger interrupt 17: invoke 'int $0x11'
 * Note! whatever interrupt is enable or disable, this routine
 * will trigger interrupt 17.
 */
#ifdef INT17_SOFTINT
void trigger_interrupt17(void)
{
    printk("Test interrupt 17: Intel Reserved "
                             "[invoke 'int $0x11']\n");
    __asm__ ("int $0x11");
}
#endif
