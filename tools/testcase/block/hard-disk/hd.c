/*
 * Hard disk device driver (AT, PS/2)
 *
 * (C) 2018.02 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/hdreg.h>
#include <asm/io.h>
#include <asm/system.h>

#include <test/debug.h>

/*
 * 1st Fixed Disk Controller (ISA, EISA)
 *   Range 0x1F0 -- 0x1F7 HDC 1 same as 0x17x (ISA, EISA)
 *   Reigster Map:
 *
 *   1F0   r/w   Data register
 *
 *   1F1   r     error register
 *               diagnostic mode errors:
 *                bit 7-3        reserved
 *                bit 2-1 = 001  no error detected
 *                        = 010  formatter device error
 *                        = 011  sector buffer error
 *                        = 100  ECC circuitry error
 *                        = 101  controlling microprocessor error
 *               operation mode:
 *                bit 7   = 1    bad block detected
 *                        = 0    block OK
 *                bit 6   = 1    uncorrectable ECC error
 *                        = 0    no error
 *                bit 5          reserved
 *                bit 4   = 1    ID found
 *                        = 0    ID not found
 *                bit 3          reserved
 *                bit 2   = 1    command completed
 *                        = 0    aborted
 *                bit 1   = 1    track 000 ont found
 *                        = 0    track 000 found
 *                bit 0   = 1    DAM not found
 *                        = 0    DAM found (CP-3022 always 0)
 *
 *   1F1   w     WPC/4  (Write Precompensation Cylinder divided by 4)
 *
 *   1F2   r/w   sector count
 *   1F3   r/w   sector number
 *   1F4   r/w   cylinder low
 *   1F5   r/w   cylinder high
 *
 *   1F6   r/w   drive/head
 *                bit 7   = 1
 *                bit 6   = 0
 *                bit 5   = 1
 *                bit 4   = 0  drive 0 select
 *                        = 1  drive 1 select
 *                bit 3-0      head select bits
 *
 *   1F7   w     command register
 *                98 E5  check power mode       (IDE)
 *                90     execute drive diagnostics
 *                50     format track
 *                EC     identify drive         (IDE)
 *                97 E3  idle                   (IDE)
 *                95 E1  idle immediate         (IDE)
 *                91     initialize drive parameters
 *                1x     recalibrate
 *                E4     read buffer            (IDE)
 *                C8     read DMA with retry    (IDE)
 *                C9     read DMA without retry (IDE)
 *                C4     read multiplec         (IDE)
 *                20     read sectors with retry
 *                21     read sectors without retry
 *                22     read long with retry
 *                23     read long without retry
 *                40     read verify sectors with retry
 *                41     read verify sectors without retry
 *                7x     seek
 *                EF     set features           (IDE)
 *                C6     set multiple mode      (IDE)
 *                99 E6  set sleep mode         (IDE)
 *                96 E2  standby                (IDE)
 *                94 E0  standby immediate      (IDE)
 *                E8     write buffer           (IDE)
 *                CA     write DMA with retry   (IDE)
 *                CB     write DMA with retry   (IDE)
 *                C5     write multiple         (IDE)
 *                E9     write same             (IDE)
 *                30     write sectors with retry
 *                31     write sectors without retry
 *                32     write long with retry
 *                33     write long without retry
 *                3C     write verify           (IDE)
 *                9A     vendor unique          (IDE)
 *                C0-C3  vendor unique          (IDE)
 *                8x     vendor unique          (IDE)
 *                F0-F4  EATA standard          (IDE)
 *                F5-FF  vendor unique          (IDE)
 *   1F7   r     status register
 *                bit 0  ERR_STAT      
 *                       = 1    execute error
 *                       = 0    execute pass
 *                bit 1  INDEX_STAT    
 *                       = 1    receive index flag
 *                bit 2  ECC_STAT
 *                       = 1    ECC error
 *                bit 3  DRQ_STAT
 *                       = 1    Data request ready
 *                bit 4  SEEK_STAT
 *                       = 1    seek track finish
 *                bit 5  WRERR_STAT
 *                       = 1    write fault
 *                bit 6  READY_STAT
 *                       = 1    controller ready
 *                bit 7  BUSY_STAT
 *                       = 1    device busy
 */
#define CMOS_READ(addr) ({\
    outb_p(0x80 | addr, 0x70);  \
    inb_p(0x71); \
})

struct hd_struct hd2[5 * MAX_HD] = {
    {0, 0},
};

struct hd_i_struct hd2_info[] = { 
    {0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0}
};

int NR_HD = 0;

extern void (*do_hd)(void);

/* 
 * Obtian Hard disk information from BIOS 
 *   If we don't specific HD type, we will get Hard-Disk information from
 *   BIOS. The linux 0.11 define memory region [0x90080 - 0x0x900a0] to 
 *   store 1st and 2nd hard disk information. Each one hold 32 bytes region.
 *   More information see arch/x86/boot/setup.s
 */
static void obtain_hard_disk_info_BIOS(void)
{
    /* Define in arch/x86/boot/setup.s */
    unsigned char *BIOS = (unsigned char *)0x90080;
    int driver;
    unsigned char cmos_disks;

    /* Hard-Disk Region Map
     *  The information from BIOS F000h:E401h on arch/x86/boot/setup.s 
     *
     * .-----------------------------------------------------------.
     * | 1st hard-disk information (start: 0x90080)                |
     * -------------------------------------------------------------
     * | 0x90080: Cylinder number                                  |
     * | 0x90081:                                                  |
     * -------------------------------------------------------------
     * | 0x90082: Head number                                      |
     * -------------------------------------------------------------
     * | 0x90083: Cylinder number for begining reduce write        |
     * | 0x90084: current (Only PC XT used, other reserved 0)      |
     * -------------------------------------------------------------
     * | 0x90085: Cylinder number for Write Precompensation        |
     * | 0x90086:                                                  |
     * -------------------------------------------------------------
     * | 0x90087: Max trigger length for ECC (Only XT used)        |
     * -------------------------------------------------------------
     * | 0x90088: Control byte                                     |
     * |          bit 0    Unused                                  |
     * |          bit 1    Reserved 0 (close IRQ)                  |
     * |          bit 2    Permit Reset                            |
     * |          bit 3    Set 1(head > 8)                         |
     * |          bit 4    Unused                                  |
     * |          bit 5    If bad block map of vendor exists       |
     * |                   on Cylinder+1, and set this bit.        |
     * |          bit 6    Forbid ECC retry                        |
     * |          bit 7    Forbid Access retry                     |
     * -------------------------------------------------------------
     * | 0x90089: Standard timeout value (Only XT used)            |
     * -------------------------------------------------------------
     * | 0x9008a: Format timeout value (Only XT used)              |
     * -------------------------------------------------------------
     * | 0x9008b: Check driver timeout value (Only XT used)        |
     * -------------------------------------------------------------
     * | 0x9008c: Cylinder number that head location on            |
     * | 0x9008d:                                                  |
     * -------------------------------------------------------------
     * | 0x9008e: Sector number for track                          |
     * -------------------------------------------------------------
     * | 0x9008f: Reserved                                         |
     * .-----------------------------------------------------------.
     */
    /* Obtain hard-disk information */
    for (driver = 0; driver < 2; driver++) {
        hd2_info[driver].cyl    = *(unsigned short *)BIOS;
        hd2_info[driver].head   = *(unsigned char *)(2 + BIOS);
        hd2_info[driver].wpcom  = *(unsigned short *)(5 + BIOS);
        hd2_info[driver].ctl    = *(unsigned char *)(8 + BIOS);
        hd2_info[driver].lzone  = *(unsigned short *)(12 + BIOS);
        hd2_info[driver].sect   = *(unsigned char *)(14 + BIOS);
        BIOS += 16;
    }
    /* Compute HD number */
    if (hd2_info[1].cyl)
        NR_HD = 2;
    else
        NR_HD = 1;
    /* Establish Hard-Disk partition table */
    for (driver = 0; driver < NR_HD; driver++) {
        hd2[driver * 5].start_sect = 0;
        hd2[driver * 5].nr_sects   = hd2_info[driver].sect * 
                                     hd2_info[driver].cyl;
    }
    /*
     * We querry CMOS about hard disks: it could be that we have
     * a SCSI/ESDI/etc controller that is BIOS compatable with 
     * ST-506, and thus showing up in our BIOS table, but not register
     * compatable, and therefore not present in CMOS.
     *
     * The first drive is stored in the high nibble of CMOS byte
     * 0x12, the second in the low nibble. This will be either a 
     * 4 bit drive type or 0xf indicating use byte 0x19 for an 8
     * byte, drive 1, 0x1a drive 2 in CMOS.
     *
     * Needless to say, a non-zero value means we have an AT controller
     * hard disk for that drive.
     */
    if ((cmos_disks = CMOS_READ(0x12)) & 0xf0) {
        if (cmos_disks & 0x0f)
            NR_HD = 2;
        else
            NR_HD = 1;
    } else
        NR_HD = 0;

    /* Clear un-used partition table */
    for (driver = NR_HD; driver < 2; driver++) {
        hd2[driver * 5].start_sect = 0;
        hd2[driver * 5].nr_sects = 0;
    }
}

/*
 * Determin hard-disk controller is ready.
 *
 *   HD will be busy on different situation and BUSY_STAT(bit 7) bit of
 *   status register (0x1F7) will be set. Details as follow:
 *
 *    1) HD controller receives 'RESET' signal within 400ns.
 *    2) 'SRST' bit on control register is set within 400ns.
 *    3) `BUSY_STAT` bit must be clear within 30s when machine reset.
 *    4) Host write 'Re-correction' to command register(1F7) within 400ns.
 *    5) Host read buffer within 400ns.
 *    6) Host write buffer within 400ns.
 *    7) Host initialize controller within 400ns.
 *    8) Host send diagnoistic command within 400ns. 
 *    9) Transfer 512 Bytes on Read/Write buffer within 5us.
 *    a) Format track within 5us.
 *
 *    @return: 0  hard-disk is busy.
 *             >0 hard-disk is ready.
 */
static int HD_controller_ready(void)
{
    int retries = 100000;

    while (--retries && (inb_p(HD_STATUS) & 0x80));
    return retries;
}

/*
 * Determine hard-disk driver is ready
 *
 *   If driver is ready to transfer data with CPU, The READY_STAT will be
 *   set that determine Hard-Disk driver is ready.
 *
 *   @return: 0 driver isn't busy.
 *            1 driver is busy.
 */
static int HD_drive_busy(void)
{
    unsigned int i;

    for (i = 0; i < 10000; i++)
        if (READY_STAT == (inb_p(HD_STATUS) & (BUSY_STAT | READY_STAT)))
            break;

    i = inb(HD_STATUS);
    i &= BUSY_STAT | READY_STAT | SEEK_STAT;
    if (i == (READY_STAT | SEEK_STAT))
        return 0;
    printk("HD driver timeout\n\r");
    return 1;
}

/*
 * Diagnostic/Error status Register HD_ERROR (0x1f1)
 *   This register hold 8-bit error status. It's valid only HD_STATUS is set.
 *   Each bit has diffence means between Diagnostic command with other 
 *   comand. See Table:
 *
 *   .------------------------------------------------------------------.
 *   | Value | WIN_DIAGNOSE             | Other WIN_*                   |
 *   --------------------------------------------------------------------
 *   | 0x01  | No Error                 | Data ID Lose                  |
 *   --------------------------------------------------------------------
 *   | 0x02  | Controller Error         | Track 0 Error                 |
 *   --------------------------------------------------------------------
 *   | 0x03  | Sector buffer Error      |                               |
 *   --------------------------------------------------------------------
 *   | 0x04  | ECC Error                | Lose command                  |
 *   --------------------------------------------------------------------
 *   | 0x05  | Controller process Error |                               |
 *   --------------------------------------------------------------------
 *   | 0x10  |                          | Un-find ID                    |
 *   --------------------------------------------------------------------
 *   | 0x40  |                          | ECC Error                     |
 *   --------------------------------------------------------------------
 *   | 0x80  |                          | Bad sector                    |
 *   .------------------------------------------------------------------.
 */
static void HD_obtain_diagnose_status(void)
{
    inb_p(HD_ERROR);
}

static void HD_obtain_error_status(void)
{
    inb_p(HD_ERROR);
}

/*
 * Write Precompensation Register
 *
 *   HD_ERROR treat as Write Precompensation Register when write data into
 *   this register. This register hold Write Precompenstation Cylinder.
 *   It corresponding to where offset 0x50 on hard-disk argument table,
 *   and it must divide 4 before output. Now, a lot of hard-disk ignore
 *   this command.
 */
static void HD_Write_Precompensation(short count)
{
    outb_p(count >> 2, HD_ERROR);
}

/*
 * Setup Read/Write/Verify/Initialize sector number.
 *   Before Read/Write/Verify/Initialize operation, system need setup
 *   sector number on HD_NSECTOR (0x1f2) register. On multip sectors,
 *   The Sector Number will be decreased 1 when finished a sector request,
 *   untill it is 0. If initialization value is 0, the system refers
 *   maximue sector number 256.
 *
 *   @sect: the sector number for transferring.
 */
static void HD_setup_nsector(int sect)
{
    outb_p(sect, HD_NSECTOR);
}

/*
 * Setup Read/Write/Verify original sector number.
 *   Before Read/Write/Verify sector, system need setup original sector
 *   number on HD_SECTOR (0x1f3) register. On multip sectors, The original
 *   number will be increased 1 when finished a sector request.
 *
 *   @sect: The original sector number.
 */
static void HD_setup_original_sector(int sect)
{
    outb_p(sect, HD_SECTOR);
}

/*
 * Setup Cylinder.
 *   HD used 10 bits to store Cylinder.The HD_LCYL (0x1f4) register holds 
 *   LSB of Cylinder[7:0], and HD_HCYL (0x1f5) register holds MSB of
 *   Cylinder[9:8].
 *
 *   @cyl: The Cylinder number.
 */
static void HD_setup_cylinder(unsigned short cyl)
{
    /* Setup LSB of Cylinder */
    outb_p(cyl & 0xFF, HD_LCYL);
    /* Setup MSB of Cylinder */
    outb_p(cyl >> 8, HD_HCYL);
}

/*
 * Setup Read/Write/Verify/Seek/Format driver and head.
 *   Before Read/Write/Verify/Seek/Format, system need setup driver and
 *   head on HD_CURRENT (0x1f6) register. The Bitmap of HD_CURRENT is 
 *   101dxxxx. '101' determines that uses ECC verify and each sector comprises
 *   512 Bytes. 'd' determines driver number (0 or 1). 'xxxx' determins 
 *   specifical head. HD_CURRENT Bitmap see table:
 *
 *   .--------------------------------------------------------------------.
 *   | Bit | Item                  | Describe                             |
 *   ----------------------------------------------------------------------
 *   |  0  | HS0       | Head 0    | The lowest bit of head               |
 *   ----------------------------------------------------------------------
 *   |  1  | HS1       | Head 1    |                                      |
 *   ----------------------------------------------------------------------
 *   |  2  | HS2       | Head 2    |                                      |
 *   ----------------------------------------------------------------------
 *   |  3  | HS3       | Head 3    | The highest bit of head              |
 *   ----------------------------------------------------------------------
 *   |  4  | DRV       | Driver    | 0 - Driver0;  1 - Drvier1            |
 *   ----------------------------------------------------------------------
 *   |  5  | Reserved  | Reserved  | Alway 1                              |
 *   ----------------------------------------------------------------------
 *   |  6  | Reserved  | Reserved  | Alway 0                              |
 *   ----------------------------------------------------------------------
 *   |  7  | Reserved  | Reserved  | Alway 1                              |
 *   .--------------------------------------------------------------------.
 */
static void HD_setup_driver_head(int drive, int head)
{
    /* 0xA0 hold 0b101xxxxx */
    outb_p(0xA0 | (drive & 0x1) << 4 | (head & 0xF), HD_CURRENT);
}

/*
 * Send Command to HD controller
 *   HD_STATUS (0x1f7) receives command from CPU, it can prase 8 commands.
 *   When controll receives and executes command, a HD interrupt will be
 *   trigger. AT HD controller command list:
 *
 *   .-------------------------------------------------------------------.
 *   |    Command     |          Command byte        | Return    | Value |
 *   |                | MSB [7:4]    | D3 D2 D1 D0   |           |       |
 *   ---------------------------------------------------------------------
 *   | WIN_RESTORE    | 0x1          |  R  R  R  R   | interrupt | 0x10  |
 *   ---------------------------------------------------------------------
 *   | WIN_READ       | 0x2          |  0  0  L  T   | interrupt | 0x20  |
 *   ---------------------------------------------------------------------
 *   | WIN_WRITE      | 0x3          |  0  0  L  T   | interrupt | 0x30  |
 *   ---------------------------------------------------------------------
 *   | WIN_VERIFY     | 0x4          |  0  0  0  T   | interrupt | 0x40  |
 *   ---------------------------------------------------------------------
 *   | WIN_FORMAT     | 0x5          |  0  0  0  0   | interrupt | 0x50  |
 *   ---------------------------------------------------------------------
 *   | WIN_INIT       | 0x6          |  0  0  0  0   | interrupt | 0x60  |
 *   ---------------------------------------------------------------------
 *   | WIN_SEEK       | 0x7          |  R  R  R  R   | interrupt | 0x70  |
 *   ---------------------------------------------------------------------
 *   | WIN_DIAGNOSE   | 0x9          |  0  0  0  0   | interrupt | 0x90  |
 *   ---------------------------------------------------------------------
 *   | WIN_SPECIFY    | 0x9          |  0  0  0  0   | interrupt | 0x91  |
 *   .-------------------------------------------------------------------.
 *
 *   Tips:
 *    1) R: Stepper speed
 *       R = 0, stepper speed is 35us.
 *       R = 1, stepper speed is 0.5us.
 *       Default R = 0.
 *    2) L: Date mode
 *       L = 0, A Sector only contains 512 Bytes
 *       L = 1, A Sector contains 512 Bytes and 4 Bytes ECC code.
 *       Default L = 0.
 *    3) T: Retry mode
 *       T = 0, permit retry.
 *       T = 1, forbid retry.
 */
static void HD_write_command(int cmd)
{
    switch (cmd & 0xF0) {
    case WIN_RESTORE:
        /* WIN_RESTORE: Driver recalibrate
         *  
         *  This command will move Read/Write head to Cylinder 0. Driver will
         *  set BUSY_STAT flag and send a command that seek 0 Cylinder when
         *  controller received this 'WIN_RESTORE' command. Then reset
         *  BUSY_STAT flag and trigger a HD interrupt when SEEK finished.
         */
        outb_p(WIN_RESTORE, HD_COMMAND);
        break;
    case WIN_READ:
        /* WIN_READ: 0x20 Retry read buffer - 0x21 non-Retry read buffer
         *  
         *  WIN_READ command can read 1 to 256 serctors, if HD_NSECTOR 
         *  register is 0, the sectors number is 256. The controller will
         *  set 'BUSY_STAT' flag and immediately read sector when recevice
         *  'WIN_READ' from CPU. On single sector reading, if head doesn't
         *  locate in incorrect track, and controller will execute SEEK 
         *  implicitly. If head locates on correct track, head will be 
         *  locate in ID field for track.
         *
         *  For Non-Retry mode, the drvier will write track ID into Diagnostic
         *  register when head doesn't locat correct track ID.
         *
         *  For Retry-mode, controller will retry obtain correct track ID,
         *  As to retry times that according to Vendor.
         *
         *  If driver obtain correct ID field, and must determine Data address
         *  mark on specifical byte filed. If not, driver will report loss
         *  Data address mark. Once obtain Data address mark, driver will
         *  read data from Data field to sector buffer. If occure failed,
         *  driver will set fault, DRQ_STAT flag and trigger a interrupt.
         *  Whatever failed or succeed, driver always set 'DRQ_STAT' flag
         *  after reading sector. When command executed, the command register
         *  will hold the Cylinder, head and sector number for last reading.
         *
         *  As for reading multip sectors, driver will set DRQ_STAT flag,
         *  clear BUSY_STAT flag and trigger a interrupt when ready to send
         *  a sector data to CPU. When transfer complete, driver will reset
         *  'DRQ_STAT' and 'BUSY_STAT' flag. But on reading last sector,
         *  driver will set 'BUSY_STAT' flag. When command succeed to execute,
         *  the command register will hold the Cyliner, head and sector 
         *  number for last reading.
         *
         *  If obtain an erroneous erron on reading multip sectors, driver
         *  will stop read on fault sector. As same time, the command register
         *  will hold the Cylinder, head and sector number for fault sector.
         *  Wtatwver fault can be fixed, driver alway transfer data to buffer.
         */
        outb_p(WIN_READ | (cmd & 0x1), HD_COMMAND);
        break;
    case WIN_WRITE:
        /* WIN_WRITE: 0x30 Retry write buffer - 0x31 Non-Retry write buffer 
         *
         *  WIN_WRITE can write specifical 1 to 256 sector to driver. If
         *  specifical number is 0, and then 256 sectors will be write to 
         *  driver. When driver receive this command, and driver will set
         *  DRQ_STAT flag and wait buffer fill to the full. Driver doesn't
         *  trigger interrupt when first fill data to buffer, but buffer is
         *  filled to the full, driver will reset DRQ and set 'BUSY_STAT'
         *  flag.
         *
         *  As for write a sector operation, driver alway set DRQ_STAT flag
         *  and wait buffer fill to the full when receives this command.
         *  Once transfer finish, driver will set BUSY_STAT and reset DRQ_STAT
         *  flag. As same as read operation, if head doesn't locate correct
         *  on track, and driver will implicitly seek correct track. Once
         *  head obtain correct track, head will locate on corresponding ID
         *  filed on track.
         *
         *  If driver read correct ID field, ECC and bufffer data will be
         *  write to disk. Driver will clear BUSY_STAT flag and trigger
         *  an interrupt after transfer a sector. And now, CPU can obtain 
         *  current state register. After execute command, The command
         *  register will hold the last Cylinder, head and sector number.
         *  
         *  During writing multip sectors, driver will set DRQ_STAT flag,
         *  clear BUSY_STAT flag and trigger an interrupt when driver ready
         *  to receive a sector from CPU except receive first sector. Once
         *  transfer a sector, driver will reset DRQ and set BUSY_STAT flag.
         *  After last sector write to disk, driver will clear BUSY_STAT flag
         *  and trigger an interrupt (DRQ_STAT has reset). On driver finish
         *  all request, command register will hold the last Cylinder, head
         *  and sector number.
         *
         *  If occure a fault on writing multip sectors, driver will stop
         *  write operation on current sector. As same time, the command
         *  register will hold Cylinder, head and sector number for fault 
         *  sector.
         */
        outb_p(WIN_WRITE | (cmd & 0x1), HD_COMMAND);
        break;
    case WIN_VERIFY:
        /* WIN_VERIFY: 0x40 Retry Read Verify - 0x41 Non-Retry Read Verify 
         *
         *  The same as read sector operation, but driver will not set 
         *  DRQ_STAT flag and not transfer data with CPU. Driver will set
         *  BUSY_STAT flag when receive WIN_VERIFY command. After verify
         *  a specify sector, driver will reset BUSY_STAT flag and trigger
         *  an interrupt. When finish execute command, command register
         *  will hold Cylinder, head and sector number for last verfiy
         *  sector.
         *
         *  If occur fault on verify multip sectors, driver will stop
         *  verifying on error sectors. The same as other command, command
         *  register will hold Cylinder, head and sector number.
         */
        outb_p(WIN_VERIFY | (cmd & 0x1), HD_COMMAND);
        break;
    case WIN_FORMAT:
        /* WIN_FORMAT -- Format track
         *
         *  The HD_NSECTOR register specific track address. When driver 
         *  receive this command, DRQ_STAT will be set and wait buffer be 
         *  filled to full. And then, driver will clear DRQ_STAT flag,
         *  set BUSY_STAT flag and begin to execute command when buffer
         *  is full.
         */
        outb_p(WIN_FORMAT, HD_COMMAND);
        break;
    case WIN_INIT:
        /* WIN_INIT: Initialize driver */
        outb_p(WIN_INIT, HD_COMMAND);
        break;
    case WIN_SEEK:
        /* WIN_SEEK: Seek operation
         *
         *  WIN_SEEK will move head to specifical track. Driver will set
         *  BUSY_STAT flag and trigger an interrupt when receive a WIN_SEEK.
         *  Before finish seeking, driver will not set SEEK_STAT (DSC, seek
         *  finish). Driver will not trigger an interrupt before seek finish.
         *  Before current seek doesn't finish, driver receive a new WIN_SEEK,
         *  thus, BUSY_SAT flag alway be set untill WIN_SEEK complete, and 
         *  then execute new WIN_SEEK.
         */
        outb_p(WIN_SEEK, HD_COMMAND);
        break;
    case WIN_DIAGNOSE:
        /* WIN_DIAGNOSE: Diagnose driver
         * 
         *  This command will dignose driver. Driver 0 will set BUSY_STAT
         *  flag within 400ns After receive WIN_DIAGNOSE.
         *
         *  If Driver 1 exists, Driver 1 and Driver 0 both be diagnose when
         *  reveice WIN_DIAGNOSE. Driver 0 will wait 5s to diagnose Driver 1.
         *  If diagnose Driver 1 failed, Driver 0 attach 0x80 on itself 
         *  diagnostic statue. Driver will set bit 4 on HD_CURRENT register
         *  and obtain status for Driver 0 when controller obtain Driver 0
         *  status Driver 1 diagnose failed. If Driver 1 succeed diagnose or
         *  Driver 1 doesn't exist, Driver 0 will store itself diagnostic
         *  status into HD_ERROR (0x1f1) register.
         *
         *  If Driver 1 doesn't exist, Driver only report itself diagnostic
         *  statue, reset BUSY_STAT and trigger an interrupt.
         */
        outb_p(WIN_DIAGNOSE, HD_COMMAND);
        break;
    case WIN_SPECIFY:
        /* WIN_SPECIFY: Establish driver argument 
         *
         *  This command uses to set Head-exchange number and loop value of 
         *  sector counter on multip sectors mode. This command only uses
         *  two register's value. The first reigster's value is HD_NSECTOR
         *  register that specific sector number and another register's value 
         *  hold on HD_CURRENT register that decream head 1 and set driver
         *  slector flag (bit 4) according to specifical driver.
         *
         *  This command to verify sector number and head for selecting. If
         *  these value are invalid, driver will not be report failed untill
         *  another command to use these value to trigger a invalid access
         *  violation.
         */
        outb_p(WIN_SPECIFY, HD_COMMAND);
        break;
    default:
        panic("Invalid HD command. trigger #GP");
        break;
    }
}

/*
 * Main Status Register (HD_STATUS)
 *
 *   When reading HD_STATUS register, it represent current driver status.
 *   Details see talbe:
 *
 *   .------------------------------------------------------------------.
 *   | bit |     Item     | Mask | Describe                             |
 *   --------------------------------------------------------------------
 *   | 0   | ERR_STAT     | 0x01 | Command execute error. When it set,  |
 *   |     |              |      | command end of error. Now, HD_ERROR  |
 *   |     |              |      | and HD_STATUS hold error information |
 *   --------------------------------------------------------------------
 *   | 1   | INDEX_STAT   | 0x02 | Receive index. When driver obtain ID |
 *   |     |              |      | on rolling, it will be set.          |
 *   --------------------------------------------------------------------
 *   | 2   | ECC_STAT     | 0x04 | ECC error. It will be set when get a |
 *   |     |              |      | recovable-data and fixup data. Drive |
 *   |     |              |      | doesn't trigger an interrupt.        |
 *   --------------------------------------------------------------------
 *   | 3   | DRQ_STAT     | 0x08 | Data request. It's set when drver is |
 *   |     |              |      | ready to transfer data with CPU.     |
 *   --------------------------------------------------------------------
 *   | 4   | SEEK_STAT    | 0x10 | Seek finish. It's set when head stop |
 *   |     |              |      | on specifical track and seek finish. |
 *   |     |              |      | Driver not change bit when occurs    |
 *   |     |              |      | error.                               |
 *   --------------------------------------------------------------------
 *   | 5   | WRERR_STAT   | 0x20 | Write fault. Drive doesn't change it |
 *   |     |              |      | when occur error.                    |
 *   --------------------------------------------------------------------
 *   | 6   | READY_STAT   | 0x40 | Ready. More see `HD_ready()`         |
 *   --------------------------------------------------------------------
 *   | 7   | BUSY_STAT    | 0x80 | Busy. More see `HD_ready()`          |
 *   --------------------------------------------------------------------
 */
static unsigned char HD_obtain_status(void)
{
    return inb_p(HD_STATUS);
}

/*
 * Hard-Disk Control Register (HD_CMD, 0x3f6, R).
 *
 *   This register is read-only. The register is used to store control byte
 *   and controls reset operation. The definition of Bitmap is same with
 *   where offset 0x08 in hard-disk base argument table. See table:
 *
 *   .------------------------------------------------------------------.
 *   | Offset | Size | Describe                                         |
 *   --------------------------------------------------------------------
 *   |        |      | Control Byte (selector of stepper speed)         |
 *   |        |      |---------------------------------------------------
 *   |        |      | Bit 0 Un-used                                    |
 *   |        |      |---------------------------------------------------
 *   |        |      | Bit 1 Reserved (0) (Close IRQ)                   |
 *   |        |      |---------------------------------------------------
 *   |        |      | Bit 2 Permite Bit                                |
 *   |        |      |---------------------------------------------------
 *   | 0x08   | Byte | Bit 3 If head > 8 is 1                           |
 *   |        |      |---------------------------------------------------
 *   |        |      | Bit 4 Un-used (0)                                |
 *   |        |      |---------------------------------------------------
 *   |        |      | Bit 5 Set when bad block locate in Cylinder + 1  |
 *   |        |      |---------------------------------------------------
 *   |        |      | Bit 6 Forbid ECC retry                           |
 *   |        |      |---------------------------------------------------
 *   |        |      | Bit 7 Forbid access retry                        |
 *   .------------------------------------------------------------------.
 */
static void HD_control_cmd(char cmd)
{
    outb_p(cmd, HD_CMD);
}

void HD_reset_controller(void)
{
    int i;

    HD_control_cmd(4);
    for (i = 0; i < 100; i++)
        nop();

    HD_control_cmd(hd2_info[0].ctl & 0x0f);
    if (HD_drive_busy())
        printk("HD-controller still busy.\n");
    if ((i = inb(HD_ERROR)) != 1)
        printk("HD-controller reset failed: %02x\n", i);
}

/*
 * Hard-disk request 
 *
 *   On operate hard-disk controller, CPU must sent argument and command.
 *   At first, send 6 bytes as argument, and send one byte as command.
 *   For all command, must send specifical 7 bytes to hard-disk controller
 *   to execute specifial command, and write to 0x1f1~0x1f7. Once 7 bytes
 *   write to registers, driver will execute command immediately.
 *
 *   @driver: Driver number.
 *   @nsect:  Sector numbers.
 *   @sect:   Original sector number.
 *   @head:   head number.
 *   @cyl:    Cylinder number.
 *   @cmd:    Hard-disk controll command.
 *   @intr_addr: interrupt handler. 
 */
void HD_out(unsigned int drive, unsigned int nsect, 
                          unsigned int sect, unsigned int head,
                          unsigned int cyl, unsigned int cmd,
                          void (*intr_addr)(void))
{
    /*
     * After write control byte to HD_CMD, CPU should write byte as follow
     * step to send argument and command.
     *
     * 1) Check controller busy state.
     *    CPU obtain Main status register value. If bit 7 (BUSY_STAT) is 0,
     *    controller is free. If bit 7 alway is set within specifical time
     *    that timeout and indicate controller alway busy. More information
     *    see 'HD_ready()'.
     */
    if (!HD_controller_ready())
        panic("HD controller not ready");
    /*
     * 2) Check driver is ready.
     *    CPU obtain Main status register value. If bit 6 (READY_STAT) is
     *    1 that determine driver is ready.
     */
 //   if (!HD_drive_busy())
 //       panic("HD driver not ready");
    /*
     * 3) Write argument and command to 0x1f0 ~ 0x1f7
     */
    do_hd = intr_addr;
    /*
     * At first, CPU should send control byte to HD_CMD (0x3f6), and 
     * establish corresponding to controll mode for hard-disk. More
     * information about HD_CMD, refer 'HD_control_cmd()'.
     */
    HD_control_cmd(hd2_info[drive].ctl);
    HD_Write_Precompensation(hd2_info[drive].wpcom);
    HD_setup_nsector(nsect);
    HD_setup_original_sector(sect);
    HD_setup_cylinder(cyl);
    HD_setup_driver_head(drive, head);
    HD_write_command(cmd);
}

/* common block entry */
int debug_hd_dev_common(void)
{
    
    if (1) {
        obtain_hard_disk_info_BIOS();
    } else {
        obtain_hard_disk_info_BIOS();
        HD_controller_ready();
        HD_setup_nsector(1);
        HD_setup_original_sector(1);
        HD_setup_driver_head(1, 1);
        HD_write_command(1);
        HD_setup_cylinder(1);
        HD_control_cmd(1);
        HD_obtain_diagnose_status();
        HD_obtain_error_status();
        HD_Write_Precompensation(1);
        HD_obtain_status();
        HD_out(0, 1, 0, 0, 0, WIN_READ, 0);
    }
    return 0;
}
