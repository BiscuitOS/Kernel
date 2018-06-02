/*
 * Kmalloc memory allocator on MMU
 *
 * (C) 2018.06.01 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/page.h>
#include <linux/mm.h>

#include <asm/system.h>

#include <test/debug.h>

/*
 * I want this low enough for a while to catch errors.
 * I want this number to be increased in the near future.
 *   loadable device drivers should use this function to get memory.
 */
#define MAX_KMALLOC_K     4

/*
 * This defines how many times we should try to allocate a free page
 * before giving up. Normally this shouldn't happen at all.
 */
#define MAX_GET_FREE_PAGE_TRIES    4

/*
 * A block header. This is in fromt of every malloc-block,
 * whether free or not.
 */
struct block_header {
    unsigned long bh_flags;
    union {
        unsigned long ubh_length;
        struct block_header *fbh_next;
    } vp;
};

/*
 * The page descriptor is at the front of every page that malloc
 * has in use.
 */
struct page_descriptor {
    struct page_descriptor *next;
    struct block_header *firstfree;
    int order;
    int nfree;
};

/*
 * A size descriptor describes a specific class of malloc sizes.
 * Each class of sizes has its own fresslist.
 */
struct size_descriptor {
    struct page_descriptor *firstfree;
    int size;
    int nblocks;
    int nmallocs;
    int nfrees;
    int nbytesmalloced;
    int npages;
};

#define NBLOCKS(order)         (ksizes[order].nblocks)
#define BLOCKSIZE(order)       (ksizes[order].size)

static struct size_descriptor ksizes[] = {
    { NULL,    32,  127, 0, 0, 0, 0 },
    { NULL,    64,   63, 0, 0, 0, 0 },
    { NULL,   128,   31, 0, 0, 0, 0 },
    { NULL,   252,   16, 0, 0, 0, 0 },
    { NULL,   508,    8, 0, 0, 0, 0 },
    { NULL,  1020,    4, 0, 0, 0, 0 },
    { NULL,  2040,    2, 0, 0, 0, 0 },
    { NULL,  4080,    1, 0, 0, 0, 0 },
    { NULL,    0,     0, 0, 0, 0, 0 },
};

#ifdef CONFIG_DEBUG_KMALLOC_ALLOCATOR
/*
 * Extablish a kmalloc allocator.
 */
static int extablish_kmalloc_alloctor(void)
{
    int order;

    /*
     * Check the static info array. Things will blow up terribly
     * if it's incorrect. This is a late "compile time" check ....
     */
    for (order = 0; BLOCKSIZE(order); order++) {
        if ((NBLOCKS(order) * BLOCKSIZE(order) +
            sizeof(struct page_descriptor)) > PAGE_SIZE) {
            printk("Cannot use %d bytes out of %d in order = %d "
                   "block mallocs\n",
                   NBLOCKS(order) * BLOCKSIZE(order) + 
                   sizeof(struct page_descriptor),
                   (int) PAGE_SIZE,
                   BLOCKSIZE(order));
            panic("This only happens if someone messes with kmalloc");
        }
    }

    return 0;
}
#endif

#ifdef CONFIG_DEBUG_KMALLOC_USAGE

static int get_order(int size)
{
    int order;

    /* Add the size of the header */
    size += sizeof(struct block_header);
    for (order = 0; BLOCKSIZE(order); order++)
        if (size <= BLOCKSIZE(order))
            return order;
    return -1;
}
/*
 * Allocate memory in 'kmalloc'
 */
static void *kmalloc_alloc(size_t size, int priority)
{
    int order, tries;
    unsigned long flags;
    extern unsigned long intr_count;
    struct page_descriptor *page;
    struct block_header *p;

    /* Sanity check ... */
    if (intr_count && priority != GFP_ATOMIC) {
        printk("kmalloc called nonatomicall from interrupt %08lx\n",
                ((unsigned long *)&ksizes)[-1]);
        priority = GFP_ATOMIC;
    }
    if (size > MAX_KMALLOC_K * 1024) {
        printk("kmalloc: I refuse to allocate %d bytes "
               "(for now max = %d).\n",
                size, MAX_KMALLOC_K * 1024);
        return (NULL);
    }

    order = get_order(size);
    if (order < 0) {
        printk("kmalloc of too large a block (%d bytes).\n", size);
        return (NULL);
    }

    save_flags(flags);

    /* It seems VERY unlikely to me that it would be possible that this
     * loop will get executed more than once. */
    tries = MAX_GET_FREE_PAGE_TRIES;
    while (tries--) {
        /* Try to allocate a "recently" free memory block */
        cli();
        if ((page = ksizes[order].firstfree) && (p = page->firstfree)) {
            printk("DDDDDD\n");
        }
        restore_flags(flags);
    }

    return NULL;
}
#endif

/* Kmalloc allocator common entory. */
int debug_kmalloc_allocator_common(void)
{
#ifdef CONFIG_DEBUG_KMALLOC_ALLOCATOR
    extablish_kmalloc_alloctor();
#endif

#ifdef CONFIG_DEBUG_KMALLOC_USAGE
    kmalloc_alloc(48, GFP_KERNEL);
#endif
    return 0;
}
