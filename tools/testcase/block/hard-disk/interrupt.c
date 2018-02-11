/*
 * Interupt handler for Hard Disk request.
 *
 * (C) 2018.02 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/blk.h>
#include <linux/hdreg.h>
#include <linux/sched.h>
#include <asm/io.h>

#include <test/debug.h>

/* Max read/write errors/sector */
#define MAX_ERRORS                 7

#define MAJOR_NR                   3
#define DEVICE_NAME                "harddisk"
#define DEVICE_NR(device)          (MINOR(device) / 5)

#define port_read(port, buf, nr) \
    __asm__ ("cld;rep;insw" :: "d" (port), "D" (buf), "c" (nr))

#define port_write(port, buf, nr) \
    __asm__ ("cld;rep;outsw" :: "d" (port), "S" (buf), "c" (nr))

static int reset = 0;
static int recalibrate = 0;

static inline void unlock_buffer(struct buffer_head *bh)
{
    if (!bh->b_lock)
        printk(": free buffer being unlocked.\n");
    bh->b_lock = 0;
    wake_up(&bh->b_wait);
}

static inline void end_request(int uptodate)
{
    if (blk_dev[MAJOR_NR].current_request->bh) {
        blk_dev[MAJOR_NR].current_request->bh->b_uptodate = uptodate;
        unlock_buffer(blk_dev[MAJOR_NR].current_request->bh);
    }
   if (!uptodate) {
       printk("I/O error\n\r");
       printk("dev %04x, block %d\n\r", 
           blk_dev[MAJOR_NR].current_request->dev,
           blk_dev[MAJOR_NR].current_request->bh->b_blocknr);
   }
   wake_up(&blk_dev[MAJOR_NR].current_request->waiting);
   wake_up(&wait_for_request);
   blk_dev[MAJOR_NR].current_request->dev = -1;
   blk_dev[MAJOR_NR].current_request = 
                     blk_dev[MAJOR_NR].current_request->next;
}

static int win_result(void)
{
    int i = inb_p(HD_STATUS);

    if ((i & (BUSY_STAT | READY_STAT | WRERR_STAT | SEEK_STAT | ERR_STAT))
         == (READY_STAT | SEEK_STAT))
        return 0; /* OK */
    if (i & 1)
        i = inb(HD_ERROR);
    return 1;
}

static void bad_rw_intr(void)
{
    if (++blk_dev[MAJOR_NR].current_request->errors >= MAX_ERRORS)
        end_request(0);
    if (blk_dev[MAJOR_NR].current_request->errors > MAX_ERRORS / 2)
        reset = 1;
}

void HD_recal_intr(void)
{
    if (win_result())
        bad_rw_intr();
    HD_do_request();
}

static void HD_write_intr(void)
{
    if (win_result()) {
        bad_rw_intr();
        HD_do_request();
        return;
    }
    if (--blk_dev[MAJOR_NR].current_request->nr_sectors) {
        blk_dev[MAJOR_NR].current_request->sector++;
        blk_dev[MAJOR_NR].current_request->buffer += 512;
        do_hd = &HD_write_intr;
        port_write(HD_DATA, blk_dev[MAJOR_NR].current_request->buffer, 256);
        return;
    }
    end_request(1);
    HD_do_request();
}

static void HD_read_intr(void)
{
    if (win_result()) {
        bad_rw_intr();
        HD_do_request();
        return;
    }
    port_read(HD_DATA, blk_dev[MAJOR_NR].current_request->buffer, 256);
    blk_dev[MAJOR_NR].current_request->errors = 0;
    blk_dev[MAJOR_NR].current_request->buffer += 512;
    blk_dev[MAJOR_NR].current_request->sector++;

    if (--blk_dev[MAJOR_NR].current_request->nr_sectors) {
        do_hd = &HD_read_intr;
        return;
    }
    end_request(1);
    HD_do_request();
}

static void reset_hd(int nr)
{
    HD_reset_controller();
    HD_out(nr, hd2_info[nr].sect, hd2_info[nr].sect, hd2_info[nr].head - 1,
           hd2_info[nr].cyl, WIN_SPECIFY, &HD_recal_intr);
}

void HD_do_request(void)
{
    unsigned int block, dev;
    unsigned int sec, head, cyl;
    unsigned int nsect;
    int i, r = 0;

repeat:
    if (!blk_dev[MAJOR_NR].current_request)
        return;
    if (MAJOR(blk_dev[MAJOR_NR].current_request->dev) != MAJOR_NR)
        panic(DEVICE_NAME ": request list destroyed");
    if (blk_dev[MAJOR_NR].current_request->bh) {
        if (!blk_dev[MAJOR_NR].current_request->bh->b_lock)
            panic(DEVICE_NAME ": block not locked");
    }

    dev = MINOR(blk_dev[MAJOR_NR].current_request->dev);
    block = blk_dev[MAJOR_NR].current_request->sector;

    if (dev >= 5 * NR_HD || block + 2 > hd2[dev].nr_sects) {
        end_request(0);
        goto repeat;
    }

    block += hd2[dev].start_sect;
    dev /= 5;
    __asm__ ("divl %4": "=a" (block), "=d" (sec) : "0" (block), "1" (0),
             "r" (hd2_info[dev].sect));
    __asm__ ("divl %4": "=a" (cyl), "=d" (head) : "0" (block), "1" (0),
             "r" (hd2_info[dev].head));
    sec++;
    nsect = blk_dev[MAJOR_NR].current_request->nr_sectors;
    if (reset) {
        reset = 0;
        recalibrate = 1;
        reset_hd(DEVICE_NR(blk_dev[MAJOR_NR].current_request->dev));
        return;
    }

    if (recalibrate) {
        recalibrate = 0;
        HD_out(dev, 
          hd2_info[DEVICE_NR(blk_dev[MAJOR_NR].current_request->dev)].sect,
               0, 0, 0, WIN_RESTORE, &HD_recal_intr);
        return;
    }

    if (blk_dev[MAJOR_NR].current_request->cmd == WRITE) {
        HD_out(dev, nsect, sec, head, cyl, WIN_WRITE, &HD_write_intr);
        for (i = 0; i < 3000 && !(r = inb_p(HD_STATUS) & DRQ_STAT); i++)
            /* nothing */;
        if (!r) {
            bad_rw_intr();
            goto repeat;
        }
        port_write(HD_DATA, blk_dev[MAJOR_NR].current_request->buffer, 256);
    } else if (blk_dev[MAJOR_NR].current_request->cmd == READ) {
        HD_out(dev, nsect, sec, head, cyl, WIN_READ, &HD_read_intr);
    } else
        panic("unknow hd-command");
}

/* common HD interrupt entry */
int debug_hd_interrupt_common(void)
{
    if (1) {
        ;
    } else {
        win_result();
        bad_rw_intr();
        HD_do_request();
    }
    return 0;
}
