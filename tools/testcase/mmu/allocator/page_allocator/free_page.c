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
#include <linux/mm.h>

#include <asm/system.h>

#include <test/debug.h>

#ifdef CONFIG_GET_ONE_FREE_PAGE

/* 
 * The following are used to make sure we don't thrash too much...
 * NOTE!! NR_LAST_FREE_PAGES must be a power of 2...
 */
#define NR_LAST_FREE_PAGES    32
static unsigned long last_free_pages[NR_LAST_FREE_PAGES] = {0, };

extern unsigned long free_page_list;
extern int nr_free_pages;

/*
 * Get physical address of first (actully last :-) free page, and mark it
 * used. If no free page left, return 0.
 *
 * Note that this is one of the most heavily called functions in the kernel.
 * so it's a bit timing-critical (especially as we have to disable interrupts
 * in it). See the above macro which does most of the work, and which is
 * optimized for a fast normal path of execution.
 */
static long _get_free_page(int priority)
{
    extern unsigned long intr_count;
    unsigned long result, flag;
    static unsigned long index = 0;

    /* This routine can be called at interrupt time via
       malloc. we want to make sure that the critical
       sections of code have interrupts disabled. -RAB
       Is this code reentrant? */
    if (intr_count && priority != GFP_ATOMIC) {
        printk("gfp called nonatomically from interrupt %08x\n",
               (unsigned int)((unsigned long *)&priority)[-1]);
        priority = GFP_ATOMIC;
    }
    save_flags(flag);
repeat:
    cli();
    if ((result = free_page_list) != 0) {
        if (!(result & ~PAGE_MASK) && result < high_memory) {
            free_page_list = *(unsigned long *)result;
            if (!mem_map[MAP_NR(result)]) {
                mem_map[MAP_NR(result)] = 1;
                nr_free_pages--;
                last_free_pages[index = 
                  (index + 1) & (NR_LAST_FREE_PAGES - 1)] = result;
                restore_flags(flag);
                return result;
            }
            printk("Free page %08lx has mem_map = %d\n",
                    result, mem_map[MAP_NR(result)]);
        } else
            printk("Result = 0x%08lx - memory map destoryed\n", result);
        free_page_list = 0;
        nr_free_pages = 0;
    } else if (nr_free_pages) {
        printk("nr_free_pages is %d, but free_page_list is empty\n", 
                   nr_free_pages);
        nr_free_pages = 0;
    }
    restore_flags(flag);
    if (priority == GFP_BUFFER)
        return 0;
    goto repeat;
}

static int free_page_one(void)
{
    unsigned long page;

    page = _get_free_page(GFP_KERNEL);
    printk("_get_free_page: %#08x\n", (unsigned int)page);
    return 0;
}
late_debugcall(free_page_one);
#endif
