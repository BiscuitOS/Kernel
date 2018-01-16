/*
 * Debug free_list of buffer head on Buffer
 *
 * (C) 2018.1 BiscuitOS <buddy.zhang@aliyun.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/fs.h>
#include <test/debug.h>

extern struct buffer_head *start_buffer;
extern struct buffer_head *free_list_head;
/*
 * loop all member on free_list
 */
static void loop_free_list(void)
{
    struct buffer_head *head;
    struct buffer_head *bh;

    /* obtain the head of buffer */
    head = start_buffer;
    /* pre-loop free list */
    bh = head->b_next_free;

    while (bh != head) {
        printk("Buffer regin: %#x\n", bh->b_data);
        bh = bh->b_next_free;
    }
}

/*
 * Obtain specific buffer by index
 *
 *   @index: The index on free buffer list.
 */
static void obtain_buffer_by_index(int index)
{
    struct buffer_head *head;
    struct buffer_head *bh;

    /* Obtain the head of buffer */
    head = start_buffer;

    bh = head + index;

    /* information for special buffer head */
    printk("Buffer[%d] Regin: %#x\n", index, bh->b_data);
}

int debug_free_list(void)
{

    if (1) {
        loop_free_list();
        obtain_buffer_by_index(4);
    }

    /* Ignore warning */
    if (0) {
        loop_free_list();
        obtain_buffer_by_index(4);
    }
    
    return 0;
}
