/*
 * Interrupt 4: Trigger overflow error by INTO
 *
 * (C) 2018.01 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>

/* 
 * Trigger interrupt 4: invoke 'into'
 *   'into' requires only one byte to encode, as it does not require 
 *   the specification of the interrupt type number as part of the 
 *   instruction. Unlike 'int4', which unconditionally generates a 
 *   type 4 interrupt, 'into' generates a type 4 interrupt only if the
 *   overflow flag is set. We do not normally use 'into' , as the 
 *   overflow condition is usually detected and processed by using 
 *   the conditional jump instructions 'jo' and 'jno'.
 */
void trigger_interrupt4(void)
{
    printk("Test interrupt 4: invoke 'into'\n");
    /* 'OF' set and call 'into' */
    __asm__("pushl %%ebx\n\t"
            "movb $0x7f, %%bl\n\t"
            "addb $10, %%bl\n\t"
            "into\n\t"
            "popl %%ebx"
            ::);
}

