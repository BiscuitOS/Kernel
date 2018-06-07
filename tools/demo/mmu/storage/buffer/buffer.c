/*
 * Virutal Filesystem Buffer
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
#include <linux/string.h>

#include <demo/debug.h>

#ifdef CONFIG_DEBUG_BUFFER_INIT

/* nr free pages needed before buffer grows */
static int min_free_pages = 0;

static struct buffer_head *hash_table[NR_HASH];
static struct buffer_head *free_list = NULL;
static struct buffer_head *unused_list = NULL;

/* pseudo buffer header number */
static int pseudo_nr_buffer_heads = 0;
/* pseudo buffer number */
static int pseudo_nr_buffers = 0;
/* pseudo buffer memory number */
static int pseudo_buffermem = 0;

/*
 * get_more_buffer_heads()
 *  This function establish a buffer_header list that hold a serial of
 *  unused buffer header.
 *
 *  0------+------+------+------+------------------+-------+-----4k
 *  |      |      |      |      |                  |       |      |
 *  |  bh  |  bh  |  bh  |  bh  |  ....            |  bh   | hole |
 *  |    <-|-   <-|-   <-|-   <-|-               <-|-      |      |
 *  +------+------+------+------+------------------+-------+------+
 *     |                                           A
 *     |                                           |
 *     o----> NULL                  unused_list----o
 *
 */
static void get_more_buffer_heads(void)
{
    int i;
    struct buffer_head *bh;

    if (unused_list)
        return;

    if (!(bh = (struct buffer_head *)get_free_page(GFP_BUFFER)))
        return;

    for (pseudo_nr_buffer_heads += i = PAGE_SIZE / sizeof(*bh); 
                              i > 0; i--) {
        bh->b_next_free = unused_list;  /* only make link */
        unused_list = bh++;
    }
}

static struct buffer_head *get_unused_buffer_head(void)
{
    struct buffer_head *bh;

    get_more_buffer_heads();
    if (!unused_list)
        return NULL;
    bh = unused_list;
    unused_list = bh->b_next_free;
    bh->b_next_free = NULL;
    bh->b_data = NULL;
    bh->b_size = 0;
    bh->b_req  = 0;
    return bh;
}

/*
 * See fs/inode.c for the weird use of volatile..
 */
static void put_unused_buffer_head(struct buffer_head *bh)
{
    struct wait_queue *wait;

    wait = ((volatile struct buffer_head *)bh)->b_wait;
    memset((void *)bh, 0, sizeof(*bh));
    ((volatile struct buffer_head *)bh)->b_wait = wait;
    bh->b_next_free = unused_list;
    unused_list = bh;
}

/*
 * Create the appropriate buffers when given a page for data area and
 * the size of each buffer.. User the bh->b_this_page linked list to
 * follow the buffers created. Return NULL if unable to create more
 * buffers.
 *
 * 0---------+-------------+-------------+----+-------------+-----4k
 * |.........|             |             |....|             |      |
 * |.........| buffer_head | buffer_head |....| buffer_head | Hole |
 * |.........|             |             |....|             |      |
 * +---------+-------------+-------------+----+-------------+------+
 *           A             |                  |
 *           |             |                  |
 *     unused_list         |                  |
 *                         |                  |
 *                         |                  |
 * o-----------------------o                  o----o
 * |                                               |
 * |                                               |
 * |                                               |
 * V                                               V
 * 0---------------+---------------+---------------+--------------4k
 * |               |               |               |               |
 * |     BLOCK     |     BLOCK     |     BLOCK     |     BLOCK     | 
 * |               |               |               |               |
 * +---------------+---------------+---------------+---------------+
 *
 *
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
        bh->b_this_page = head;
        head = bh;
        bh->b_data = (char *)(page + offset);
        bh->b_size = size;
    }
    return head;

no_grow:
    bh = head;
    while (bh) {
        bh = bh->b_this_page;
        put_unused_buffer_head(head);
    }
    return NULL;
}

/*
 * Try to increase the number of buffers available: the size argument
 * is used to determine what kind of buffer we want.
 *
 *   o----------------------------------------------------------------o
 *   |                                                                |
 *   |                                                                |
 *   |                                                                |
 *   |          +----------+     +----------+        +----------+     |
 *   |          |          |<----|-         |<--ooo--|-         |<----o
 *   |          |  buffer  |     |  buffer  |        |  buffer  |
 *   |          |   head   |     |   head   |        |   head   |
 *   |          |         -|---->|         -|---ooo->|         -|-----o
 * free_list--->+----------+     +----------+        +----------+     |
 *   A                                                                |
 *   |                                                                |
 *   |                                                                |
 *   o----------------------------------------------------------------o
 *
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
    if (!bh) {
        free_page(page);
        return 0;
    }
    tmp = bh;
    while (1) {
        if (free_list) {
            tmp->b_next_free = free_list;
            tmp->b_prev_free = free_list->b_prev_free;
            free_list->b_prev_free->b_next_free = tmp;
            free_list->b_prev_free = tmp;
        } else {
            tmp->b_prev_free = tmp;
            tmp->b_next_free = tmp;
        }
        free_list = tmp;
        ++pseudo_nr_buffers;
        if (tmp->b_this_page)
            tmp = tmp->b_this_page;
        else
            break;
    }
    tmp->b_this_page = bh;
    pseudo_buffermem += PAGE_SIZE;
    return 1;
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
    if (!free_list)
        panic("VFS: Unable to initialize buffer free list!\n");
    return;
}

static int debug_buffer(void)
{
    debug_buffer_init();
    return 0;
}
fs_debugcall(debug_buffer);
#endif
