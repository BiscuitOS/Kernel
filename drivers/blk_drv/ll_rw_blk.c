/*
 * base block device
 * Maintainer: Buddy <buddy.zhang@aliyun.com>
 *
 * Copyright (C) 2017 BiscuitOS
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/blk.h>
#include <linux/buffer.h>
#include <asm/system.h>


/*
 * The request-struct contains all necessary data
 * to load a nr of sectors into memory.
 */
static struct request request_queue[NR_REQUEST];
static int blk_dev_num;

/*
 * blk_dev_struct is:
 * do_request-address
 * next->request
 */
struct blk_dev_struct blk_dev[] = {
	{NULL, NULL},		/* no_dev */
	{NULL, NULL},		/* dev mem */
	{NULL, NULL},		/* dev fd */
	{NULL, NULL},		/* dev hd */
	{NULL, NULL},		/* dev ttyx */
	{NULL, NULL},		/* dev tty */
};

/*
 * used to wait on when there are no free request.
 */
struct task_struct *wait_for_request;

static void lock_buffer(struct buffer_head *bh)
{
	irq_disable();
	while (bh->b_lock)
		sleep_on(&bh->b_wait);
	bh->b_lock = 1;
	irq_enable();
}

static void unlock_buffer(struct buffer_head *bh)
{
	if (!bh->b_lock)
		printk("Tring to unlock an unlocked buffer.\n");

	bh->b_lock = 0;
	wake_up(&bh->b_wait);
}

static void add_request(struct blk_dev_struct *dev, struct request *req)
{
	struct request *tmp = dev->current_request;
	irq_disable();

	if (req->bh)
		req->bh->b_dirt = 1;

	if (!tmp) {
		dev->current_request = req;
		dev->request_fn();
		irq_enable();
		return;
	}

	for (; tmp; tmp = tmp->next) {
		if (IN_ORDER(tmp, req) && IN_ORDER(req, tmp->next))
			break;

		if (IN_ORDER(tmp->next, req) && IN_ORDER(req, tmp))
			break;
	}

	req->next = tmp->next;
	tmp->next = req;
	irq_enable();
}

static void make_request(int major, int cmd, struct buffer_head *bh)
{
	struct request *req;
	int read_ahead;

	read_ahead = cmd == READA ? 1 : 0;
	if (read_ahead) {
		if (buffer_locked(bh))
			return;
		cmd = READ;
	}

	if (cmd != READ && cmd != WRITE)
		panic("Unkown request command.\n");

	lock_buffer(bh);
	if ((cmd == READ && buffer_uptodated(bh)) ||
		(cmd == WRITE && buffer_dirtied(bh))) {
		unlock_buffer(bh);
		return;
	}

	/*
	 * Ready to fulfill the req, write can only use the 2/3 of the queue.
	 * It is resonable, because read is the most frequently used.
	*/
repeat:
	if (cmd == READ)
		req = request_queue + NR_REQUEST;
	else
		req = request_queue + NR_REQUEST * 2 / 3;

	while (--req > request_queue) {
		if (req->dev == -1)
			break;
	}

	if (req < request_queue) {
		if (read_ahead) {
			unlock_buffer(bh);
			return;
		}

		sleep_on(&wait_for_request);
		goto repeat;
	}

	req->dev = bh->b_dev;
	req->cmd = cmd;
	req->errors = 0;
	/* sector size 512 */
	req->sector = bh->b_blocknr << 1;
	req->nr_sectors = 2;
	req->buffer = bh->b_data;
	req->waiting = NULL;
	req->next = NULL;
	req->bh = bh;
	add_request(blk_dev + major, req);
}

void ll_rw_block(int rw, struct buffer_head *bh)
{
	unsigned int major = MAJOR(bh->b_dev);

	if (major >= blk_dev_num || !(blk_dev[major].request_fn)) {
		printk("Trying to %s noexist block device.\n",
			   rw ? "WRITE" : "READ");
		return;
	}

	make_request(major, rw, bh);
}

void blk_dev_init(void)
{
    int i;

    for (i = 0; i < NR_REQUEST; i++) {
        request_queue[i].dev = -1;
        request_queue[i].next = NULL;
    }

    blk_dev_num = sizeof(blk_dev) / sizeof(blk_dev[0]);
}
