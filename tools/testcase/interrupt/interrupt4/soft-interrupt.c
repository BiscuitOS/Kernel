/*
 * Interrupt 4: Trigger overflow error by soft-interrupt
 *
 * (C) 2018.01 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>

/*
 * Trigger interrupt 4: invoke 'int $0x4'
 * This routine will trigger interrupt 4 whatever interrupt is
 * enable or disable.
 */
void trigger_interrupt4(void)
{
    printk("Trigger interrupt 4: invoke 'int $0x04'\n");
    __asm__ ("int $0x04");
}

