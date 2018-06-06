/*
 * vmalloc memory allocator on MMU
 *
 * (C) 2018.06.06 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/malloc.h>
#include <linux/sched.h>

#include <test/debug.h>

/*
 * Just any arbitrary offset to the start of the vmalloc VM area: the
 * current 8MB value just means that there will be a 8MB "hole" after
 * the physical memory until the kernel virtual memory starts. That means
 * that any out-of-bounds memory accesses will hopefully to caught.
 * The vmalloc() routines leaves a hole of 4Kb between each vmalloced
 * area for the same reason. :-)
 */
#define VMALLOC_OFFSET    (8*1024*1024)

struct vm_struct {
    unsigned long flags;
    void *addr;
    unsigned long size;
    struct vm_struct *next;
};

static struct vm_struct *vmlist = NULL;

static inline void set_pgdir(unsigned long dindex, unsigned long value)
{
    static struct task_struct *p;

    p = &init_task;
    do {
        ((unsigned long *)p->tss.cr3)[dindex] = value;
        p = p->next_task;
    } while (p != &init_task);
}

static int free_area_pages(unsigned long dindex, unsigned long index,
                              unsigned long nr)
{
    unsigned long page, *pte;

    if (!(PAGE_PRESENT & (page = swapper_pg_dir[dindex])))
        return 0;
    page &= PAGE_MASK;
    pte = index + (unsigned long *)page;
    do {
        unsigned long pg = *pte;

        *pte = 0;
        if (pg & PAGE_PRESENT)
            free_page(pg);
        pte++;
    } while (--nr);
    pte = (unsigned long *)page;
    for (nr = 0; nr < 1024; nr++, pte++)
        if (*pte)
            return 0;
    set_pgdir(dindex, 0);
    mem_map[MAP_NR(page)] = 1;
    free_page(page);
    invalidate();
    return 0;
}

/*
 * alloc_area_page
 *   This routine is used to assign page table and page to vmalloc.
 *   At first, the system will check whether page table exist, if
 *   it doesn't exist and establish it. After, the systen will point
 *   to special pte that point new page. Final, the system will
 *   allocate fixed page number from get_free_page. Now the virtual
 *   address has fixed page table and page area. So the system will
 *   invoke 'invalidate()' to refresh TLB.
 *
 *   0---------------------------------------------+--------+----4k
 *   |                                             |        |     |
 *   | swapper_pg_dir .....                        | dindex | ... |
 *   |                                             |        |     |
 *   +---------------------------------------------+--------+-----+
 *                                                     |
 *                                                     |
 *   o-------------------------------------------------o
 *   |
 *   |
 *   |
 *   V
 *   0------------------------+-------------+--------------------4k
 *   |                        |             |                     |
 *   | Page Table .....       | PTE (index) |                     |
 *   |                        |             |                     |
 *   +------------------------+-------------+---------------------+
 *                               |
 *                               |
 *   o---------------------------o
 *   |
 *   |
 *   V
 *   0-----------------------------------------------------------4k
 *   |                                                            |
 *   |  PAGE                                                      |
 *   |                                                            |
 *   +------------------------------------------------------------+
 *
 */
static int alloc_area_page(unsigned long dindex, unsigned long index,
                      unsigned long nr)
{
    unsigned long page, *pte;

    page = swapper_pg_dir[dindex];
    if (!page) { /* Page table doesn't exist. */
        page = get_free_page(GFP_KERNEL);
        if (!page)
            return -ENOMEM;
        if (swapper_pg_dir[dindex]) {
            free_page(page);
            page = swapper_pg_dir[dindex];
        } else {
            mem_map[MAP_NR(page)] = MAP_PAGE_RESERVED;
            set_pgdir(dindex, page | PAGE_SHARED);
        }
    }
    page &= PAGE_MASK;
    pte = index + (unsigned long *)page;
    *pte = PAGE_SHARED;   /* remove a race with vfree() */
    do {
        unsigned long pg = get_free_page(GFP_KERNEL);

        if (!pg)
            return -ENOMEM;
        *pte = pg | PAGE_SHARED;
        pte++;
    } while (--nr);
    invalidate();
    return 0;
}

/*
 * do_area
 *   Calculate index of pgdir and pte. If size over one page, we
 *   re-allocate two page.
 */
static int do_area(void *addr, unsigned long size,
           int (*area_fn)(unsigned long, unsigned long, unsigned long))
{
    unsigned long nr, dindex, index;

    nr = size >> PAGE_SHIFT;
    dindex = (TASK_SIZE + (unsigned long)addr) >> 22;
    index = (((unsigned long)addr) >> PAGE_SHIFT) & (PTRS_PER_PAGE - 1);
    while (nr > 0) {
        unsigned long i = PTRS_PER_PAGE - index;

        if (i > nr)
            i = nr;
        nr -= i;
        if (area_fn(dindex, index, i))
            return -1;
        index = 0;
        dindex++;
    }
    return 0;
}

void vmalloc_vfree(void *addr)
{
    struct vm_struct **p, *tmp;

    if (!addr)
        return;
    if ((PAGE_SIZE - 1) & (unsigned long)addr) {
        printk("Trying to vfree() bad address (%p)\n", addr);
        return;
    }
    for (p = &vmlist; (tmp = *p); p = &tmp->next) {
        if (tmp->addr == addr) {
            *p = tmp->next;
            do_area(tmp->addr, tmp->size, free_area_pages);
            kfree(tmp);
            return;
        }
    }
    printk("Trying to vfree() nonexistent vm area (%p)\n", addr);
}

/*
 * vmalloc_alloc
 *   Allocate a bunk virtual address area. The start address of vmalloc
 *   is 'high_memory + VMALLOC_OFFSET'. As figure
 * 
 *   0-----------+------+------------------------3G---------------4G
 *   |           |      |                         |               |
 *   |           | Hole | Vmalloc ....            |               |
 *   |           |      |                         |               |
 *   +-----------+------+-------------------------+---------------+
 *               A
 *               |
 *               |
 *         high_memory
 *
 *
 *   The vmalloc will allocate a bunk vitual address but these virtual
 *   address will map to physical page which lower then 'high_memory'.
 *   The vmalloc allocator utilize mata structure 'vm_struct' to manage
 *   a virtual area and hold a vma list named 'vmlist' to hold all 
 *   virtual area. And different other allocator, the 'vm_struct' from
 *   kmalloc and it may be belong to different physical page. As follow:
 *   
 *              +-----------+     +-----------+         +-----------+
 *              |           |     |           |         |           |
 *              | vm_struct |     | vm_struct |         | vm_struct |
 *              |           |     |           |         |           |
 *   vmlist---->+-----------+---->+-----------+--....-->+-----------+---o
 *     A                                                                |
 *     |                                                                |
 *     |                                                                |
 *     |                                                                |
 *     o----------------------------------------------------------------o
 *
 */
static void *vmalloc_alloc(unsigned long size)
{
    void *addr;
    struct vm_struct **p, *tmp, *area;

    size = PAGE_ALIGN(size);
    if (!size || size > high_memory)
        return NULL;
    area = (struct vm_struct *)kmalloc(sizeof(*area), GFP_KERNEL);
    if (!area)
        return NULL;
    addr = (void *)((high_memory + VMALLOC_OFFSET) & ~(VMALLOC_OFFSET - 1));
    area->size = size + PAGE_SIZE;
    area->next = NULL;
    for (p = &vmlist; (tmp = *p); p = &tmp->next) {
        if (size + (unsigned long)addr < (unsigned long)tmp->addr)
            break;
        addr = (void *)(tmp->size + (unsigned long)tmp->addr);
    }
    area->addr = addr;
    area->next = *p;
    *p = area;
    if (do_area(addr, size, alloc_area_page)) {
        vfree(addr);
        return NULL;
    }
    return addr;
}

static int debug_vmalloc(void)
{
    unsigned long addr;

    addr = (unsigned long)vmalloc_alloc(1025);

    printk("Vmalloc %#x\n", (unsigned int)addr);
    addr = (unsigned long)vmalloc_alloc(1025);

    printk("Vmalloc %#x\n", (unsigned int)addr);
    vmalloc_vfree((void *)addr);
    addr = (unsigned long)vmalloc_alloc(1025);

    printk("Vmalloc %#x\n", (unsigned int)addr);
    return 0;
}
late_debugcall(debug_vmalloc);
