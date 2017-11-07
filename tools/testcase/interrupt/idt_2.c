/*
 * Interrupt 2: NMI interrupt
 *
 * (C) 2017.10 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>

/*
 * Test Interrupt 2 - NMI
 * In computing, a non-maskable interrupt (NMI) is a hardware interrupt 
 * that standard interrupt-masking techniques in the system cannot ignore. 
 * It typically occurs to signal attention for non-recoverable hardware 
 * errors. (Some NMIs may be masked, but only by using proprietary methods
 * specific to the particular NMI.)
 * An NMI is often used when response time is critical or when an interrupt
 * should never be disabled during normal system operation. Such uses 
 * include reporting non-recoverable hardware errors, system debugging
 * and profiling, and handling of special cases like system resets.
 */

/* trigger interrupt 2: soft-interrupt on enable interrupt */
#define INT2_SOFTINT       0x01

/* Trigger interrupt 2:
 * Invoke 'int $0x2' to trigger interrupt 2, interrupt 2 is NMI.
 * NMI(Uon Mask Interrupt) will not be mask when set IF bit on EFLAGS, so
 * NMI will be triggered whatever soft-interrupt or NMI signal.
 */
#ifdef INT2_SOFTINT
void trigger_interrupt2(void)
{
    printk("test interrupt 2: invoke 'int $0x02'\n");
    __asm__ ("int $0x2");
}
#endif
