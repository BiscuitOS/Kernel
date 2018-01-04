/*
 * interrupt 12: Stack segment
 *
 * (C) 2017.10 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>

/* trigger interrupt 12: invoke 'int $0xC' */
#define INT12_SOFTINT         0x01

/*
 * trigger interrupt 12: invoke 'int $0xC'
 * Note! whatever interrupt is enable or disable, this routine
 * will trigger interrupt 12.
 */
#ifdef INT12_SOFTINT
void trigger_interrupt12(void)
{
    printk("Test interrupt 12: Stack segment "
                              "[invoke 'int $0xC']\n");
    __asm__ ("int $0xC");
}
#endif
