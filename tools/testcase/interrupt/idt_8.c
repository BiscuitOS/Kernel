/*
 * interrupt 8: double fault
 *
 * (C) 2017.10 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>

/*
 * Test Interrupt 8 - double fault
 * On the x86 architecture, a double fault exception occurs if the 
 * processor encounters a problem while trying to service a pending 
 * interrupt or exception. An example situation when a double fault 
 * would occur is when an interrupt is triggered but the segment in 
 * which the interrupt handler resides is invalid. If the processor 
 * encounters a problem when calling the double fault handler, a triple
 * fault is generated and the processor shuts down.
 * As double faults can only happen due to kernel bugs, they are rarely 
 * caused by user space programs in a modern protected mode operating
 * system, unless the program somehow gains kernel access (some viruses
 * and also some low-level DOS programs). Other processors like PowerPC 
 * or SPARC generally save state to predefined and reserved machine 
 * registers. A double fault will then be a situation where another 
 * exception happens while the processor is still using the contents of 
 * these registers to process the exception. SPARC processors have four 
 * levels of such registers, i.e. they have a 4-window register system.
 */

/* trigger interrupt 8: invoke 'int $0x8' */
#define INT8_SOFTIDT        0x01

/*
 * trigger interrupt 8: invoke 'int $0x8'
 * Note! This routine will trigger interrupt 8 whatever interrupt is
 * enable or disable. 
 */
#ifdef INT8_SOFTIDT
void trigger_interrupt8(void)
{
    printk("Test interrupt 8: duble fault.\n");
    __asm__ ("int $8");
}
#endif
