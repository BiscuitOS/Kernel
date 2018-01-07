/*
 * Interrupt 9: Coprocessor Segment Overrun by soft-interrupt
 *
 * (C) 2018.01 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>

/*
 * trigger interrupt 9: invoke 'int $0x9'
 *   Note! whatever interrupt is enable or disable, this routine
 *   will trigger interrupt 9.
 */
void trigger_interrupt9(void)
{
    printk("Trigger interrupt 9: Coprocessor segment overrun "
                              "[invoke 'int $0x9]'\n");
    __asm__ ("int $9");
}
