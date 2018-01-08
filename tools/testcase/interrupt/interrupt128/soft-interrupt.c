/*
 * Interrupt 128: system call trigger by soft-interrupt
 *
 * (C) 2018.01 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>

/*
 * Trigger interrupt 128: invoke 'int $0x80'
 *   Note! whatever interrupt is enable or disable, this routine
 *   will trigger interrupt 128.
 */
void trigger_interrupt128(void)
{
    printk("Trigger interrupt 128: system call [invoke 'int $0x80']\n");
    __asm__ ("int $0x80");
}
