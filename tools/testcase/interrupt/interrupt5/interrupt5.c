/*
 * interrupt 5: Bound Range Exceeded Exception (#BR)
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
 * Trigger Interrupt 5 - Bound error
 *   Determines if the first operand (array index) is within the bounds of
 *   an array specified the second operand (bounds operand). The array 
 *   index is a signed integer located in a register. The bounds operand 
 *   is a memory location that contains a pair of signed doubleword-integers 
 *   (when the operand-size attribute is 32) or a pair of signed word-integers
 *   (when the operand-size attribute is 16). The first doubleword (or word) 
 *   is the lower bound of the array and the second doubleword (or word) 
 *   is the upper bound of the array. The array index must be greater than 
 *   or equal to the lower bound and less than or equal to the upper bound
 *   plus the operand size in bytes.
 *   If the index is not within bounds, a BOUND range exceeded exception 
 *   (#BR) is signaled. When this exception is generated, the saved return 
 *   instruction pointer points to the BOUND instruction.
 *   The bounds limit data structure (two words or doublewords containing 
 *   the lower and upper limits of the array) is usually placed just before
 *   the array itself, making the limits addressable via a constant offset 
 *   from the beginning of the array. Because the address of the array already
 *   will be present in a register, this practice avoids extra bus cycles 
 *   to obtain the effective address of the array bounds.
 */
void common_interrupt5(void)
{
    trigger_interrupt5();
}
