/*
 * Interrupt 6 (#UD): Trigger #UD by BOUND
 *
 * (C) 2018.01 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>

/* 
 * Trigger interrupt 6 (#UD): The second operand is not a memory location
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
 */
void trigger_interrupt6(void)
{
    int buffer[2] = { 0, 6 };
    int index = 7; /* safe value: 0, 1, 2, 3, 4, 5, 6 */

    printk("Trigger interrupt 6 (#UD): BOUND\n");
    /* If the second operand is not a memory location */
    __asm__ ("lea %0, %%edx\n\t"
             "movl %1, %%eax\n\t"
             "boundl %%eax, %0"
             :: "a" (buffer), "m" (index));
}
