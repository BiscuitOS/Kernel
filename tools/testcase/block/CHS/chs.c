/*
 * CHS: Cylinder, Head, Sector and Platter
 *
 * (C) 2018.02 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>

/*
 * Obtian CHS of HD0
 */
static void obtain_CHS_HD0(void)
{
    /* Define in arch/x86/boot/setup.s */
    unsigned char *BIOS = (unsigned char *)0x90080;
    int cylinder;
    int track;
    int head;
    int nsector;
    int platter;
    int volume;

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
    cylinder = *(unsigned short *)BIOS;
    head     = *(unsigned char *)(2 + BIOS);
    nsector  = *(unsigned char *)(14 + BIOS);
    track = cylinder;
    platter  = head / 2;
    
    /*
     * Volume = Cylinder * Head * Sector * 512 
     * or
     * Volume = Track * Head * Sector * 512
     */
    volume = cylinder * head * nsector * 512;

    printk("HD0: %#x\n", volume);
    printk("Cylinder/Track: %#x\n", track);
    printk("Head: %#x\n", head);
    printk("NSector: %#x\n", nsector);
    printk("Platter: %#x\n", platter);
}

/* common MBR entry */
int debug_block_disk_CHS_common(void)
{
    if (1) {
        obtain_CHS_HD0();
    } else {
        obtain_CHS_HD0();
    }
    return 0;
}
