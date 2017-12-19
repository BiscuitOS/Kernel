/*
 * Debug CMOS clock
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

void debug_cmos_clk_common(void)
{
    /* Add item */

    /* Ignore warning */
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
    }
}
