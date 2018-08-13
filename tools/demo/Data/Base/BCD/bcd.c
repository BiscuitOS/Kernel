/*
 * BCD: Unpacked BCD and Packed BCD 
 *
 * (C) 2018.08.10 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <demo/debug.h>

/*
 * Unpacked BCD
 *
 * Each numeral is encoded into one byte, with four bits representing the
 * and the remaining bits having no significance.
 */
static __unused int Unpacked_BCD(void)
{
    unsigned short __unused AX;
    unsigned char  __unused CF;

#ifdef CONFIG_DEBUG_UNPACKED_BCD_ADD
    /*
     * Addition with BCD
     *
     * It is possible to perform addition in BCD by first adding in binary, 
     * and then converting to BCD afterwards. Conversion of the simple sum 
     * of two digits can be done by adding 6 (that is, 16 - 10) when the 
     * five-bit result of adding a pair of digits has a value greater than 9. 
     * For example:
     *
     *   1001 + 1000 = 10001
     *      9 +    8 = 17
     *
     * Note that `10001` is the binary, not decimal, representation of the 
     * desired result. Also note that it cannot fit in a 4-bit number. In 
     * BCD as in decimal, there cannot exist a value greater than 9 (1001) 
     * per digit. To correct this, 6 (0110) is added to that sum and then 
     * the result is treated as two nibbles:
     *
     *   10001 + 0110 = 00010111 => 0001 0111
     *      17 +    6 =       23       1    7
     *
     */
    __asm__ ("mov $0x9, %%ax\n\r"
             "add $0x8, %%al\n\r"
             "aaa\n\r"
             "jc CF_SET\n\r"
             "jnc CF_CLEAR\n\r"
      "CF_SET:\n\r"
             "mov $1, %%ebx\n\r"
             "jmp out\n\r"
    "CF_CLEAR:\n\r"
             "mov $0, %%ebx\n\r"
         "out:\n\r"
             "mov %%bx, %0\n\r"
             "mov %%ax, %1" : "=m" (CF), "=m" (AX) :);
    if (CF)
        printk("Unpacked BCD: 0x9 + 0x8 = %#x\n", AX);
#endif

#ifdef CONFIG_DEBUG_UNPACKED_BCD_SUB
    /*
     * Subtraction with BCD
     *   Subtraction is done by adding the ten's complement of the subtrachend. 
     *   To represent the sign of a number in BCD, the number `00000` is used 
     *   to represent a positive number, and `1001` is used to represent a 
     *   negative number. The remaining 14 combinations are invalid signs. To 
     *   illustrate signed BCD subtraction, consider the following problem: 
     *   357 - 432.
     *
     *   In signed BCD, 357 is 0000 0011 0101 0111. The ten's complement of 
     *   432 can be obtained by taking the nine's complement of 432, and then 
     *   adding one. So, 999 - 432 = 567, and 567 + 1 = 568. By preceding 568 
     *   in BCD by the negative sign code, the number -432 can be represented. 
     *   So, - 432 in signed BCD is 1001 0101 0110 1000.
     *
     *   Now that both numbers are represented in signed BCD, they can be added
     *   together:
     *
     *     0000 0011 0101 0111
     *        0    3    5    7
     *   + 1001 0101 0110 1000
     *        9    5    6    8
     *   ---------------------
     *   = 1001 1000 1011 1111
     *        9    8   11   15
     *
     *   Since `BCD` is a form of decimal representation, several of the digit 
     *   sums above are invalid. In the event that an invalid entry (any BCD 
     *   digit greater than `1001`) exists, 6 is added to generate a carry bit 
     *   and cause the sum to become a valid entry. The reason for adding 6 is
     *   that there are 16 possible 4-bit BCD values (since $$ 2^4 $$ = 16), 
     *   but only 10 values are valid (0000 through 1001). So adding 6 to 
     *   the invalid entries results in the following:
     *  
     *     1001 1000 1011 1111
     *        9    8   11   15
     *   + 0000 0000 0110 0110
     *        0    0    6    6
     *   ---------------------
     *   = 1001 1001 0010 0101
     *        9    9    2    5
     *
     *   Thus the result of the subtraction is 1001 1001 0010 0101 (-925). 
     *   To check the answer, note that the first digit is 9, which means 
     *   negative. This seems to be correct, since `357 - 432` should result 
     *   in a negative number. To check the rest of the digits, represent 
     *   them in decimal. `1001 0010 0101` is 925. The ten's complement of 
     *   925 is 1000 - 925 = 999 - 925 + 1 = 074 + 1 = 75, so the calculated 
     *   answer is `-75`. To check, perform standard subtraction to verify 
     *   that  357 - 432 is -75.
     *
     *   Note that in the event that there are a different number of nibbles 
     *   being added together (such as `1053 - 122`), the number with the 
     *   fewest digits must first be padded with zeros before taking the 
     *   ten's complement or subtracting. So, with `1053 - 122`, 122 would 
     *   have to first be represented as `0122`, and the ten's complement 
     *   of `0122` would have to be calculated.
     */
    __asm__ ("mov $357, %%ax\n\r"
             "sub $432, %%ax\n\r"
             "aas\n\r"
             "jc CF_SET\n\r"
             "jnc CF_CLEAR\n\r"
      "CF_SET:\n\r"
             "mov $1, %%ebx\n\r"
             "jmp out\n\r"
    "CF_CLEAR:\n\r"
             "mov $0, %%ebx\n\r"
         "out:\n\r"
             "mov %%bx, %0\n\r"
             "mov %%ax, %1\n\r" : "=m" (CF), "=m" (AX) :);

    if (CF)
        printk("Unpacked BCD: 357 - 432 = %#x\n", AX); 

#endif
    return 0;
}

static int debug_BCD(void)
{
#ifdef CONFIG_DEBUG_UNPACKED_BCD
    Unpacked_BCD();
#endif

    return 0;
}
late_debugcall(debug_BCD);
