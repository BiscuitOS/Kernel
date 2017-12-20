/*
 * Debug CMOS RAM
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

#define CMOS_CTR      0x70
#define CMOS_DTA      0x71

#define BCD_TO_BIN(val)  ((val)=((val) & 15) + ((val) >> 4) * 10)

/* Data fomat for RTC */
enum
{
    F_BCD,
    F_BINARY
};

/* Hour format for RTC */
enum
{
    H_12H,
    H_24H
};

/*
 * Organization of CMOS Memory - Clock
 *   00 -- 0E is defined by the clock hardware and all must follow it.
 *   Other manufactures generally follow the same format as specified
 *   for the region 10h - 2Fh. Some also follow the IBM format for 30h-33h
 *   but not all (Zenith in particular is different).
 *
 *   The first fourteen bytes are dedicated to the MC146818 chip clock 
 *   functions and consist of ten read/write data registers and four
 *   status registers, two of which are read/write and two fo which
 *   are read only.
 *
 *   The format of the ten clock data registers (bytes 00h - 09h) is:
 *
 *   ----- R00 -----------------------------------------------
 *   CMOS 00h --- RTC - SECONDS
 *   Desc: BCD 00 - 59, HEX 00 - 3B
 *   Note: Bit 7 is read only
 *   SeeAlso: CMOS 01h, CMOS 02h, CMOS 04h
 *   
 *   ----- R01 -----------------------------------------------
 *   CMOS 01h --- RTC - SECONDS ALARM
 *   Desc: BCD 00 - 59, HEX 00 - 3B
 *   SeeAlso: CMOS 00h, CMOS 03h, CMOS 05h
 *
 *   ----- R02 -----------------------------------------------
 *   CMOS 02h --- RTC - MINUTES
 *   Desc: BCD 00 - 59, HEX 00 - 3B
 *   SeeAlso: CMOS 00h, CMOS 03h, CMOS 05h
 *
 *   ----- R03 -----------------------------------------------
 *   CMOS 03h --- RTC - MINUTE ALARM
 *   Desc: BCD 00 - 59, HEX 00 - 3B
 *   SeeAlso: CMOS 00h, CMOS 02h, CMOS 05h
 *
 *   ----- R04 -----------------------------------------------
 *   CMOS 04h --- RTC - HOURS
 *   Desc: BCD 00 - 23, Hex 00 - 17 if 24 hr mode
 *         BCD 01 - 12, Hex 01 - 0C if 12 hr am
 *         BCD 81 - 92, Hex 81 - 8C if 12 hr pm
 *   SeeAlso: CMOS 00h, CMOS 02h, CMOS 05h
 *
 *   ----- R05 -----------------------------------------------
 *   CMOS 05h --- RTC - HOUR ALARM
 *   Desc: same as hours, don't care if C0 - FF
 *   SeeAlso: CMOS 01h, CMOS 03h, CMOS 04h
 *
 *   ----- R06 -----------------------------------------------
 *   CMOS 06h --- RTC - DAY OF WEEK
 *   Desc: 01 - 07 Sunday = 1
 *   SeeAlso: CMOS 07h, CMOS 08h, CMOS 09h
 *
 *   ----- R07 -----------------------------------------------
 *   CMOS 07h --- RTC - DATE of MONTH
 *   Desc: BCD 01 - 31, HEX 01 - 1F
 *   SeeAlso: CMOS 06h, CMOS 08h, CMOS 09h
 *
 *   ----- R08 -----------------------------------------------
 *   CMOS 08h --- RTC - MONTH
 *   Desc: BCD 01 - 12, HEX 01 - 0C
 *   SeeAlso: CMOS 06h, CMOS 07h, CMOS 09h
 *
 *   ----- R09 -----------------------------------------------
 *   CMOS 09h --- RTC - YEAR
 *   Desc: BCD 00 - 99, HEX 00 - 63
 *   Notes: BCD/HEX selection depends on Bit 2 of register B (0Bh)
 *          12/24 Hr selection depends on Bit 1 of register B (0Bh)
 *          Alarm will trigger when contents of all three Alarm byte 
 *          registers match their companions.
 *   SeeAlso: CMOS 06h, CMOS 07h, CMOS 08h
 *  
 *   ----- R0A -----------------------------------------------
 *   CMOS 0Ah --- RTC - STATUS REGISTER A (read/write)
 *   Bitfields for RTC Status Register A:
 *   Bit(s)  Description   (Table C001)
 *     7     = 1 time update cycle in progress, data output undefined
 *           (bit 7 is read only)
 *     6-4   22 stage divider
 *           010 = 32768 Hz time base (default)
 *     3-0   rate selection bits for interrupt
 *           0000 none
 *           0011 122 microseconds (minimum)
 *           1111 500 milliseconds
 *           0110 976.562 milliseconds (default 1024 Hz)
 *   SeeAlso: #C002, #C003, #C004
 *
 *   ----- R0B -----------------------------------------------
 *   CMOS 0Bh --- RTC - STATUS REGISTER B (read/write)
 *   Bitfields for RTC Status Register B: 
 *   Bit(s)  Description   (Table C002)
 *     7     Enable cycle update
 *     6     Enable periodic interrupt
 *     5     Enable alarm interrupt
 *     4     Enable update-enable interrupt
 *     3     Enable square wave 
 *     2     Data Mode - 0: BCD, 1: Binary
 *     1     24/12 hour selection - 1 enables 24 hour mode
 *     0     Daylight Saving Enable - 1 enables
 *   SeeAlso: #C001, #C004
 *
 *   ----- R0C -----------------------------------------------
 *   CMOS 0Ch --- RTC - STATUS REGISTER C (Read only)
 *   Bitfields for RTC Status Register C:
 *   Bit(s)  Description   (Table C003)
 *     7     Interrupt request flag
 *           =1 when any or all of bits 6-4 are 1 and appropriate 
 *           enables (Register B) are set to 1. Generates IRQ 8 
 *           when triggered.
 *     6     Periodic Interrupt flag
 *     5     Alarm Interrupt flag
 *     4     Update-Ended Interrupt Flag
 *     3-0   unused
 *   SeeAlso: #C001, #C002, #C004
 *
 *   ----- R0D -----------------------------------------------
 *   CMOS 0Dh --- RTC - STATUS REGISTER D (read only)
 *   Bit(s)  Description     (Table C004)
 *     7     Valid RAM - 1 indicates batery power good,
 *                       0 if dead or disconnected.
 *     6-0   unused
 *
 * Organization of CMOS Memory - non-Clock
 *   This last two bytes in the first hexadecimal decade (hexade ?)
 *   were not specified in the PC/AT but may have the following use
 *   on some systems:
 *
 *   ----- R0D -----------------------------------------------
 *   CMOS 0Eh --- IBM PS/2 - DIAGNOSTIC STATUS BYTE
 *   Bitfields for IBM PS/2 diagnostic status byte:
 *   Bit(s)  Description     (Table C005)
 *     7     indicates clock has last power,
 *           =1 RTC lost power, =0 power stat stable
 *     6     indicates checksum
 *           =1 config record checksum is bad, =0 ok
 *     5     equipment configuration is incorrect (power-on check
 *           requires that atleast one floppy be installed)
 *           =1 invalid config info found, =0 ok
 *     4     error in memory size
 *           =1 memory size doesn't match config info, =0 ok
 *     3     controller or disk driver failed initialization
 *           =1 fixed disk 0 failed initialized, =0 ok
 *     2     time is invalid
 *           =1 time is invalid, =0 ok (POST validity check)
 *     1     installed adaptors do not match configuration
 *     0     time-out while reading adapter ID
 *     
 */ 


/*
 * Obtain Current Seconds (RTC)
 *   CMOS 00h --- RTC - SECONDS
 *   Desc: BCD 00 - 59, HEX 00 - 3B
 *   Note: Bit 7 is read only
 *   SeeAlso: CMOS 01h, CMOS 02h, CMOS 04h
 *
 *   @return: Current seconds in BCD
 */
static unsigned char cmos_RTC_seconds(void)
{
    outb_p(0x80 | 0x00, CMOS_CTR);
    return inb_p(CMOS_DTA);
}

/*
 * Obtain Alarm Seconds (RTC)
 *   CMOS 001 --- RTC - SECONDS ALARM
 *   Desc: BCD 00 - 59, HEX 00 - 3B
 *   SeeAlso: CMOS 00h, CMOS 03h, CMOS 05h
 *
 *   @return: Alarm seconds in BCD
 */
static unsigned char cmos_Alarm_seconds(void)
{
    outb_p(0x80 | 0x01, CMOS_CTR);
    return inb_p(CMOS_DTA);
}

/*
 * Obtain RTC Minutes
 *   CMOS 02h --- RTC - MINUTES
 *   Desc: BCD 00 - 59, HEX 00 - 3B
 *   SeeAlso: CMOS 00h, CMOS 03h, CMOS 05h
 *
 *   @return: Current minutes in BCD
 */
static unsigned char cmos_RTC_minutes(void)
{
    outb_p(0x80 | 0x02, CMOS_CTR);
    return inb_p(CMOS_DTA);
}

/*
 * Obtain Minute for Alarm
 *   CMOS 03h --- RTC - MINUTE ALARM
 *   Desc: BCD 00 - 59, HEX 00 - 3B
 *   SeeAlso: CMOS 00h, CMOS 02h, CMOS 05h
 *
 *   @return: Minutes for Alarm in BCD
 */
static unsigned char cmos_Alarm_minutes(void)
{
    outb_p(0x80 | 0x03, CMOS_CTR);
    return inb_p(CMOS_DTA);
}

/*
 * Obtain Hour from RTC
 *   CMOS 04h --- RTC - HOURS
 *   Desc: BCD 00 - 23, Hex 00 - 17 if 24 hr mode
 *         BCD 01 - 12, Hex 01 - 0C if 12 hr am
 *         BCD 81 - 92, Hex 81 - 8C if 12 hr pm
 *   SeeAlso: CMOS 00h, CMOS 02h, CMOS 05h
 *
 *   @return: Current hour from RTC in BCD.
 */
static unsigned char cmos_RTC_hour(void)
{
    outb_p(0x80 | 0x04, CMOS_CTR);
    return inb_p(CMOS_DTA);
}

/*
 * Obtain Hour for Alarm
 *   CMOS 05h --- RTC - HOUR ALARM
 *   Desc: same as hours, don't care if C0 - FF
 *   SeeAlso: CMOS 01h, CMOS 03h, CMOS 04h
 *
 *   @return: The hours for Alarm in BCD
 */
static unsigned char cmos_Alarm_hour(void)
{
    outb_p(0x80 | 0x05, CMOS_CTR);
    return inb_p(CMOS_DTA);
}

/*
 * Obtain Week from RTC
 *   CMOS 06h --- RTC - DAY OF WEEK
 *   Desc: 01 - 07 Sunday = 1
 *   SeeAlso: CMOS 07h, CMOS 08h, CMOS 09h
 *
 *   @return: Current Week from RTC
 */
static unsigned char cmos_RTC_week(void)
{
    outb_p(0x80 | 0x06, CMOS_CTR);
    return inb_p(CMOS_DTA);
}

/*
 * Obtain Date of Month from RTC
 *   CMOS 07h --- RTC - DATE of MONTH
 *   Desc: BCD 01 - 31, HEX 01 - 1F
 *   SeeAlso: CMOS 06h, CMOS 08h, CMOS 09h
 *
 *   @return: the data of month in BCD
 */
static unsigned char cmos_RTC_date(void)
{
    outb_p(0x80 | 0x07, CMOS_CTR);
    return inb_p(CMOS_DTA);
}

/*
 * Obtain Month from RTC
 *   CMOS 08h ---- RTC - MONTH
 *   Desc: BCD 01 - 12, HEX 01 - 0C
 *   SeeAlso: CMOS 06h, CMOS 07h, CMOS 09h
 *
 *   @return: The current month from RCT in BCD
 */
static unsigned char cmos_RTC_month(void)
{
    outb_p(0x80 | 0x08, CMOS_CTR);
    return inb_p(CMOS_DTA);
}

/*
 * Obtain Year from RTC
 *   CMOS 09h --- RTC - YEAR
 *   Desc: BCD 00 - 99, HEX 00 - 63
 *   Notes: BCD/HEX selection depends on Bit 2 of register B (0Bh)
 *          12/24 Hr selection depends on Bit 1 of register B (0Bh)
 *          Alarm will trigger when contents of all three Alarm byte 
 *          registers match their companions.
 *   SeeAlso: CMOS 06h, CMOS 07h, CMOS 08h
 */
static unsigned char cmos_RTC_year(void)
{
    outb_p(0x80 | 0x09, CMOS_CTR);
    return inb_p(CMOS_DTA);
}

/*
 * Obtain RTC Status Register 1
 *   CMOS 0Ah --- RTC - STATUS REGISTER A (read/write)
 *   Bitfields for RTC Status register A:
 *     7     = 1 time update cycle in progress, data output undefined
 *           (bit 7 is read only)
 *     6-4   22 stage divider
 *           010 = 32768 Hz time base (default)
 *     3-0   rate selection bits for interrupt
 *           0000 none
 *           0011 122 microseconds (minimum)
 *           1111 500 milliseconds
 *           0110 976.562 milliseconds (default 1024 Hz)
 *  SeeAlso: #C002, #C003, #C004
 */
/* Read Operation */
static unsigned char cmos_RTC_statusA(void)
{
    outb_p(0x80 | 0x0A, CMOS_CTR);
    return inb_p(CMOS_DTA);
}
/* Write Operation */
static void cmos_RTC_statusA_update(unsigned char state)
{
    outb_p(0x80 | 0x0A, CMOS_CTR);
    outb_p(state, CMOS_DTA);
}

/*
 * Obtain RTC Status Register B
 *   Bitfields for RTC Status Register B: 
 *   Bit(s)  Description   (Table C002)
 *     7     Enable cycle update
 *     6     Enable periodic interrupt
 *     5     Enable alarm interrupt
 *     4     Enable update-enable interrupt
 *     3     Enable square wave 
 *     2     Data Mode - 0: BCD, 1: Binary
 *     1     24/12 hour selection - 1 enables 24 hour mode
 *     0     Daylight Saving Enable - 1 enables
 *   SeeAlso: #C001, #C004
 */
/* Read operation for StatusB */
static unsigned char cmos_RTC_statusB(void)
{
    outb_p(0x80 | 0x0B, CMOS_CTR);
    return inb_p(CMOS_DTA);
}
/* Write operation for StatusB */
static void cmos_RTC_statusB_update(unsigned char cmd)
{
    outb_p(0x80 | 0x0B, CMOS_CTR);
    outb_p(cmd, CMOS_DTA);
}

/* 
 * Setup 24/12 hour
 *   @mode: 1 - 24 hours
 *          0 - 12 hours
 */
static void setup_hour_mode(int mode)
{
    unsigned char status;

    status = cmos_RTC_statusB();
    /*
     * Bitfields for RTC Status Register B: 
     *   Bit(s)  Description   (Table C002)
     *     7     Enable cycle update
     *     6     Enable periodic interrupt
     *     5     Enable alarm interrupt
     *     4     Enable update-enable interrupt
     *     3     Enable square wave 
     *     2     Data Mode - 0: BCD, 1: Binary
     *     1     24/12 hour selection - 1 enables 24 hour mode
     *     0     Daylight Saving Enable - 1 enables
     */
    status &= 0xFD;
    status |= (mode & 0x1) << 1;
    cmos_RTC_statusB_update(status);
}

/*
 * Setup Data Format
 *   @mode: 0 -- BCD
 *          1 -- Binary
 */
static void setup_data_format(unsigned char mode)
{
    unsigned char status;

    status = cmos_RTC_statusB();
    /*
     * Bitfields for RTC Status Register B: 
     *   Bit(s)  Description   (Table C002)
     *     7     Enable cycle update
     *     6     Enable periodic interrupt
     *     5     Enable alarm interrupt
     *     4     Enable update-enable interrupt
     *     3     Enable square wave 
     *     2     Data Mode - 0: BCD, 1: Binary
     *     1     24/12 hour selection - 1 enables 24 hour mode
     *     0     Daylight Saving Enable - 1 enables
     */
    status &= 0xFB;
    status |= (mode & 0x1) << 2;
    cmos_RTC_statusB_update(status);
}

/*
 * Obtain RTC Status Register C
 *   CMOS 0Ch --- RTC - STATUS REGISTER C (Read only)
 *   Bitfields for RTC Status Register C:
 *   Bit(s)  Description   (Table C003)
 *     7     Interrupt request flag
 *           =1 when any or all of bits 6-4 are 1 and appropriate 
 *           enables (Register B) are set to 1. Generates IRQ 8 
 *           when triggered.
 *     6     Periodic Interrupt flag
 *     5     Alarm Interrupt flag
 *     4     Update-Ended Interrupt Flag
 *     3-0   unused
 *   SeeAlso: #C001, #C002, #C004 
 */
static unsigned char cmos_RTC_statusC(void)
{
    outb_p(0x80 | 0x0C, CMOS_CTR);
    return inb_p(CMOS_DTA);
}

/*
 * Obtain RTC Status Register D
 *   CMOS 0Dh --- RTC - STATUS REGISTER D (read only)
 *   Bit(s)  Description     (Table C004)
 *     7     Valid RAM - 1 indicates batery power good,
 *                       0 if dead or disconnected.
 *     6-0   unused
 */
static unsigned char cmos_RTC_statusD(void)
{
    outb_p(0x80 | 0x0D, CMOS_CTR);
    return inb_p(CMOS_DTA);
}

/*
 * Obtain power state for CMOS RAM
 * 
 *   @return: 1 indicates battery power good..
 *            0 if dead or disconnected.
 */
static int cmos_ram_power_status(void)
{
    unsigned char status;

    /*
     * CMOS 0Dh --- RTC - STATUS REGISTER D (read only)
     *   Bit(s)  Description     (Table C004)
     *     7     Valid RAM - 1 indicates batery power good,
     *                       0 if dead or disconnected.
     */
    status = cmos_RTC_statusD();
    return (status & 0x80) ? 1 : 0;
}

/*
 * Obtain Machine Diagnostic Status
 *   CMOS 0Eh --- IBM PS/2 - DIAGNOSTIC STATUS BYTE
 *   Bitfields for IBM PS/2 diagnostic status byte:
 *   Bit(s)  Description     (Table C005)
 *     7     indicates clock has last power,
 *           =1 RTC lost power, =0 power stat stable
 *     6     indicates checksum
 *           =1 config record checksum is bad, =0 ok
 *     5     equipment configuration is incorrect (power-on check
 *           requires that atleast one floppy be installed)
 *           =1 invalid config info found, =0 ok
 *     4     error in memory size
 *           =1 memory size doesn't match config info, =0 ok
 *     3     controller or disk driver failed initialization
 *           =1 fixed disk 0 failed initialized, =0 ok
 *     2     time is invalid
 *           =1 time is invalid, =0 ok (POST validity check)
 *     1     installed adaptors do not match configuration
 *     0     time-out while reading adapter ID
 */
/* Read operation */
static unsigned char cmos_diagnostic_status(void)
{
    outb_p(0x80 | 0x0E, CMOS_CTR);
    return inb_p(CMOS_DTA);
}
/* Write operation */
static void cmos_diagnostic_status_update(unsigned char value)
{
    outb_p(0x80 | 0x0E, CMOS_CTR);
    outb_p(value, CMOS_DTA);
}

/*
 * Check RTC lost power
 *
 *   @return: 1 - RTC lost power
 *            0 - RTC state stable
 */
static int cmos_RTC_power_status(void)
{
    unsigned char value;

    value = cmos_diagnostic_status();
    return (value >> 7) ? 1 : 0;
}

/* Common CMOS RAM entry */
void debug_cmos_ram_common(void)
{
    /* Add item */

    printk("Hello World\n");
    /* Ignore warning, default usage for CMOS RAM (RTC) */
    if (1) {
        unsigned char value;

        /* Obtain current seconds from RTC */
        value = cmos_RTC_seconds();
        printk("CMOS RTC Seconds %d\n", BCD_TO_BIN(value));
        /* Obtain seconds for Alarm */
        value = cmos_Alarm_seconds();
        printk("CMOS Alarm Seconds %d\n", BCD_TO_BIN(value));
        /* Obtain current minutes from RTC */
        value = cmos_RTC_minutes();
        printk("CMOS RTC Minutes %d\n", BCD_TO_BIN(value));
        /* Obtain minutes for Alarm */
        value = cmos_Alarm_minutes();
        printk("CMOS Alarm Minutes %d\n", BCD_TO_BIN(value));
        /* Obtain current hour from RTC */
        value = cmos_RTC_hour();
        printk("CMOS RTC hour %d\n", BCD_TO_BIN(value));
        /* Obtain hour for Alarm */
        value = cmos_Alarm_hour();
        printk("CMOS Alarm Hours %d\n", BCD_TO_BIN(value));
        /* Obtain current week from RTC */
        value = cmos_RTC_week();
        printk("CMOS RTC week %d\n", BCD_TO_BIN(value));
        /* Obtain date of Month from RTC */
        value = cmos_RTC_date();
        printk("CMOS RTC date %d\n", BCD_TO_BIN(value));
        /* Obtain month from RTC */
        value = cmos_RTC_month();
        printk("CMOS RTC month %d\n", BCD_TO_BIN(value));
        /* Obtain year from RTC */
        value = cmos_RTC_year();
        printk("CMOS RTC year %d\n", BCD_TO_BIN(value));
        /* Obtain RTC Status Register A */
        value = cmos_RTC_statusA();
        cmos_RTC_statusA_update(value);
        printk("CMOS RTC StatusA %#X\n", value);
        /* Obtain RTC Status Register B */
        value = cmos_RTC_statusB();
        printk("CMOS RTC StatusB %#x\n", value);
        /* Setup 24-hour, Binary format */
        setup_hour_mode(H_12H);
        setup_data_format(F_BINARY);
        value = cmos_RTC_hour();
        printk("CMOS RTC Hour(12h/Binary) %d\n", value);
        /* Obtain RTC Status Register C */
        value = cmos_RTC_statusC();
        printk("CMOS RTC StatusC %#x\n", value);
        /* Obtain RTC Status Register D */
        value = cmos_RTC_statusD();
        printk("CMOS RTC StatusD %#x\n", value);
        /* Obtain CMOS RAM Power Status */
        if (cmos_ram_power_status())
            printk("CMOS RAM Power On.\n");
        else
            printk("CMOS RAM Dead or Disconnect.\n");
        /* Obtain Diagnostic status */
        value = cmos_diagnostic_status();
        printk("CMOS Diagnostic Status %#x\n", value);
        cmos_diagnostic_status_update(value);
        /* Obtain RTC clock power status */
        if (cmos_RTC_power_status())
            printk("CMOS RTC power lose.\n");
        else
            printk("CMOS RTC power stable.\n");
    }
}
