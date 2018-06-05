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

#define GFP_LEVEL_MASK    0xf

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

/* Private flags. */
#define MF_USED    0xffaa0055
#define MF_FREE    0x0055ffaa

/*
 * Much care has gone into making these routines in this file reentrant.
 *
 * The fancy book keeping of nbytes malloced and the like are only used
 * to report them to the user (oooohhhh, aaaaahhhh...) are not
 * protected by cli(). (If that goes wrong, So what?)
 *
 * These routines restore the interrupt status to allow calling with
 * ints off.
 */

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

#define bh_length    vp.ubh_length
#define bh_next      vp.fbh_next
#define BH(p)        ((struct block_header *)(p))

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

#define PAGE_DESC(p) ((struct page_descriptor *)(((unsigned long)(p)) & \
                                                   PAGE_MASK))

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
subsys_debugcall(extablish_kmalloc_alloctor);
#endif

#ifdef CONFIG_DEBUG_KMALLOC_FREE

#define kfrees(x) kfree_kmalloc((x), 0)

static void kfree_kmalloc(void *ptr, int size)
{
    unsigned long flags;
    int order;
    register struct block_header *p = ((struct block_header *)ptr) - 1;
    struct page_descriptor *page, *pg2;

    page = PAGE_DESC(p);
    order = page->order;
    if ((order < 0) || (order > sizeof(ksizes) / sizeof(ksizes[0])) ||
             ((long)(page->next) & ~PAGE_MASK) ||
             (p->bh_flags != MF_USED)) {
        printk("kfree of non-kmalloced memory: %p, next= %p, order=%d\n",
                  p, page->next, page->order);
        return;
    }
    if (size && size != p->bh_length) {
        printk("Trying to free pointer at %p with wrong size: %d "
                  "instead of %lu.\n",
               p, size, p->bh_length);
        return;
    }
    size = p->bh_length;
    p->bh_flags = MF_FREE; /* As of now this block is officially free */

    save_flags(flags);
    cli();
    p->bh_next = page->firstfree;
    page->firstfree = p;
    page->nfree++;

    if (page->nfree == 1) {
        /* Page went from full to one free block: put it on the firstfree */
        if (page->next) {
            printk("Page %p already on freelist dazed "
                   "and confused....\n", page);
        } else {
            page->next = ksizes[order].firstfree;
            ksizes[order].firstfree = page;
        }
    }

    /* If page is completely free, free it */
    if (page->nfree == NBLOCKS(page->order)) {
        if (ksizes[order].firstfree == page) {
            ksizes[order].firstfree = page->next;
        } else {
            for (pg2 = ksizes[order].firstfree; (pg2 != NULL) &&
                (pg2->next != page); pg2 = pg2->next)
                /* Nothing */;
            if (pg2 != NULL)
                pg2->next = page->next;
        }
        free_page((long)page);
    }
    restore_flags(flags);

    ksizes[order].nfrees++;  /* Noncritical (monitoring) admin stuff */
    ksizes[order].nbytesmalloced -= size;
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
    int order, tries, sz, i;
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
            if (p->bh_flags == MF_FREE) {
                page->firstfree = p->bh_next;
                page->nfree--;
                if (!page->nfree) {
                    ksizes[order].firstfree = page->next;
                    page->next = NULL;
                }
                restore_flags(flags);
                ksizes[order].nmallocs++;
                ksizes[order].nbytesmalloced += size;
                /* As of now this block is officially in use */
                p->bh_flags  = MF_USED;
                p->bh_length = size;
                /* Pointer arithmetic: increments past header */
                return p + 1;
            }
            printk("Problem: block on freelist at %08lx "
                   "isn't free.\n", (long)p);
            return (NULL);
        }
        restore_flags(flags);

        /* Now we're in trouble: We need to get a new free page...... */
        /* sz is the size of the blocks we're dealing with this */
        sz = BLOCKSIZE(order);

        /* This can be done with ints on: This is private to this invocation */
        page = (struct page_descriptor *)__get_free_page(priority &
                                            GFP_LEVEL_MASK);
        if (!page) {
            printk("Could't get a free page....\n");
            return NULL;
        }
        ksizes[order].npages++;

        /* 
         * Loop for all but last block: 
         *
         *  Kmalloc Page
         *  0----------------------------------------------------------4k
         *  |                 |              |             |           |
         *  | page_descriptor | block_header | memory area | .....     |
         *  |                 |              |             |           |
         *  ------------------------------------------------------------
         *                                                      |
         *                     |--------------------------------|
         *                     |
         *                     V
         *  ----------------------------------------------------------4k
         *  |              |             |              |             |
         *  | block_header | memory area | block_header | memory area |
         *  |              |             |              |             |
         *  -----------------------------------------------------------
         *                     |
         *         |-----------|
         *         |
         *         V
         *  0----------------------------------------------sz
         *  |                                               |
         *  | Special size for memory area which store data |
         *  |      Length = BLOCKSIZE(order)                |
         *  -------------------------------------------------
         *
         */
        for (i = NBLOCKS(order), p = BH(page + 1); i > 1;
               i--, p = p->bh_next) {
            p->bh_flags = MF_FREE;
            p->bh_next  = BH(((long)p) + sz);
        }
        /* Last block: */
        p->bh_flags = MF_FREE;
        p->bh_next = NULL;

        page->order = order;
        page->nfree = NBLOCKS(order);
        page->firstfree = BH(page + 1);
        /* Now we're going to muck with the "global" freellist for this size:
           this should be uninterruptible */
        cli();
        /* 
         * sizes[order].firstfree used to be NULL, otherwise we wouldn't be
         * here, but you never know....
         *
         *  ksizes[order].firsetfree -------->  0 .-----------------.
         *    A                                   | page_descriptor |
         *    |                               I---|           .next |
         *    |                               |   |                 |
         *    |                               |   |      PAGE       |
         *    |      0 .-----------------. <--|   |                 |
         *    |        | page_descriptor |        |                 |
         *    L--------|           .next |        |                 |
         *             |                 |     4k .-----------------.
         *             |      PAGE       |
         *             |                 |
         *             |                 |
         *             |                 |
         *          4k .-----------------.
         */
        page->next = ksizes[order].firstfree;
        ksizes[order].firstfree = page;
        restore_flags(flags);
    }

    /* Pray that printk won't cause this to happen again :-) */
    printk ("Hey. This is very funny. I tried %d times ""to allocate a whole\n"
        "new page for an object only %d bytes long, but some other process\n"
        "beat me to actually allocating it. Also note that this 'error'\n"
        "message is soooo very long to catch your attention. I'd appreciate\n"
        "it if you'd be so kind as to report what conditions caused this to\n"
        "the author of this kmalloc: wolff@dutecai.et.tudelft.nl.\n"
        "(Executive summary: This can't happen)\n",
                MAX_GET_FREE_PAGE_TRIES, size);

    return NULL;
}

#ifdef CONFIG_DEBUG_KMALLOC_STRUCTURE
/*
 * Due to structure for kmalloc area, each block contains memory area and
 * block_header. And each page start of page_descriptor. As figure:
 *
 * 0-----------------------------------------------------------------4k
 * |                 |       |              |             |          |
 * | page_descriptor | ..... | block_header | memory area | ........ |  
 * |                 |       |              |             |          |
 * -------------------------------------------------------------------
 *                                          A
 *                                          |
 *                                          |---> addr
 *
 */
static void analyse_kmalloc_structure(unsigned long *addr)
{
    struct block_header *header;
    struct page_descriptor *page;

    /* Obtain block_header */
    header = ((struct block_header *)addr) - 1;
    page = PAGE_DESC(addr);

    printk("Page Order: %d\n", page->order);
    printk("Page nfree: %d\n", page->nfree);

    printk("Block flag: %#08x\n", (unsigned int)header->bh_flags);
    printk("Block size: %d\n", (int)header->bh_length);
}
#endif

static int debug_kmalloc_usage(void)
{
#ifdef CONFIG_FIRST_ALLOC_MEMORY
    unsigned long kmalloc_vma0, kmalloc_vma1;

    kmalloc_vma0 = (unsigned long)kmalloc_alloc(48, GFP_KERNEL);
    if (!kmalloc_vma0) {
        printk("Unable obtain memory from kmalloc0.\n");
        return -1;
    }
    kmalloc_vma1 = (unsigned long)kmalloc_alloc(48, GFP_KERNEL);
    if (!kmalloc_vma1) {
        printk("Unable obtain memory from kmalloc1.\n");
        return -1;
    }

    printk("VMA0 %#x\n", (unsigned int)kmalloc_vma0);
    printk("VMA1 %#x\n", (unsigned int)kmalloc_vma1);

#endif

#ifdef CONFIG_DEBUG_KMALLOC_FREE
    unsigned long kmalloc_vma;

    kmalloc_vma = (unsigned long)kmalloc_alloc(48, GFP_KERNEL);
    if (!kmalloc_vma) {
        printk("Unable obtain memory from kmalloc..\n");
        return -1;
    }
    printk("Kfree %#x\n", (unsigned int)kmalloc_vma);
    kfrees((void *)kmalloc_vma);
#endif

#ifdef CONFIG_ALLOC_MORE_PAGE
    unsigned long kmalloc_vma2, kmalloc_vma3;
    int i, order;
    int special_size = 56;

    /* get memory order */
    order = get_order(special_size);

    /* first allocate memory */
    kmalloc_vma2 = (unsigned long)kmalloc_alloc(special_size, GFP_KERNEL);
    if (!kmalloc_vma2) {
        printk("Unable obtain memory from kmalloc0.\n");
        return -1;
    }

    for (i = 0; i < (NBLOCKS(order) + 4); i++) {
        kmalloc_vma3 = (unsigned long)kmalloc_alloc(special_size, GFP_KERNEL);
        if (!kmalloc_vma3) {
            printk("Unable obtain memory from kmallocx.\n");
            return -1;
        }

    }

    /* allocate memory from new page */
    kmalloc_vma3 = (unsigned long)kmalloc_alloc(special_size, GFP_KERNEL);
    if (!kmalloc_vma3) {
        printk("Unable obtain memory from kmalloc1.\n");
        return -1;
    }

    /* Check whether memory locate on different page. */
    if (PAGE_ALIGN(kmalloc_vma2) == PAGE_ALIGN(kmalloc_vma3))
        printk("VMA0 and VMA1 from same Page.\n");
    else
        printk("VMA0 and VMA1 from different Page.\n");
#endif

#ifdef CONFIG_ALLOC_WITH_GFP_BUFFER
    unsigned long kmalloc_vma5;

    kmalloc_vma5 = (unsigned long)kmalloc_alloc(62, GFP_BUFFER);
    if (!kmalloc_vma5) {
        printk("Unable obtain memory from kmalloc.\n");
        return -1;
    }
    printk("GFP_BUFFER %#x\n", (unsigned int)kmalloc_vma5);
#endif

#ifdef CONFIG_ALLOC_WITH_GFP_ATOMIC
    unsigned long kmalloc_vma6;

    kmalloc_vma6 = (unsigned long)kmalloc_alloc(62, GFP_ATOMIC);
    if (!kmalloc_vma6) {
        printk("Unable obtain memory from kmalloc.\n");
        return -1;
    }
    printk("GFP_ATOMIC %#x\n", (unsigned int)kmalloc_vma6);
#endif

#ifdef CONFIG_ALLOC_WITH_GFP_USER
    unsigned long kmalloc_vma7;

    kmalloc_vma7 = (unsigned long)kmalloc_alloc(62, GFP_USER);
    if (!kmalloc_vma7) {
        printk("Unable obtain memory from kmalloc.\n");
        return -1;
    }
    printk("GFP_USER %#x\n", (unsigned int)kmalloc_vma7);
#endif

#ifdef CONFIG_ALLOC_WITH_GFP_KERNEL
    unsigned long kmalloc_vma8;

    kmalloc_vma8 = (unsigned long)kmalloc_alloc(62, GFP_KERNEL);
    if (!kmalloc_vma8) {
        printk("Unable obtain memory from kmalloc.\n");
        return -1;
    }
    printk("GFP_KERNEL %#x\n", (unsigned int)kmalloc_vma8);
#endif

#ifdef CONFIG_DEBUG_KMALLOC_STRUCTURE
    unsigned long *addr;

    addr = (unsigned long *)kmalloc_alloc(62, GFP_KERNEL);
    if (!addr) {
        printk("Unable obtain memory from kmalloc.\n");
        return -1;
    }
    analyse_kmalloc_structure(addr);
#endif

#ifdef CONFIG_ALLOC_OVER_MAX_KMALLOC_K
    unsigned long kmalloc_vma4;
    unsigned long size = MAX_KMALLOC_K * 1024 + 8;

    kmalloc_vma4 = (unsigned long)kmalloc_alloc(size, GFP_KERNEL);
    if (!kmalloc_vma4) {
        printk("Unable obtain memory from kmalloc.\n");
        return -1;
    }
    printk("Kmalloc address %#x\n", (unsigned int)kmalloc_vma4);
#endif

    return 0;
}
late_debugcall(debug_kmalloc_usage);
#endif
