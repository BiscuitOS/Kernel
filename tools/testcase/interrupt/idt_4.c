/*
 * interrupt 4: overflow error
 *
 * (C) 2017.10 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>

/*
 * Test Interrupt 4 - overflow error
 * The type 4 interrupt is dedicated to handle overflow conditions. 
 * There are two ways by which a type 4 interrupt can be generated: 
 * either by 'int4' or by 'into' . Like the breakpoint interrupt, 
 * 'into' requires only one byte to encode, as it does not require 
 * the specification of the interrupt type number as part of the 
 * instruction. Unlike 'int4', which unconditionally generates a 
 * type 4 interrupt, 'into' generates a type 4 interrupt only if the
 * overflow flag is set. We do not normally use 'into' , as the 
 * overflow condition is usually detected and processed by using 
 * the conditional jump instructions 'jo' and 'jno'.
 */

/* trigger interrupt 4: invoke 'into' */
//#define INT4_INTO       0x01

/* trigger interrupt 4: invoke 'int $0x4' */
#define INT4_SOFTINT    0x02

/* Trigger interrupt 4: invoke 'into'
 * 'into' requires only one byte to encode, as it does not require 
 * the specification of the interrupt type number as part of the 
 * instruction. Unlike 'int4', which unconditionally generates a 
 * type 4 interrupt, 'into' generates a type 4 interrupt only if the
 * overflow flag is set. We do not normally use 'into' , as the 
 * overflow condition is usually detected and processed by using 
 * the conditional jump instructions 'jo' and 'jno'.
 */
#ifdef INT4_INTO
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
#endif

/*
 * Trigger interrupt 4: invoke 'int $0x4'
 * This routine will trigger interrupt 4 whatever interrupt is
 * enable or disable.
 */
#ifdef INT4_SOFTINT
void trigger_interrupt4(void)
{
    printk("Test interrupt 4: invoke 'int $0x04'\n");
    __asm__ ("int $0x04");
}
#endif
