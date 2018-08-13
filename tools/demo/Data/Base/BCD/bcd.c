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
#ifdef CONFIG_DEBUG_UNPACKED_BCD_ADD
    __asm__ ("mov $0x9, %%ax\n\r"
             "add $0x8, %%al\n\r"
             "aaa\n\r"
             "mov %%ax, %0" : "=m" (AX) :);

    printk("Unpacked BCD: 0x9 + 0x8 = %#x\n", AX);
#endif

#ifdef CONFIG_DEBUG_UNPACKED_BCD_SUB
    ;
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
