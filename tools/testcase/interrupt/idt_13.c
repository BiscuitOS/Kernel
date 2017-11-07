/*
 * interrupt 13: General protection
 *
 * (C) 2017.10 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>

/* trigger interrupt 13: invoke 'int $0xD' */
#define INT13_SOFTINT         0x01

/*
 * trigger interrupt 13: invoke 'int $0xD'
 * Note! whatever interrupt is enable or disable, this routine
 * will trigger interrupt 13.
 */
#ifdef INT13_SOFTINT
void trigger_interrupt13(void)
{
    printk("Test interrupt 13: General protection "
                              "[invoke 'int $0xD']\n");
    __asm__ ("int $0xD");
}
#endif
