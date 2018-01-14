/*
 * Page-Dirent Machanism on MMU
 *
 * (C) 2017.12 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/head.h>

#include <test/mm.h>
/*
 * Obtain global page-dirent.
 */
static inline struct pgdir_node *obtain_pgdir(void)
{
    return (struct pgdir_node *)pg_dir;
}

/*
 * Obtain current page-dirent from CR3 register.
 */
static inline struct pgdir_node *obtain_current_pgdir(void)
{
    unsigned int CR3;

    __asm__ ("movl %%cr3, %0" : "=r" (CR3));


    return (struct pgdir_node *)CR3;
}

/*
 * Dump kernel 0-3 page table.
 */
static void dump_kernel_page_table(void)
{
    int i;
    struct pgdir_node *pg;

    /* Obtain globl page-dirent */
    pg = obtain_pgdir();

    /* Dump 0-3 kernel page-table */
    for (i = 0; i < 4; i++)
       printk("Kernel Page-Table%d: %#x\n", i, pg[i]);
}

/* common linear address entry */
void debug_pgdir_common(void)
{
    if (1) {
        dump_kernel_page_table();
    } else {
        obtain_current_pgdir();
        dump_kernel_page_table();
    }
}
