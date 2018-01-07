/*
 * interrupt 4: overflow error
 *
 * (C) 2017.10 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <test/debug.h>

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
void common_interrupt4(void)
{
    trigger_interrupt4();
}
