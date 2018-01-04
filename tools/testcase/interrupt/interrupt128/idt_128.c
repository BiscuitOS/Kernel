/*
 * interrupt 128: system call
 *
 * (C) 2017.10 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>

/* trigger interrupt 128: invoke 'int $0x80' */
#define INT128_SOFTINT         0x01

/*
 * trigger interrupt 128: invoke 'int $0x80'
 * Note! whatever interrupt is enable or disable, this routine
 * will trigger interrupt 128.
 */
#ifdef INT128_SOFTINT
void trigger_interrupt128(void)
{
    printk("Test interrupt 128: system call [invoke 'int $0x80']\n");
    __asm__ ("int $0x80");
}
#endif
