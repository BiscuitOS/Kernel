/*
 * POSIX system call: setup
 *
 * (C) 2018.06.07 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/unistd.h>
#include <linux/page.h>
#include <linux/genhd.h>
#include <linux/fs.h>

#include <demo/debug.h>

extern char empty_zero_page[PAGE_SIZE];
extern int *blk_size[];

static int current_minor = 0;

/*
 * This is setup by the setup-routine at boot-time
 */
#define PARAM                empty_zero_page
#define DRIVE_INFO           (*(struct drive_info_struct *)(PARAM+0x80))

static struct drive_info_struct {
    char dummy[32];
} drive_infos;

static void check_partition(struct gendisk *hd, unsigned int dev)
{
    static int first_time = 1;
    int i, minor = current_minor;
    unsigned long first_sector;
    struct buffer_head *bh;
    struct partition *p;
    int mask = (1 << hd->minor_shift) - 1;

    if (first_time)
        printk("Partition check:\n");
    first_time = 0;
    first_sector = hd->part[MINOR(dev)].start_sect;
    if (!(bh = bread(dev, 0, 1024))) {
        printk("  unable to read partition table of device %04x\n", dev);
        return;
    }
    printk("  %s%c:", hd->major_name, 'a'+(minor >> hd->minor_shift));
    current_minor += 4; /* first 'extra' minor */
    if (*(unsigned long *)(bh->b_data + 510) == 0xAA55) {
        p = (struct partition *)(0x1BE + bh->b_data);
        for (i = 1; i <= 4; minor++, i++, p++) {
            if (!(hd->part[minor].nr_sects = p->nr_sects))
                continue;
            hd->part[minor].start_sect = first_sector + p->start_sect;
            printk(" %s%c%d", hd->major_name, 'a' +
                    (minor >> hd->minor_shift), i);
            if ((current_minor & 0x3f) >= 60)
                continue;
        }
        /*
         * check for Disk Manager partition table
         */
        if (*(unsigned short *)(bh->b_data + 0xfc) == 0x55AA) {
            p = (struct partition *)(0x1BE + bh->b_data);
            for (i = 4; i < 16; i++, current_minor++) {
                p--;
                if ((current_minor & mask) >= mask - 2)
                    break;
                if (!(p->start_sect && p->nr_sects))
                    continue;
                hd->part[current_minor].start_sect = p->start_sect;
                hd->part[current_minor].nr_sects = p->nr_sects;
                printk(" %s%c%d", hd->major_name, 
                       'a' + (current_minor >> hd->minor_shift),
                              current_minor & mask);
            }
        }
    } else
        printk(" bad partition table");
    printk("\n");
    brelse(bh);
}

static void setup_dev(struct gendisk *dev)
{
    int i;
    int j = dev->max_nr * dev->max_p;
    int major = dev->major << 8;
    int drive;

    for (i = 0; i < j; i++) {
        dev->part[i].start_sect = 0;
        dev->part[i].nr_sects   = 0;
    }
    dev->init();
    for (drive = 0; drive < dev->nr_real; drive++) {
        current_minor = 1 + (drive << dev->minor_shift);
        check_partition(dev, major + (drive << dev->minor_shift));
    }
    for (i = 0; i < j; i++)
        dev->sizes[i] = dev->part[i].nr_sects >> (BLOCK_SIZE_BITS - 9);
    blk_size[dev->major] = dev->sizes;
}

/* This may be used only once, enforced by 'static int callable' */
int sys_demo_setup(void *BIOS)
{
    static int callable = 1;
    struct gendisk *p;
    int nr = 0;

    if (!callable)
        return -1;
    callable = 0;

    for (p = gendisk_head; p; p = p->next) {
        setup_dev(p);
        nr += p->nr_real;
    }
    mount_root();
    return 0;
}

static int debug_setup(void)
{

    /* Obtain hd information */
    drive_infos = DRIVE_INFO;
    demo_setup((void *) &drive_infos);
    return 0;
}
user0_debugcall_sync(debug_setup);
