/*
 * Interrupt 16: X87 FPU Floating-Point Error (#TS) by soft-interrupt
 *
 * (C) 2018.01 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>

/*
 * Trigger interrupt 16: invoke 'int $0x10'
 *   Note! whatever interrupt is enable or disable, this routine
 *   will trigger interrupt 16.
 */
void trigger_interrupt16(void)
{
    printk("Trigger interrupt 16: Coprocessor error "
                              "[invoke 'int $0x10']\n");
    __asm__ ("int $0x10");
}
