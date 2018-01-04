/*
 * Interrupt 0: divide zero
 *
 * (C) 2017.10 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <test/debug.h>

/*
 * Indicates the divisor operand for a DIV or IDIV instruction is 0 or that
 * the result cannot be represented in the number of bits specified for
 * the destination operand.
 */
void common_interrupt0(void)
{
    trigger_interrupt0();
}
