/*
 * Interrupt 2: NMI interrupt
 *
 * (C) 2017.10 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>

/* Trigger interrupt 2:
 * Invoke 'int $0x2' to trigger interrupt 2, interrupt 2 is NMI.
 * NMI(Uon Mask Interrupt) will not be mask when set IF bit on EFLAGS, so
 * NMI will be triggered whatever soft-interrupt or NMI signal.
 */
void trigger_interrupt2(void)
{
    printk("test interrupt 2: invoke 'int $0x02'\n");
    __asm__ ("int $0x2");
}
