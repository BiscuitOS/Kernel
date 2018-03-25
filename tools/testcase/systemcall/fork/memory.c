/*
 * memory.c
 *
 * (C) 2018.03 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/mm.h>
#include <test/debug.h>

#define LOW_MEM          0x100000
#define PAGING_MEMORY    (15 * 1024 * 1024)
#define PAGING_PAGES     (PAGING_MEMORY >> 12)

#define invalidate() \
    __asm__ ("movl %%eax, %%cr3" :: "a" (0))

extern unsigned char mem_map[PAGING_PAGES];

/*
 * copy page table
 */
int d_copy_page_tables(unsigned long from, unsigned long to, long size)
{
    unsigned long *from_dir, *to_dir;
    unsigned long *from_page_table;
    unsigned long *to_page_table;
    unsigned long this_page;
    unsigned long nr;

    /* PDE alignment check: linear address [21:0] must be clear */
    if ((from & 0x3fffff) || (to & 0x3fffff))
        panic("copy_page_table called with wrong alignment");
    from_dir = (unsigned long *)((from >> 20) & 0xFFC); /* __pg_dir = 0 */
    to_dir   = (unsigned long *)((to >> 20) & 0xFFC);
    size = (unsigned)((size + 0x3fffff) >> 22);
    for ( ; size-- > 0; from_dir++, to_dir++) {
        /* Check PDE P flag */
        if (1 & *to_dir)
            panic("copy_page_tables: already exist");
        if (!(1 & *from_dir))
            continue;
        from_page_table = (unsigned long *)(0xfffff000 & *from_dir);
        if (!(to_page_table = (unsigned long *)get_free_page()))
            return -1; /* Out of memory, see freeing */
        /* Addition: Set Write/Read, U/S and P flag */
        *to_dir = ((unsigned long)to_page_table) | 7;
        nr = (from == 0) ? 0xA0 : 1024;
        /* Copy old PTE contents to new PTE */
        for ( ; nr-- > 0; from_page_table++, to_page_table++) {
            this_page = *from_page_table;
            if (!(1 & this_page))
               continue;
            /* Only read this 4-KByte page */
            this_page &= ~2;
            *to_page_table = this_page;
            if (this_page > LOW_MEM) {
                *from_page_table = this_page;
                this_page -= LOW_MEM;
                this_page >>= 12;
                mem_map[this_page]++;
            }
        }
    }
    invalidate();
    return 0;
}

/*
 * free page table
 */
int d_free_page_tables(unsigned long from, unsigned long size)
{
    unsigned long *pg_table;
    unsigned long *dir, nr;

    if (from & 0x3fffff)
        panic("free_page_tables called while wrong alignment");
    if (!from)
        panic("Trying to free up swapper memory space");
    size = (size + 0x3fffff) >> 22;
    dir = (unsigned long *)((from >> 20) & 0xffc); /* __pg_dir = 0 */
    for ( ; size-- > 0; dir++) {
        if (!(1 & *dir))
            continue;
        pg_table = (unsigned long *)(0xfffff000 & *dir);
        for (nr = 0; nr < 1024; nr++) {
            if (1 & *pg_table)
                free_page(0xfffff000 & *pg_table);
            *pg_table = 0;
            pg_table++;
        }
        free_page(0xfffff000 & *dir);
        *dir = 0;
    }
    invalidate();
    return 0;
}
