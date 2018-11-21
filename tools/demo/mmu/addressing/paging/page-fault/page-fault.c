/*
 * Page-fault mechanism
 *
 * (C) 2018.11.20 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/head.h>
#include <linux/ptrace.h>
#include <linux/sched.h>
#include <asm/system.h>

#include <demo/debug.h>

extern unsigned long pg0[1024];    /* page table for 0-4MB for everybody */

extern void die_if_kernel(char *,struct pt_regs *,long);

#define copy_page(from,to) \
    __asm__ ("cld ; rep ; movsl" : : "S" (from), "D" (to), "c" (1024))

static void __do_wp_page(unsigned long error_code, unsigned long address,
             struct task_struct *tsk, unsigned long user_esp)
{
    unsigned long *pde, pte, old_page, prot;
    unsigned long new_page;

    new_page = __get_free_page(GFP_KERNEL);
    pde = PAGE_DIR_OFFSET(tsk->tss.cr3, address);
    pte = *pde;
    if (!(pte & PAGE_PRESENT))
        goto end_wp_page;
    if ((pte & PAGE_TABLE) != PAGE_TABLE || pte >= high_memory)
        goto bad_wp_pagetable;
    pte &= PAGE_MASK;
    pte += PAGE_PTR(address);
    old_page = *(unsigned long *)pte;
    if (!(old_page & PAGE_PRESENT))
        goto end_wp_page;
    if (old_page >= high_memory)
        goto bad_wp_page;
    if (old_page & PAGE_RW)
        goto end_wp_page;
    tsk->min_flt++;
    prot = (old_page & ~PAGE_MASK) | PAGE_RW;
    old_page &= PAGE_MASK;
    if (mem_map[MAP_NR(old_page)] != 1) {
        if (new_page) {
            if (mem_map[MAP_NR(old_page)] & MAP_PAGE_RESERVED)
                ++tsk->rss;
            copy_page(old_page, new_page);
            *(unsigned long *) pte = new_page | prot;
            free_page(old_page);
            invalidate();
            return;
        }
        free_page(old_page);
        oom(tsk);
        *(unsigned long *) pte = BAD_PAGE | prot;
        invalidate();
        return;
    }
    *(unsigned long *) pte |= PAGE_RW;
    invalidate();
    if (new_page)
        free_page(new_page);
    return;
bad_wp_page:
    printk("do_wp_page: bogus page at address %08lx (%08lx)\n",
                                address, old_page);
    *(unsigned long *) pte = BAD_PAGE | PAGE_SHARED;
    send_sig(SIGKILL, tsk, 1);
    goto end_wp_page;
bad_wp_pagetable:
    printk("do_wp_page: bogus page-table at address %08lx (%08lx)\n",
                                address, pte);
    *pde = BAD_PAGETABLE | PAGE_TABLE;
    send_sig(SIGKILL, tsk, 1);
end_wp_page:
    if (new_page)
        free_page(new_page);
    return;
}

static unsigned long put_pages(struct task_struct *tsk, unsigned long page,
                 unsigned long address, int prot)
{
    unsigned long *page_table;

    if ((prot & (PAGE_MASK | PAGE_PRESENT)) != PAGE_PRESENT)
        printk("put_page: prot = %08x\n", prot);
    if (page >= high_memory) {
        printk("put_page: trying to put page %08lx at %08lx\n", page, address);
        return 0;
    }
    page_table = PAGE_DIR_OFFSET(tsk->tss.cr3, address);
    if ((*page_table) & PAGE_PRESENT)
        page_table = (unsigned long *) (PAGE_MASK & *page_table);
    else {
        printk("put_page: bad page directory entry\n");
        oom(tsk);
        *page_table = BAD_PAGETABLE | PAGE_TABLE;
        return 0;
    }
    page_table += (address >> PAGE_SHIFT) & (PTRS_PER_PAGE - 1);
    if (*page_table) {
        printk("put_page: page already exists\n");
        *page_table = 0;
        invalidate();
    }
    *page_table = page | prot;
    /* no need for invalidate */
    return page;
}

static inline void get_empty_page(struct task_struct *tsk, 
                                           unsigned long address)
{
    unsigned long tmp;

    if (!(tmp = get_free_page(GFP_KERNEL))) {
        oom(tsk);
        tmp = BAD_PAGE;
    }
    if (!put_pages(tsk, tmp, address, PAGE_PRIVATE))
        free_page(tmp);
}

static void debug_do_wp_page(unsigned long error_code, unsigned long address,
       struct task_struct *tsk, unsigned long user_esp)
{
    unsigned long page;
    unsigned long *pg_table;

    /*                                  
     *                                          Page Directory
     *                                       +-----------------+
     *                                       |                 |
     *                                       +-----------------+
     *                                       |                 |
     *                                       +-----------------+
     *                                       |                 |
     *                                       +-----------------+
     *                                       |                 |
     *                          pg_table     +-----------------+
     *                     o---------------->|      page       |
     *                     |                 +-----------------+
     *                     |                 |                 |
     *                     |                 +-----------------+
     *                     |                 |                 |
     *                     |                 +-----------------+
     *                     |                 |                 |
     * CR3                 |                 +-----------------+
     * +---------------+   |                 |                 |
     * | tsk->tss.cr3 -|--(+)--------------->+-----------------+
     * +---------------+
     */
    pg_table = PAGE_DIR_OFFSET(tsk->tss.cr3, address);
    while (1);
    page = *pg_table;

    /* Page table is empty or invalide */
    if (!page)
        return;
    if ((page & PAGE_PRESENT) && page < high_memory) {
        pg_table = (unsigned long *)((page & PAGE_MASK) + PAGE_PTR(address));
        page = *pg_table;
        if (!(page & PAGE_PRESENT))
            return;
        if (page & PAGE_RW)
            return;
        if (!(page & PAGE_COW)) {
            if (user_esp && tsk == current) {
                current->tss.cr2 = address;
                current->tss.error_code = error_code;
                current->tss.trap_no = 14;
                send_sig(SIGSEGV, tsk, 1);
                return;
            }
        }
        if (mem_map[MAP_NR(page) == 1]) {
            *pg_table |= PAGE_RW | PAGE_DIRTY;
            invalidate();
            return;
        }
        __do_wp_page(error_code, address, tsk, user_esp);
        return;
    }
    printk("bad page directory entry %08lx\n", page);
    *pg_table = 0;
}

static inline unsigned long get_empty_pgtable(struct task_struct *tsk, 
                 unsigned long address)
{
    unsigned long page;
    unsigned long *p;

    p = PAGE_DIR_OFFSET(tsk->tss.cr3, address);
    if (PAGE_PRESENT & *p)
        return *p;
    if (*p) {
        printk("get_empty_pgtable: bad page-directory entry \n");
        *p = 0;
    }
    page = get_free_page(GFP_KERNEL);
    p = PAGE_DIR_OFFSET(tsk->tss.cr3, address);
    if (PAGE_PRESENT & *p) {
        free_page(page);
        return *p;
    }
    if (*p) {
        printk("get_empty_pgtable: bad page-directory entry \n");
        *p = 0;
    }
    if (page) {
        *p = page | PAGE_TABLE;
        return *p;
    }
    oom(current);
    *p = BAD_PAGETABLE | PAGE_TABLE;
    return 0;
}

static void debug_do_no_page(unsigned long error_code, unsigned long address,
           struct task_struct *tsk, unsigned long user_esp)
{
    unsigned long tmp;
    unsigned long page;
    struct vm_area_struct *mpnt;

    page = get_empty_pgtable(tsk, address);
    if (!page)
        return;
    page &= PAGE_MASK;
    page += PAGE_PTR(address);
    tmp = *(unsigned long *)page;
    if (tmp & PAGE_PRESENT)
        return;
    ++tsk->rss;
    if (tmp) {
        ++tsk->maj_flt;
        swap_in((unsigned long *) page);
        return;
    }
    address &= 0xFFFFF000;
    tmp = 0;
    for (mpnt = tsk->mmap; mpnt != NULL; mpnt = mpnt->vm_next) {
        if (address < mpnt->vm_start)
            break;
        if (address >= mpnt->vm_end) {
            tmp = mpnt->vm_end;
            continue;
        }
        if (!mpnt->vm_ops || !mpnt->vm_ops->nopage) {
            ++tsk->min_flt;
            get_empty_page(tsk, address);
            return;
        }
        mpnt->vm_ops->nopage(error_code, mpnt, address);
        return;
    }
    if (tsk != current)
        goto ok_no_page;
    if (address >= tsk->end_data && address < tsk->brk)
        goto ok_no_page;
    if (mpnt && mpnt == tsk->stk_vma &&
               address - tmp > mpnt->vm_start - address &&
               tsk->rlim[RLIMIT_STACK].rlim_cur > mpnt->vm_end - address) {
        mpnt->vm_start = address;
        goto ok_no_page;
    }
    tsk->tss.cr2 = address;
    current->tss.error_code = error_code;
    current->tss.trap_no = 14;
    send_sig(SIGSEGV, tsk, 1);
    if (error_code & 4) /* user level access? */
        return;
ok_no_page:
    ++tsk->min_flt;
    get_empty_page(tsk, address);
}

asmlinkage void __unused page_fault_entence(struct pt_regs *regs, 
                                              unsigned long error_code)
{
    unsigned long address;
    unsigned long user_esp = 0;
    unsigned int bit;

    /* get the address */
    __asm__ ("movl %%cr2, %0" : "=r" (address));
    if (address < TASK_SIZE) {
        if (error_code & 4) { /* User mode access? */
            if (regs->eflags & VM_MASK) {
                bit = (address - 0xA0000) >> PAGE_SHIFT;
                if (bit < 32)
                    current->screen_bitmap |= 1 << bit;
            } else
                user_esp = regs->esp;
        }
        if (error_code & 1)
            debug_do_wp_page(error_code, address, current, user_esp);
        else
            debug_do_no_page(error_code, address, current, user_esp);
        return;
    }
    address -= TASK_SIZE;
    if (wp_works_ok < 0 && address == 0 && (error_code & PAGE_PRESENT)) {
        wp_works_ok = 1;
        pg0[0] = PAGE_SHARED;
        printk("This processor honours the WP bit even when in supervisor"
               " mode. Good. :-)\n");
        return;
    }
    if (address < PAGE_SIZE) {
        printk("Unable to handle kernel NULL pointer dereference");
        pg0[0] = PAGE_SHARED;
    } else
        printk("Unable to handle kernel paging request");
    printk(" at address %08lx\n", address);
    die_if_kernel("Oops", regs, error_code);
    do_exit(SIGKILL);
}

asmlinkage void debug_page_fault(void);

static int __unused page_fault_init(void)
{
    /* Replace and Register into IDT */
    set_trap_gate(14, &debug_page_fault);

    return 0;
}
arch_debugcall_sync(page_fault_init);

/* Trigger Page-fault(#PF) entence */
static int __unused trigger_PF(void)
{

    return 0;
}

