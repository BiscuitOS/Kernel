/*
 * Paging mechanism on MMU
 *
 * (C) 2018.06.05 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/head.h>

#include <demo/debug.h>

#ifdef CONFIG_DEBUG_PAGING_INIT

static unsigned long memory_start = 0x100000;
static unsigned long memory_end   = 0x1000000;
/*
 * Paging init
 *   sets up the page tables - note that the first 4MB are
 *   already mapped by head.S
 *
 *   This routines also unmaps the page at virtual kernel address 0,
 *   so that we can trap those pesky NULL-reference errors in the
 *   kernel.
 */
static void debug_paging_init(unsigned long start_mem, 
                                   unsigned long end_mem)
{
    unsigned long *pg_dir;
    unsigned long *pg_table;
    unsigned long tmp;
    unsigned long address;

/*
 * Physical page 0 is special. It's not touched by Linux since BIOS
 * and SMM (for laptops with [34]86/SL chips) may need it. It is read
 * and write protected to detect null pointer references in the kernel.
 */
    start_mem = PAGE_ALIGN(start_mem);
    address = 0;
    pg_dir = swapper_pg_dir;

/*
 * pg_dir/swapper_pg_dir
 *   The page dirent contain 1024 item, each item manage 4MB virtual 
 *   address. So page dirent manage 4GB virtual address. The offset of
 *   0xC0000000 on page table is 768 (768 * 4MB = 0xC0000000). 
 *
 *   virtual memory:
 *   0-------------------------------------------3G------------4G
 *   |                                           |              |
 *   +-------------------------------------------+--------------+
 *                                               A
 *                                               |
 *                                               |
 *                         o---------------------o  
 *                         |                     
 *                        0-+-------------------------4k
 *                        | |                          |
 *     o----------------->| | Page Table .....         |
 *     |                  | |                          |
 *     |                  +-+--------------------------+
 *     |                   A
 *     |                   |
 *     |                   |
 *     |                   |
 *     |                   |
 *     |                   |
 *     |                   |
 *     |                   o----------o
 *     |                              |
 *   0---+-------------------------+-----+-------------4k
 *   |   |                         |     |              |
 *   | 0 |  swapper_pg_dir ....    | 768 | ...          |  
 *   |   |                         |     |              |
 *   +---+-------------------------+-----+--------------+
 */
    while (address < end_mem) {
        tmp = *(pg_dir + 768);   /* at virtual address 0xC0000000 */
        if (!tmp) { /* Page Table doesn't exist */
            tmp = start_mem | PAGE_TABLE;
            *(pg_dir + 768) = tmp;
            start_mem += PAGE_SIZE;
        }
        *pg_dir = tmp;   /* also map it in at 0x00000000 for init */
        pg_dir++;
        pg_table = (unsigned long *)(tmp & PAGE_MASK);
        for (tmp = 0; tmp < PTRS_PER_PAGE; tmp++, pg_table++) {
            if (address < end_mem)
                *pg_table = address | PAGE_SHARED;
            else
                *pg_table = 0;
            address += PAGE_SIZE;
        }
    }
    invalidate();
}

static int debug_paging(void)
{
    debug_paging_init(memory_start, memory_end);
    
    return 0;
}
arch_debugcall(debug_paging);
#endif
