/*
 * interrupt 39: parallel interrupt
 *
 * (C) 2017.10 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>

/* trigger interrupt 39: invoke 'int $0x27' */
#define INT39_SOFTINT         0x01

/*
 * trigger interrupt 39: invoke 'int $0x27'
 * Note! whatever interrupt is enable or disable, this routine
 * will trigger interrupt 39.
 */
#ifdef INT39_SOFTINT
void trigger_interrupt39(void)
{
    printk("Test interrupt 39: parallel interrupt "
                             "[invoke 'int $0x27']\n");
    __asm__ ("int $0x27");
}
#endif
