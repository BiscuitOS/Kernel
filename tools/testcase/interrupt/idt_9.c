/*
 * interrupt 9: Coprocessor segment overrun
 *
 * (C) 2017.10 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>

/* trigger interrupt 9: invoke 'int $0x9' */
#define INT9_SOFTINT         0x01

/*
 * trigger interrupt 9: invoke 'int $0x9'
 * Note! whatever interrupt is enable or disable, this routine
 * will trigger interrupt 9.
 */
#ifdef INT9_SOFTINT
void trigger_interrupt9(void)
{
    printk("Test interrupt 9: Coprocessor segment overrun "
                              "[invoke 'int $0x9]'\n");
    __asm__ ("int $9");
}
#endif
