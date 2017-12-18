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

#define INTEL8253_BASE           0x40
#define PORT_COUNTER0            0x40
#define PORT_COUNTER1            0x41
#define PORT_COUNTER2            0x42
#define PORT_CTRL                0x43

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

enum
{
    DATA_LATCH, 
    DATA_MSB,   /* Read/Load MSB only */
    DATA_LSB,   /* Read/Load LSB only */
    DATA_ALL    /* Read/Load LSB first, then MSB */
};

enum
{
    F_16BIT,
    F_BCD
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
 *    | 0 | Binary Counter 16Bits                                    |
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
 * READ/LOAD
 *   LSB --- Least significant byte
 *   MSB --- Most significant byte.
 *   
 *     RL1   RL2   
 *   --------------------------------------------------------
 *   |  0  |  0  | Counter Latching operation               |
 *   --------------------------------------------------------
 *   |  0  |  1  | Read/Load most significant byte only     |
 *   --------------------------------------------------------
 *   |  1  |  0  | Read/Load least significant byte only    |
 *   --------------------------------------------------------
 *   |  1  |  1  | Read/Load least significant byte first,  |
 *   |     |     | then most significant byte.              |
 *   --------------------------------------------------------
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
    outb_p(ctr, PORT_CTRL);
}

/*
 * READ OPERATIONS
 *   In most counter applications it becomes neccssary to read the value
 *   of the count in progress and make a computation decision based on 
 *   this quantity. Event counters are probably the most common application
 *   that uses this functions. The 8253 contains logic that will allow
 *   the programmer to easily read the contents of any of the three 
 *   counters without disturbing the actual count in progress.
 *
 *   There are two methods that the programmer can use to read the value
 *   of the counters. The first method involves the use of simple I/O
 *   read operations of the selected counter. By controlling the A0, A1
 *   inputs to the 8253 the programmer can select the counter to be read
 *   (remember that no read operation of the mode register is allowed A0,
 *   A1 -- 11). The only requirement with this method is that in order to 
 *   assure a stable count reading the actual operation of the selected 
 *   counter must be inhibited either by controlling the Gate input or by
 *   external logic that inhibits the clock input. The contents of the 
 *   counter selected will be available as follow:
 *
 *   First I/O Read contains the least significant byte (LSB)
 * 
 *   Second I/O Read contains the most significant byte (MSB)
 *
 *   Due to the internal logic of the 8253 it is absolutely necessary
 *   to complete the entire reading procedure. If two bytes are programmed
 *   to be read, then two bytes must be read before any loading WR command 
 *   can be sent to the same counter.
 */
static unsigned short intel8253_read(unsigned int nr)
{
    unsigned short value;

    value = inb_p(INTEL8253_BASE + nr);
    value |= inb_p(INTEL8253_BASE + nr) << 8;
    return value;
}

/*
 * WRITE PROCEDURE
 *   The systems software must program each counter of the 8253 with the
 *   mode and quantity desired. The programmer must write out ot the 8253
 *   a MODE control word and the programmed number of count register 
 *   bytes (1 or 2) prior to actually using the selected counter.
 *
 *   The actual order of the programming is quite flexible. Writing out
 *   of the MODE countrol word can be in any sequence of counter 
 *   selection, eg, counter0 does't have to be first or counter2 has a 
 *   separate address so that its loading is completely sequence 
 *   independent. (SC0, SC1).
 *
 *   The loading of the Count Register with the actual count value, however,
 *   must be done in exactly word (RL0, RL1). This loading of the counter's
 *   count register is still sequence independent like the MODE control
 *   word loading, but when a selected count register is to be loaded it 
 *   must be loaded with the number of bytes programmed in the MODE control
 *   word (RL0, RL1). The one or two bytes to be loaded in the count 
 *   counter do not have to follow the associated MODE control word. They
 *   can be programmed at any time following the MODE control word loading
 *   as long as the correct number of bytes is loaded in order.
 *  
 *   All counters are down counters. Thus, the value loaded into the count
 *   register will actually be decremented. Loading all zeros into a count
 *   register will result in the maximum count (2^16 for Binary or 10^4 for
 *   BCD). In MODE 0 the new count will not restart until the load has been
 *   completed. It will accept one of two bytes depending on how the MODE
 *   control words (RL0, RL1) are programmed. Then proceed with the restart
 *   operation.
 */
/* Only write MSB of data to counter Register */
static int intel8253_write_MSB(unsigned int nr, unsigned char data)
{
    outb_p(data, INTEL8253_BASE + nr);
    return 0;
}

/* Only write LSB of data to counter Register  */
static int intel8253_write_LSB(unsigned int nr, unsigned char data)
{
    outb(data, INTEL8253_BASE + nr);
    return 0;
}

/* Write LSB first and then MSB of data to counter Register */
static int intel8253_write(unsigned int nr, unsigned short data)
{
    outb_p(data & 0xFF, INTEL8253_BASE + nr);
    outb_p((data >> 8) & 0xFF, INTEL8253_BASE + nr);
    return 0;
}

/* common entry on 8253 */
void debug_8253_common(void)
{
    /* Add item */

    /* Ignor warning */
    if (0) {
        unsigned short value;

        /* Counter1, LSB/MSB, ONE_SHOT, 16Bit */
        intel8253_write_ctr(INTEL8253_COUNTER2, MODE0_INTERRUPT, 
                            DATA_ALL, F_16BIT);
        /* Write LSB data */
        intel8253_write_LSB(INTEL8253_COUNTER2, 0x20);
        /* Write MSB data */
        intel8253_write_MSB(INTEL8253_COUNTER2, 0x11);
        /* Read from Counter 1 */
        value = intel8253_read(PORT_COUNTER2);
        printk("Value1 %#x\n", value);

        /* Re-Write new value */
        intel8253_write(INTEL8253_COUNTER2, 0x1102);
        value = intel8253_read(PORT_COUNTER2);
        printk("Value2 %#x\n", value);
    }
}
