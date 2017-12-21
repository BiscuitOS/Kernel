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
 *   ----- R0F -----------------------------------------------
 *   CMOS 0Fh --- IBM - RESET CODE (IBM PS/2 "Shutdown Status Byte") 
 *   (Table C006)
 *   Values for Rest Code / Shutdown Status Byte:
 *   00h - 03h    Perform power-on reset
 *         01h    Reset after memory size check in real/virtual mode
 *                (or: chip set initialization for real mode reentry)
 *         02h    Reset after succssful memory test in real/virtual mode
 *         03h    Reset after failed memory test in real/virtual mode
 *   04h          INT 19h reboot
 *   05h          flush keyboard (issue EOI) and jump via 40h:0067h
 *   06h          reset (after failed test in virtual mode)
 *                (or: jump via 40h:0067h without EOI)
 *   07h          reset (after failed test in virtual mode)
 *   08h          used by POSI during protected-mode RAM test 
 *                (Return to POST)
 *   09h          used for INT 15/87h (bclock move) support
 *   0Ah          resume execution by jump via 40h:0067h
 *   0Bh          resume execution via IRET via 40h:0067h
 *   0Ch          resume execution via RETF via 40h:0067h
 *   0Dh - FFh    perform power-on reset
 *  
 * The second group of values extends from address 10h to 2Dh. The word at
 * 2Eh - 2Fh is a byte-wise summation of the values in these bytes. Most
 * BIOSes will generate a CMOS Checksum error if this value is invalid 
 * however many programs ignore checksum and report the apparent value.
 * The current version of MSD reports my XT as having 20+ MB of extended
 * memory.
 *
 * Where a definition apperas universal, no identification is made. Where
 * the definition is though to be specific to a manufacturer/mode (AMI,
 * AMSTRAD, IBM AT, IBM PS/2) the identification is enclosed in parens.
 * The AMSTAD definitions appear to relate to 8088/8086 (PC and PC/XT
 * class) machines only. AT class machines appear to adhere to IBM PC/AT
 * fornat. 
 *
 *   ----- R10 -----------------------------------------------
 *   CMOS 10h --- IBM - FLOPPY DRIVE TYPE
 *   Note:
 *     a PC having a 5 1/4 1.2 Mb A.
 *     a 1.44 Mb B.
 *     drive will have a value of 24h in byte 10h. 
 *     With a single 1.44 drive: 40h.
 *   Bitfields for floppy drivers A/B types:
 *   Bit(s)  Description    (Table C007)
 *    7 - 4  first floppy disk drive type (see #C008)
 *    3 - 0  second floppy disk drive type (see #C008)
 *   
 *   (Table C008)
 *   Values for floppy drive type:
 *    00h    no drive
 *    01h    360KB   5.25 Drive
 *    02h    1.2MB   5.25 Drive - note: not listed in PS/2 technical manual
 *    03h    720KB   3.5  Drive
 *    04h    1.44MB  3.5  Drive
 *    05h    2.88MB  3.5  Drive
 *   06h-0fh unused
 *   SeeAlso: #C007
 *
 *   ----- R11 -----------------------------------------------
 *   Reserved / AMI Extended CMOS setup (AMI Hi-Flex BIOS)
 *
 *   ----- R12 -----------------------------------------------
 *   CMOS 12h --- IBM - HARD DISK DATA
 *   Notes:
 *     A PC with a single type 2 (20Mb ST-225) hard disk will have 20h in
 *     byte 12h. some PCs utilizing external disk controller ROMs will 
 *     use type 0 to disable ROM BIOS (e.g. Zenith 248 with Plus HardCard).
 *
 *   Bitfields for IBM hard disk data:
 *   Bit(s)   Description      (Table C014)
 *    7-4     First Hard Disk Drive
 *            00          No Drive
 *            01 - 0EH    Hard Drive Type 1-14
 *            0Fh         Hard Disk Type  16-255
 *                        (actual Hard Drive Type is in CMOS RAM 19H)
 *    3-0     Second Hard Disk Drive Type
 *            (same as first except extrnede type will be found in 1Ah)
 *
 *   ----- R12 -----------------------------------------------
 *   CMOS 12h --- IBM PS/2 - SECOND FIXED DISK DRIVE TYPE (00-FFh)
 *
 *   ----- R14 -----------------------------------------------
 *   CMOS 14h --- IBM - EQUIPMENT BYTE
 *   Bitfields for IBM equipment byte:
 *   Bit(s)   Description    (Table C019)
 *    7-6     number of floppy drivers (system must have at least one)
 *            00b    1 Drive
 *            01b    2 Drive
 *            10b    3 Drive
 *            11b    4 Drive
 *    5-4     monitor type
 *            00b    Not CGA or MDA (observed for EGA & VGA)
 *            01b    40x25 CGA
 *            10b    80x25 CGA
 *            11b    MDA (Monochrome)
 *    3       display enabled (turned off to enable boot of rackmount)
 *    2       keyboard enabled (turn off to enable boot of rackmount)
 *    1       mach coprocessor installed
 *    0       floppy drive installed (turned off for rackmount boot) 
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
 * Diagnose RTC whether lost power
 *
 *   @return: 1 - RTC lost power
 *            0 - RTC state stable
 */
static int diagnose_RTC_power_status(void)
{
    unsigned char value;

    value = cmos_diagnostic_status();
    return (value >> 7) ? 1 : 0;
}

/*
 * Diagnose CMOS RAM checksum
 * 
 *   @return: 1 - config record checksum is bad.
 *            0 - config record checksum is ok
 */
static int diagnose_cmos_ram_checksum(void)
{
    unsigned char status;

    status = cmos_diagnostic_status();
    return (status >> 6) & 0x1 ? 1 : 0;
}

/*
 * Diagnose whether configuration information at POST is invalid.
 *
 *  @return: 1 equirment configure is incorrect
 *           0 equirment configure is correct
 */
static int diagnose_configure_info(vvoid)
{
    unsigned char value;

    value = cmos_diagnostic_status();
    return (value >> 5) & 0x1 ? 1 : 0;
}

/*
 * Diagnose Whether CMOS RAM memory size matches configure
 *
 *   @return: 1 - memory size doesn't match configure
 *            0 - memory size match configure
 */
static int diagnose_cmos_memory_size(void)
{
    unsigned char value;

    value = cmos_diagnostic_status();
    return (value >> 4) & 0x01 ? 1 : 0;
}

/*
 * Diagnose whether Disk0 has initialized
 *
 *  @return: 1 - Disk 0 doesn't initialize.
 *           0 - Disk 0 has initialized.
 */
static int diagnose_disk0_init_statue(void)
{
    unsigned char value;

    value = cmos_diagnostic_status();
    return (value >> 3) & 0x1 ? 1 : 0;
}

/*
 * Diagnose whether CMOS RAM time is valid?
 *
 *   @return: 1 - CMOS RAM time invalid
 *            0 - CMOS RAM time valid
 */
static int diagnose_cmos_time_status(void)
{
    unsigned char value;

    value = cmos_diagnostic_status();
    return (value >> 2) & 0x1 ? 1 : 0;
}

/*
 * Diagnose whether EISA match configure
 *
 *   @return: 1 - EISA doesn't match configure
 *            0 - EISA match configure 
 */
static int diagnose_EISA_status(void)
{
    unsigned char value;

    value = cmos_diagnostic_status();
    return (value >> 1) & 0x1 ? 1 : 0;
}

/*
 * Diagnose whether timeout when reading EISA ID
 *
 *   @return: 1 - Reading EISA ID timeout
 *            0 - Reading EISA ID doesn't timeout
 */
static int diagnose_obtain_EISA_timeout(void)
{
    unsigned char value;

    value = cmos_diagnostic_status();
    return value & 0x1 ? 1 : 0;
}

/*
 * Obtain Shutdown Status
 *   CMOS 0Fh --- IBM - RESET CODE (IBM PS/2 "Shutdown Status Byte") 
 *   (Table C006)
 *   Values for Rest Code / Shutdown Status Byte:
 *   00h - 03h    Perform power-on reset
 *         01h    Reset after memory size check in real/virtual mode
 *                (or: chip set initialization for real mode reentry)
 *         02h    Reset after succssful memory test in real/virtual mode
 *         03h    Reset after failed memory test in real/virtual mode
 *   04h          INT 19h reboot
 *   05h          flush keyboard (issue EOI) and jump via 40h:0067h
 *   06h          reset (after failed test in virtual mode)
 *                (or: jump via 40h:0067h without EOI)
 *   07h          reset (after failed test in virtual mode)
 *   08h          used by POSI during protected-mode RAM test 
 *                (Return to POST)
 *   09h          used for INT 15/87h (bclock move) support
 *   0Ah          resume execution by jump via 40h:0067h
 *   0Bh          resume execution via IRET via 40h:0067h
 *   0Ch          resume execution via RETF via 40h:0067h
 *   0Dh - FFh    perform power-on reset
 */
static int obtain_shutdown_status(void)
{
    unsigned char status;

    outb_p(0x80 | 0xF, CMOS_CTR);
    status = inb_p(CMOS_DTA);

    printk("Shutdown CODE[%#x]\n", status);
    switch (status) {
    case 0x1:
        printk("Reset after memory size check in real/virtual mode\n");
        break;
    case 0x2:
        printk("Reset after succssful memory test in real/virtual mode\n");
        break;
    case 0x3:
        printk("Reset after failed memory test in real/virtual mode\n");
        break;
    case 0x4:
        printk("INT 19h reboot\n");
        break;
    case 0x5:
        printk("flush keyboard (issue EOI) and jump via 40h:0067h\n");
        break;
    case 0x6:
        printk("reset (after failed test in virtual mode)\n");
        printk("(or: jump via 40h:0067h without EOI)\n");
        break;
    case 0x7:
        printk("reset (after failed test in virtual mode)\n");
        break;
    case 0x8:
        printk("used by POSI during protected-mode RAM test\n");
        break;
    case 0x9:
        printk("used for INT 15/87h (bclock move) support\n");
        break;
    case 0xA:
        printk("resume execution by jump via 40h:0067h\n");
        break;
    case 0xB:
        printk("resume execution via IRET via 40h:0067h\n");
        break;
    case 0xC:
        printk("resume execution via RETF via 40h:0067h\n");
        break;
    default:
        printk("perform power-on reset\n");
        break;
    }
    return status;
}

/*
 * Obtain floppy information
 *   CMOS 10h --- IBM - FLOPPY DRIVE TYPE
 *   Note:
 *     a PC having a 5 1/4 1.2 Mb A.
 *     a 1.44 Mb B.
 *     drive will have a value of 24h in byte 10h. 
 *     With a single 1.44 drive: 40h.
 *   Bitfields for floppy drivers A/B types:
 *   Bit(s)  Description    (Table C007)
 *    7 - 4  first floppy disk drive type (see #C008)
 *    3 - 0  second floppy disk drive type (see #C008)
 *   
 *   (Table C008)
 *   Values for floppy drive type:
 *    00h    no drive
 *    01h    360KB   5.25 Drive
 *    02h    1.2MB   5.25 Drive - note: not listed in PS/2 technical manual
 *    03h    720KB   3.5  Drive
 *    04h    1.44MB  3.5  Drive
 *    05h    2.88MB  3.5  Drive
 *   06h-0fh unused
 *   SeeAlso: #C007
 */
static unsigned char obtain_floppy_info(void)
{
    unsigned char status;
    int i;

    outb_p(0x80 | 0x10, CMOS_CTR);
    status = inb_p(CMOS_DTA);

    for (i = 0; i < 2; i++) {
        /* Parse floppy information */
        switch ((status >> (i * 4)) & 0xF) {
        case 0x00:
            printk("Floppy %d doesn't exist.\n", i);
            break;
        case 0x01:
            printk("Floppy %d 5.25 - 360KB\n", i);
            break;
        case 0x02:
            printk("Floppy %d 5.25 - 1.2MB\n", i);
            break;
        case 0x03:
            printk("Floppy %d 3.5 - 720KB\n", i);
            break;
        case 0x04:
            printk("Floppy %d 3.5 - 1.44KB\n", i);
            break;
        case 0x05:
            printk("Floppy %d 3.5 - 2.88KB\n", i);
            break;
        default:
            printk("Undefind for floppy %d\n", i);
            break;
        }
    }
    return status;
}

/*
 * Obtain Hard-Disk Type
 *   CMOS 12h --- IBM - HARD DISK DATA
 *   Notes:
 *     A PC with a single type 2 (20Mb ST-225) hard disk will have 20h in
 *     byte 12h. some PCs utilizing external disk controller ROMs will 
 *     use type 0 to disable ROM BIOS (e.g. Zenith 248 with Plus HardCard).
 *
 *   Bitfields for IBM hard disk data:
 *   Bit(s)   Description      (Table C014)
 *    7-4     First Hard Disk Drive
 *            00          No Drive
 *            01 - 0EH    Hard Drive Type 1-14
 *            0Fh         Hard Disk Type  16-255
 *                        (actual Hard Drive Type is in CMOS RAM 19H)
 *    3-0     Second Hard Disk Drive Type
 *            (same as first except extrnede type will be found in 1Ah)
 */
static void obtain_HardDisk_type(void)
{
    unsigned char value;

    outb_p(0x80 | 0x12, CMOS_CTR);
    value = inb_p(CMOS_DTA);
    if (value & 0xF)
        printk("Hard Disk 1 Type %d\n", value & 0xF);
    else
        printk("Hard Disk 1 doesn't exist\n");
    if ((value >> 4) & 0xF)
        printk("Hard Disk 0 Type %d\n", (value >> 4) & 0xf);
    else
        printk("Hard Disk 0 doesn't exist.\n");
}

/*
 * Obtain equirment type
 *   CMOS 14h --- IBM - EQUIPMENT BYTE
 *   Bitfields for IBM equipment byte:
 *   Bit(s)   Description    (Table C019)
 *    7-6     number of floppy drivers (system must have at least one)
 *            00b    1 Drive
 *            01b    2 Drive
 *            10b    3 Drive
 *            11b    4 Drive
 *    5-4     monitor type
 *            00b    Not CGA or MDA (observed for EGA & VGA)
 *            01b    40x25 CGA
 *            10b    80x25 CGA
 *            11b    MDA (Monochrome)
 *    3       display enabled (turned off to enable boot of rackmount)
 *    2       keyboard enabled (turn off to enable boot of rackmount)
 *    1       mach coprocessor installed
 *    0       floppy drive installed (turned off for rackmount boot)
 */
static unsigned char obtain_equirment_byte(void)
{
    outb_p(0x80 | 0x14, CMOS_CTR);
    return inb_p(CMOS_DTA);
}

/*
 * Obtain floppy number
 *
 *   @return: The number of floppy on system.
 */
static int obtain_floppy_number(void)
{
    return ((obtain_equirment_byte() >> 6) & 0x3) + 1;
}

/*
 * Detect Display power status
 *
 *  @return: 1 is enable
 *           0 is disable
 */
static int detect_display_enable(void)
{
    return ((obtain_equirment_byte() >> 3) & 0x1);
}

/*
 * Detect Keyboard enable
 *
 *   @return: 0 enable
 *            1 disable
 */
static int detect_keyboard_enable(void)
{
    return ((obtain_equirment_byte() >> 2) & 0x1);
}

/*
 * Detect Math corprocessor
 *
 *   @return: 1 -- installed
 *            0 -- not installed
 */
static int detect_math_corprocessor(void)
{
    return ((obtain_equirment_byte() >> 1) & 0x1);
}

/*
 * Detect floppy driver
 *
 *   @return: 1 -- installed
 *            0 -- not installed
 */
static int detect_floppy_driver(void)
{
    return (obtain_equirment_byte() & 0x1);
}

/*
 * Detect monitor type
 *
 *   @return: 00b    Not CGA or MDA (observed for EGA & VGA)
 *            01b    40x25 CGA
 *            10b    80x25 CGA
 *            11b    MDA (Monochrome)
 */
static int detect_monitor_type(void)
{
    switch ((obtain_equirment_byte() >> 0x4) & 0x3) {
    case 0x00:
        printk("Not CGA or MDA (observed for EGA & VGA)\n");
        break;
    case 0x01:
        printk("40x25 CGA\n");
        break;
    case 0x02:
        printk("80x25 CGA\n");
        break;
    case 0x03:
        printk("MDA (Monochrome)\n");
        break;
    }
    return (obtain_equirment_byte() >> 0x4) & 0x3;
}

/* Common CMOS RAM entry */
void debug_cmos_ram_common(void)
{
    /* Add item */

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
        if (diagnose_RTC_power_status())
            printk("CMOS RTC power lose.\n");
        else
            printk("CMOS RTC power stable.\n");
        /* Check CMOS RAM configure */
        if (diagnose_cmos_ram_checksum())
            printk("CMOS RAM config record is bad.\n");
        else
            printk("CMOS RAM config record is ok.\n");
        /* Check equirment configure info incorrect? */
        if (diagnose_configure_info())
            printk("CMOS configure info incorrect!\n");
        else
            printk("CMOS configure info correct!\n");
        /* Check whether CMOS RAM size matchs configure */
        if (diagnose_cmos_memory_size())
            printk("CMOS RAM memory size doesn't match configure.\n");
        else
            printk("CMOS RAM memory size match configure.\n");
        /* Check whether Disk0 has initialzed? */
        if (diagnose_disk0_init_statue())
            printk("Disk0 doesn't initialize.\n");
        else
            printk("Disk0 has initialized.\n");
        /* Check whether CMOS RAM time is valid */
        if (diagnose_cmos_time_status())
            printk("CMOS RAM time invalid.\n");
        else
            printk("CMOS RAM time valid.\n");
        /* Check whether EISA match configure */
        if (diagnose_EISA_status())
            printk("EISA doesn't match configure.\n");
        else
            printk("EISA match configure.\n");
        /* Diagnose whether timeout when reading EISA ID */
        if (diagnose_obtain_EISA_timeout())
            printk("Timeout when reading EISA ID\n");
        else
            printk("Correctly obtaining EISA ID\n");
        /* Obtain Shutdown status */
        obtain_shutdown_status();
        /* Obtain Floppy information */
        obtain_floppy_info();
        /* Obtain Hard-Disk type */
        obtain_HardDisk_type();
        /* obtain the number of floppy */
        value = obtain_floppy_number();
        printk("The system has %d floppy(s)\n", value);
        /* Detect Monitor Type */
        detect_monitor_type();
        /* Detect Display power status */
        if (detect_display_enable())
            printk("Display Enable\n");
        else
            printk("Display Disable\n");
        /* Detect Keyboard */
        if (detect_keyboard_enable())
            printk("Keyboard Disable\n");
        else
            printk("Keyboard enable\n");
        /* Detect Math Corprocessor */
        if (detect_math_corprocessor())
            printk("Math corprocessor has installed\n");
        else
            printk("Math corprocessor doesn't install\n");
        /* Detect floppy driver install? */
        if (detect_floppy_driver()) 
            printk("Floppy driver has installed\n");
        else
            printk("Floppy driver doesn't install\n");
    }
}
