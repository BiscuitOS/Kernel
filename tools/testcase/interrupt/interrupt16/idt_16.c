/*
 * interrupt 16: Coprocessor error
 *
 * (C) 2017.10 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>

/* trigger interrupt 16: invoke 'int $0x10' */
#define INT16_SOFTINT         0x01

/*
 * trigger interrupt 16: invoke 'int $0x10'
 * Note! whatever interrupt is enable or disable, this routine
 * will trigger interrupt 16.
 */
#ifdef INT16_SOFTINT
void trigger_interrupt16(void)
{
    printk("Test interrupt 16: Coprocessor error "
                              "[invoke 'int $0x10']\n");
    __asm__ ("int $0x10");
}
#endif
