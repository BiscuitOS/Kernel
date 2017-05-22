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

/*
 * The request-struct contains all necessary data
 * to load a nr of sectors into memory.
 */
static struct request request[NR_REQUEST];

/*
 * blk_dev_struct is:
 * do_request-address
 * next->request
 */
struct blk_dev_struct blk_dev[NR_BLK_DEV] = {
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

void blk_dev_init(void)
{
	int i;

	for (i = 0; i < NR_REQUEST; i++) {
		request[i].dev = -1;
		request[i].next = NULL;
	}
}
