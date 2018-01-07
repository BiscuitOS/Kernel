/*
 * Interrupt 5: Trigger BOUND Range Exceeded Exception by soft-interrupt
 *
 * (C) 2018.01 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>

/*
 * Trigger #BR: invoke 'int $0x5'
 *   This routine must be trigger interrupt 5 whatever interrupt is 
 *   enable or disable.
 */
void trigger_interrupt5(void)
{
    printk("Trigger interrupt 5: invoke 'int $0x5'\n");
    __asm__ ("int $0x05");
}
