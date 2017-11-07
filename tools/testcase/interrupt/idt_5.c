/*
 * interrupt 5: Bound error
 *
 * (C) 2017.10 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>

/*
 * Test Interrupt 5 - Bound error
 * Determines if the first operand (array index) is within the bounds of
 * an array specified the second operand (bounds operand). The array 
 * index is a signed integer located in a register. The bounds operand 
 * is a memory location that contains a pair of signed doubleword-integers 
 * (when the operand-size attribute is 32) or a pair of signed word-integers
 * (when the operand-size attribute is 16). The first doubleword (or word) 
 * is the lower bound of the array and the second doubleword (or word) 
 * is the upper bound of the array. The array index must be greater than 
 * or equal to the lower bound and less than or equal to the upper bound
 * plus the operand size in bytes.
 * If the index is not within bounds, a BOUND range exceeded exception 
 * (#BR) is signaled. When this exception is generated, the saved return 
 * instruction pointer points to the BOUND instruction.
 * The bounds limit data structure (two words or doublewords containing 
 * the lower and upper limits of the array) is usually placed just before
 * the array itself, making the limits addressable via a constant offset 
 * from the beginning of the array. Because the address of the array already
 * will be present in a register, this practice avoids extra bus cycles 
 * to obtain the effective address of the array bounds.
 */

/* trigger interrupt 5: invoke 'bound' */
//#define INT5_BOUND           0x01

/* trigger interrupt 5: invoke 'int $0x5' */
#define INT5_SOFTINT         0x02

/* Trigger interrupt 5: invoke 'bound' 
 * Determines if the first operand (array index) is within the bounds of
 * an array specified the second operand (bounds operand). The array 
 * index is a signed integer located in a register. The bounds operand 
 * is a memory location that contains a pair of signed doubleword-integers 
 * (when the operand-size attribute is 32) or a pair of signed word-integers
 * (when the operand-size attribute is 16). The first doubleword (or word) 
 * is the lower bound of the array and the second doubleword (or word) 
 * is the upper bound of the array. The array index must be greater than 
 * or equal to the lower bound and less than or equal to the upper bound
 * plus the operand size in bytes.
 */
#ifdef INT5_BOUND
void trigger_interrupt5(void)
{
    int buffer[2] = { 0, 6 };
    int index = 7; /* safe value: 0, 1, 2, 3, 4, 5, 6 */

    printk("Test interrupt 5: bound array.\n");
    /*
     * Upper = buffer[1]
     * lower = buffer[0]
     * if index < lower || index > upper
     * Invoke Bound interrupt.
     */
    __asm__ ("lea %0, %%edx\n\t"
             "movl %1, %%eax\n\t"
             "boundl %%eax, (%%edx)"
             :: "m" (buffer), "m" (index));
}
#endif

/*
 * Trigger interrupt 5: invoke 'int $0x5'
 * This routine must be trigger interrupt 5 whatever interrupt is 
 * enable or disable.
 */
#ifdef INT5_SOFTINT
void trigger_interrupt5(void)
{
    printk("Test interrupt 5: invoke 'int $0x5'\n");
    __asm__ ("int $0x05");
}
#endif
