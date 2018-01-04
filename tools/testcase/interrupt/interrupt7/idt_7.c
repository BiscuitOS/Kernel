/*
 * interrupt 7: device not available
 *
 * (C) 2017.10 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>

/* trigger interrupt 7: invoke 'int $0x7' */
#define INT7_SOFTINT         0x01

/*
 * trigger interrupt 7: invoke 'int $0x6'
 * Note! whatever interrupt is enable or disable, this routine
 * will trigger interrupt 7.
 */
#ifdef INT7_SOFTINT
void trigger_interrupt7(void)
{
    printk("Test interrupt 7: invoke 'int $0x7'\n");
    __asm__ ("int $7");
}
#endif
