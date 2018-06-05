/*
 * Filesystem Buffer
 *
 * (C) 2018.06.05 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/fs.h>

#include <test/debug.h>

#ifdef CONFIG_DEBUG_BUFFER_INIT

/* nr free pages needed before buffer grows */
static int min_free_pages = 0;

static struct buffer_head *hash_table[NR_HASH];
static struct buffer_head *free_list = NULL;
static struct buffer_head *unused_list = NULL;

static void get_more_buffer_heads(void)
{
    int i;
    struct buffer_head *bh;

    if (unused_list)
        return;

    if (!(bh = (struct buffer_head *)get_free_page(GFP_BUFFER)))
        return;

    for (nr_buffer_heads += i = PAGE_SIZE / sizeof(*bh); i > 0; i--) {
        bh->b_next_free = unused_list;  /* only make link */
        unused_list = bh++;
    }
}

static struct buffer_head *get_unused_buffer_head(void)
{
    struct buffer_head *bh;

    get_more_buffer_heads();
}

/*
 * Create the appropriate buffers when given a page for data area and
 * the size of each buffer.. User the bh->b_this_page linked list to
 * follow the buffers created. Return NULL if unable to create more
 * buffers.
 */
static struct buffer_head *create_buffers(unsigned long page,
                                      unsigned long size)
{
    struct buffer_head *bh, *head;
    unsigned long offset;

    head = NULL;
    offset = PAGE_SIZE;
    while ((offset -= size) < PAGE_SIZE) {
        bh = get_unused_buffer_head();
        if (!bh)
            goto no_grow;
    }
}

/*
 * Try to increase the number of buffers available: the size argument
 * is used to determine what kind of buffer we want.
 */
static int grow_buffers(int pri, int size)
{
    unsigned long page;
    struct buffer_head *bh, *tmp;

    if ((size & 511) || (size > PAGE_SIZE)) {
        printk("VFS: grow_buffers: size = %d\n", size);
        return 0;
    }
    if (!(page = __get_free_page(pri)))
        return 0;

    bh = create_buffers(page, size);
    return 0;
}

/*
 * This initializes the initial buffer free list. nr_buffer is set
 * to one less the actual number of buffer, as a sop to backwards
 * compatibility --- the old code did this (I think unintentionally,
 * but I'm not sure), and programs in the ps package expect it.
 *                                             - TYT 8/30/92
 */
static void debug_buffer_init(void)
{
    int i;

    if (high_memory >= 4 * 1024 * 1024)
        min_free_pages = 200;
    else
        min_free_pages = 20;
    for (i = 0; i < NR_HASH; i++)
        hash_table[i] = NULL;
    free_list = 0;
    grow_buffers(GFP_KERNEL, BLOCK_SIZE);
}


static int debug_buffer(void)
{
    debug_buffer_init();
    return 0;
}
fs_debugcall(debug_buffer);
#endif
