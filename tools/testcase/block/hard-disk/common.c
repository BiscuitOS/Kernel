/*
 * Block device: Hard-disk, Floppy and Ramdisk
 *
 * (C) 2018.01 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/blk.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <asm/system.h>

#include <test/debug.h>

typedef void (*HD_handler_t) (void);

/*
 * The request-struct contains all necessary data
 * to load a nr of sectors into memory.
 */
static struct request HD_request_queue[NR_REQUEST];

/*
 * used to wait on when there are no free request.
 */
struct task_struct *HD_wait_for_request = NULL;

static inline void lock_buffer(struct buffer_head *bh)
{
    cli();
    while (bh->b_lock)
        sleep_on(&bh->b_wait);
    bh->b_lock = 1;
    sti();
}

static inline void unlock_buffer(struct buffer_head *bh)
{
    if (!bh->b_lock)
        panic("Trying to unlock an unlock buffer.\n");
    bh->b_lock = 0;
    wake_up(&bh->b_wait);
}

/*
 * HD_add_request adds a request to linked list.
 * It disables interrupts so that it can work with the 
 * request-lists in place.
 */
static void HD_add_request(struct blk_dev_struct *dev, struct request *req)
{
    struct request *tmp;

    req->next = NULL;
    cli();

    if (req->bh)
        req->bh->b_dirt = 0;
    if (!(tmp = dev->current_request)) {
        dev->current_request = req;
        sti();
        /* Invoke HD do handler */
        (dev->request_fn)();
        return;
    }
    for (; tmp->next; tmp = tmp->next)
        if ((IN_ORDER(tmp, req) ||
             !IN_ORDER(tmp, tmp->next)) &&
              IN_ORDER(req, tmp->next))
            break;
    req->next = tmp->next;
    tmp->next = req;
    sti();
}

/*
 * Establish a request for Reading/Writing.
 */
static void HD_make_request(int major, int rw, struct buffer_head *bh)
{
    struct request *req;
    int rw_ahead;

    /* WRITEA/READA is special case - it is not readlly needed, so if the
     * buffer is locked, we just forget about it. else it's a normal read */
    if ((rw_ahead = (rw == READA || rw == WRITEA))) {
        if (bh->b_lock)
            return;
        if (rw == READA)
            rw = READ;
        else
            rw = WRITE;
    }
    
    if (rw != READ && rw != WRITE)
        panic("Bad block dev command, must be R/W/RA/WA");

    lock_buffer(bh);
    if ((rw == WRITE && !bh->b_dirt) ||
        (rw == READ && bh->b_uptodate)) {
        unlock_buffer(bh);
        return;
    }

repeat:
    /* we don't allow the write-request to fill up the queue completely:
     * we want some room for reads:  take precedence. The last third
     * of the requests are only for reads.
     */
    if (rw == READ)
        req = HD_request_queue + NR_REQUEST;
    else
        req = HD_request_queue + ((NR_REQUEST * 2) / 3);

    /* Find an empty request */
    while (--req >= HD_request_queue) {
        if (req->dev < 0)
            break;
    }

    /* If none found, sleep on new requests: check for rw_ahead */
    if (req < HD_request_queue) {
        if (rw_ahead) {
            unlock_buffer(bh);
            return;
        }
        sleep_on(&HD_wait_for_request);
        goto repeat;
    }

    /* fill up the request-info, and add it to the queue */
    req->dev = bh->b_dev;
    req->cmd = rw;
    req->errors = 0;
    req->sector = bh->b_blocknr << 1;
    req->nr_sectors = 2;
    req->buffer = bh->b_data;
    req->waiting = NULL;
    req->next = NULL;
    req->bh = bh;
    HD_add_request(blk_dev + major, req);
}

/*
 * Read/Write block
 */
static void HD_rw_block(int rw, struct buffer_head *bh)
{
    unsigned int major;

    if ((major = MAJOR(bh->b_dev)) >= NR_BLK_DEV ||
       !(blk_dev[major].request_fn)) {
        printk("Trying to read non-existent block-device.\n");
        return;
    }

    HD_make_request(major, rw, bh);
}

static inline void wait_on_buffer(struct buffer_head *bh)
{
    cli();
    while (bh->b_lock)
        sleep_on(&bh->b_wait);
    sti();
}

/*
 * HD_bread() reads a specified block and returns the buffer that contains
 * it. It return NULL if the block was unreadable.
 */
struct buffer_head *HD_bread(int dev, int block)
{
    struct buffer_head *bh;

    if (!(bh = getblk(dev, block)))
        panic("Can't get a valid buffer.");

    if (bh->b_uptodate)
        return bh;

    HD_rw_block(READ, bh);
    wait_on_buffer(bh);
    if (bh->b_uptodate)
        return bh;

    brelse(bh);
    return NULL;
}

/*
 * Initialize HD request queue.
 */
static void HD_dev_init(void)
{
    int i;

    for (i = 0; i < NR_REQUEST; i++) {
        HD_request_queue[i].dev  = -1;
        HD_request_queue[i].next = NULL;
    }
}


/* common block entry */
int debug_block_hd_common(void)
{
#ifdef CONFIG_DEBUG_BLOCK_HD_DEV
    HD_handler_t handler_p;

    HD_dev_init();
    handler_p = blk_dev[3].request_fn;
    blk_dev[3].request_fn = HD_do_request;

    debug_hd_dev_common();

#ifdef CONFIG_DEBUG_BLOCK_HD_INT
    debug_hd_interrupt_common();
#endif
    
#ifdef CONFIG_DEBUG_BLOCK_HD_USAGE
    debug_block_usage_common();
#endif

    blk_dev[3].request_fn = handler_p;

#endif
    return 0;
}
