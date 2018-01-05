/*
 * interrupt 2: NMI Interrupt
 *
 * (C) 2018.01 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <test/debug.h>

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
void common_interrupt2(void)
{
    trigger_interrupt2();
}
