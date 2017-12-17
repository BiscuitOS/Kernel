/*
 * Debug Intel 8253 Timer
 *
 * (C) 2017.12 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation. 
 */
#include <linux/kernel.h>
#include <asm/io.h>

#include <test/debug.h>

#define INTEL8253_BASE      0x40
#define CNT1_REG            0x40
#define CNT2_REG            0x41
#define CNT3_REG            0x42
#define CTR_REG             0x43

enum 
{
    MODE0_INTERRUPT,
    MODE1_ONE_SHOT,
    MODE2_RATE,
    MODE3_SW_RATE,
    MODE4_SF_TRIGGER,
    MODE5_HD_TROIGGER  
};

enum
{
    INTEL8253_COUNTER0,
    INTEL8253_COUNTER1,
    INTEL8253_COUNTER2
};
/*
 * The complete function definition of the 8253 is programmed by
 * the system software. A set of control words must be sent out
 * by the CPU to initialize each counter of the 8253 with the desired
 * MODE and quantity information. Prior to initialization, the 
 * MODE, count, and output of all counters is undefined. These
 * control words program the MODE, Loading sequence and selection of
 * binary or BCD counting.
 *
 * Once programmed, the 8253 is ready to perform whatever timing tasks
 * it is assigned to accomplish.
 *
 * The actual counting operation of each counter is completely independent
 * and additional logic is provided on-chip so that the usual problems
 * associated with efficient monitoring and management of external,
 * asynchronous events or rates to the microcomputer system have been
 * eliminated.
 */

/*
 * Programming the 8253
 *   All of the MODES for each counter are programmed by the systems
 *   software by simple I/O operations.
 * 
 *   Each counter of the 8253 is individually programmed by writing a
 *   control word into the Control Word Register. (A0, A1 = 11)
 *
 * Control Word Format
 *     D7    D6    D5    D4    D3   D2   D1   D0
 *   ----------------------------------------------
 *   | SC1 | SC0 | RL1 | RL0 | M2 | M1 | M0 | BCD |
 *   ----------------------------------------------
 *
 *  Definition of Control
 *    SC --- Select counter
 *      SC1   SC2   
 *    --------------------------------
 *    |  0  |  0  | Select Counter 0 |
 *    --------------------------------
 *    |  0  |  1  | Select Counter 1 |
 *    --------------------------------
 *    |  1  |  0  | Select Counter 2 |
 *    --------------------------------
 *    |  1  |  1  | Illegal          |
 *    -------------------------------- 
 *
 *    RL --- READ/LOAD
 *      RL1   RL0
 *    ----------------------------------------------------------------
 *    |  0  |  0  | Counter latching operation                       |
 *    ----------------------------------------------------------------
 *    |  0  |  1  | Read/Load most significant byte only             |
 *    ----------------------------------------------------------------
 *    |  1  |  0  | Read/Load least significant byte only            |
 *    ----------------------------------------------------------------
 *    |  1  |  1  | Read/Load lasst significant byte first,          |
 *    |     |     | then most significant byte                       |
 *    ----------------------------------------------------------------
 * 
 *    M --- MODE
 *      M2    M1    M0
 *    ----------------------------
 *    |  0  |  0  |  0  | Mode 0 |
 *    ----------------------------
 *    |  0  |  0  |  1  | Mode 1 |
 *    ----------------------------
 *    |  X  |  1  |  0  | Mode 2 |
 *    ----------------------------
 *    |  X  |  1  |  1  | Mode 3 |
 *    ----------------------------
 *    |  1  |  0  |  0  | Mode 4 |
 *    ----------------------------
 *    |  1  |  0  |  1  | Mode 5 |
 *    ----------------------------
 *
 *    BCD
 *    ----------------------------------------------------------------
 *    | 0 | Binary Counter 160Bits                                   |
 *    ----------------------------------------------------------------
 *    | 1 | Binary Coded Decimal (BCD) Counter                       |
 *    ----------------------------------------------------------------
 */

/*
 * Counter MODE
 *   MODE 0: Interrupt on Terminal Count.
 *     The output will be initially low after the mode set operation.
 *     After the count is loaded into the selected count register, the 
 *     output will remain low and the counter will count. When terminal
 *     counter is reached, the output will go high and remain high until
 *     the selected count register is reloaded with the mode or a new 
 *     count is loaded. The counter continues to decrement after terminal
 *     count has been reached.
 *
 *     Rewriting a counter register during counting results in the follwing:
 *     (1) Write 1st byte stops the current counting.
 *     (2) Write 2nd byte start the new count.
 *
 *   MODE 1: Programmable One-Shot
 *     The output will go low on the count following the rising edage of
 *     the gate input.
 *
 *     The output will go high on the terminal count. If a new count value
 *     is loaded while the output is low it will not affect the duration
 *     of the one-shot pulse until the succeeding trigger. The current can
 *     be read at any time without affecting the one-shot pulse.
 */
static void intel8253_counter_mode(unsigned char *ctl, int mode)
{
    if (mode > 5 || mode < 0) {
        printk("Invalid mode\n");
        return;
    }
    if (mode == 2 || mode == 3)
       *ctl |= (mode & 0x3) << 0x1;
    else
       *ctl |= (mode & 0x7) << 0x1;
}

/*
 * Setting BCD
 */
static void intel8253_counter_BCD(unsigned char *ctl, int BCD)
{
    *ctl |= BCD & 0x1;
}

/*
 * Setting READ/LOAD
 */
static void intel8253_counter_RL(unsigned char *ctl, int rl)
{
    *ctl |= (rl & 0x3) << 0x4;
}

/*
 * Control Register Operation
 * 
 * @nr: The number of counter.
 * @mode: The work mode for counter.
 * @rl:
 * @BCD: 
 */
static void intel8253_write_ctr(unsigned int nr, unsigned int mode,
                                   unsigned int rl, unsigned int BCD)
{
    unsigned char ctr;

    /* Specify counter number */
    ctr = (nr & 0x3) << 0x6; 
    /* Setting Counter mode */
    intel8253_counter_mode(&ctr, mode);
    /* Setting Counter BCD */
    intel8253_counter_BCD(&ctr, BCD);
    /* Setting Counter RL */
    intel8253_counter_RL(&ctr, rl);
    /* Output */
    outb_p(ctr, CTR_REG);
}

/*
 * Read from Counter
 */
static unsigned char intel8253_read(unsigned int nr)
{
    return 0;
}

/*
 * Write to Counter
 */
static int intel8253_write(unsigned int nr, unsigned char data)
{
    return 0;
}

void debug_8253_common(void)
{
    /* Add item */

    /* Ignor warning */
    if (0) {
        intel8253_write_ctr(INTEL8253_COUNTER1, MODE1_ONE_SHOT, 
                            1, 1);
        intel8253_write(1, 1);
        intel8253_read(1);
    }
}
