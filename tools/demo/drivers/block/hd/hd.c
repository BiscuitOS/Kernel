/*
 * Hard-Disk
 *
 * (C) 2018.06.08 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/genhd.h>
#include <linux/sched.h>
#include <linux/major.h>

#include <asm/io.h>

#include <demo/debug.h>

#define MAJOR_NR    HD_MAJOR
#define HD_IRQ      14
#define MAX_HD      2

/*
 * This struct defines the HD's and their types.
 */
struct hd_i_struct {
    unsigned int head, sect, cyl, wpcom, lzone, ctl;
};

struct hd_i_struct hds_info[] = { { 0, 0, 0, 0, 0, 0}, { 0, 0, 0, 0, 0, 0} };

struct hd_struct hds[MAX_HD << 6];

static int hd_sizes[MAX_HD << 6] = {0, };
static int NR_HD = 0;

static void hd_interrupt(int unused)
{
}

/*
 * This is the harddisk IRQ description. The SA_INTERRUPT in sa_flags
 * means we run the IRQ-handler with interrupts disabled:  this is bad
 * for interrupt latency, but anything else has led to problems on some
 * mechines....
 *
 * We enable interrupts in some of the routines after making sure it's
 * safe.
 */
static struct sigaction hd_sigaction = {
    hd_interrupt,
    0,
    SA_INTERRUPT,
    NULL
};

static inline unsigned char CMOS_READ(unsigned char addr)
{
    outb_p(addr, 0x70);
    return inb_p(0x71);
}

#ifdef CONFIG_DEBUG_HD_INIT
static void HDD_init(void);
#else
static void HDD_init(void) 
{
}
#endif

static struct gendisk hds_gendisk = {
    MAJOR_NR,          /* Major number */
    "hd",              /* Major name */
    6,                 /* Bits to shift to get real from partition */
    1 << 6,            /* Number of partitions per real */
    MAX_HD,            /* maxuimu number of real */
    HDD_init,          /* init function */
    hds,               /* hd struct */
    hd_sizes,          /* block sizes */
    0,                 /* number */
    (void *) hds_info,  /* internal */
    NULL               /* next */
};

#ifdef CONFIG_DEBUG_HD_INIT
static void HDD_init(void)
{
    int drive, i;
    extern struct drive_info drive_info;
    unsigned char *BIOS = (unsigned char *) &drive_info;
    int cmos_disks;

    if (!NR_HD) {
        for (drive = 0; drive < 2; drive++) {
            hds_info[drive].cyl    = *(unsigned short *) BIOS;
            hds_info[drive].head   = *(2 + BIOS);
            hds_info[drive].wpcom  = *(unsigned short *) (5 + BIOS);
            hds_info[drive].ctl    = *(8 + BIOS);
            hds_info[drive].lzone  = *(unsigned short *) (12 + BIOS);
            hds_info[drive].sect   = *(14 + BIOS);
            BIOS += 16;
        }

        /*
         * We querry CMOS about hard disks : it could be that 
         * we have a SCSI/ESDI/etc controller that is BIOS
         * compatable with ST-506, and thus showing up in our
         * BIOS table, but not register compatable, and therefore
         * not present in CMOS.
         *
         * Furthurmore, we will assume that our ST-506 drives
         * <if any> are the primary drives in the system, and 
         * the ones reflected as drive 1 or 2.
         * 
         * The first drive is stored in the high nibble of CMOS
         * byte 0x12, the second in the low nibble.  This will be
         * either a 4 bit drive type or 0xf indicating use byte 0x19 
         * for an 8 bit type, drive 1, 0x1a for drive 2 in CMOS.
         *
         * Needless to say, a non-zero value means we have 
         * an AT controller hard disk for that drive.
         */
        if ((cmos_disks = CMOS_READ(0x12)) & 0xf0) {
            if (cmos_disks & 0x0f)
                NR_HD = 2;
            else
                NR_HD = 1;
        }
    }
    i = NR_HD;
    while (i-- > 0) {
        hds[i << 6].nr_sects = 0;
        if (hds_info[i].head > 16) {
            printk("hd.c: ST-506 interface disk with more "
                   "than 16 heads detected,\n");
            printk("  probably due to non-standard sector "
                   "translation. Giving up.\n");
            printk("  (disk %d: cyl=%d, sect=%d, head=%d)\n", i,
                    hds_info[i].cyl, hds_info[i].sect,
                    hds_info[i].head);
            if (i + 1 == NR_HD)
                NR_HD--;
            continue;
        }
        hds[i << 6].nr_sects = hds_info[i].head *
                               hds_info[i].sect * hds_info[i].cyl;
    }
    if (NR_HD) {
        if (irqaction(HD_IRQ, &hd_sigaction)) {
            printk("hd.c: unable to get IRQ%d for the "
                   "harddisk drive\n", HD_IRQ);
            NR_HD = 0;
        }
    }
    hds_gendisk.nr_real = NR_HD;
}
#endif

static int debug_hd(void)
{
#ifdef CONFIG_DEBUG_HD_INIT
    HDD_init();
#endif
    return 0;
}
late_debugcall(debug_hd);
