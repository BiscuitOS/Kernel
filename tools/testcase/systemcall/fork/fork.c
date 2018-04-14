/*
 * System Call: fork
 *
 * (C) 2018.03 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#define __LIBRARY__
#include <unistd.h>

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/mm.h>

#include <asm/system.h>

#include <test/debug.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

extern long last_pid;
/*
 * find a empty pid.
 */
static int d_find_empty_process(void)
{
    int i;

repeat:
    if ((++last_pid) < 0)
        last_pid = 1;
    for (i = 0; i < NR_TASKS; i++)
        if (task[i] && task[i]->pid == last_pid)
            goto repeat;
    for (i = 1; i < NR_TASKS; i++)
        if (!task[i])
            return i;

    return -EAGAIN;
}

/*
 * duplicate memory from father process.
 */
static int d_copy_mem(int nr, struct task_struct *p)
{
    unsigned long old_data_base, new_data_base, data_limit;
    unsigned long old_code_base, new_code_base, code_limit;

    code_limit = get_limit(0x0f); /* Obtain Code segment from LDT[1] */
    data_limit = get_limit(0x17); /* Obtain Data segment from LDT[2] */
    old_data_base = get_base(current->ldt[2]); /* Current data segment */
    old_code_base = get_base(current->ldt[1]); /* Current code segment */
    if (old_data_base != old_code_base)
        panic("We don't support separate I&D");
    if (data_limit < code_limit)
        panic("Bad data_limit");
    new_data_base = new_code_base = nr * 0x4000000; /* 64MB */
    p->start_code = new_code_base;
    set_base(p->ldt[1], new_code_base);
    set_base(p->ldt[2], new_data_base);
    if (d_copy_page_tables(old_data_base, new_data_base, data_limit)) {
        printk("free_page_tables: from copy_mem\n");
        d_free_page_tables(new_data_base, data_limit);
        return -ENOMEM;
    }
    return 0;
}

/*
 * Copy process
 */
static int d_copy_process(int nr, unsigned long ebp, 
                          unsigned long edi, unsigned long esi,
                          unsigned long gs, unsigned long none,
                          unsigned long ebx, unsigned long ecx,
                          unsigned long edx, unsigned long fs,
                          unsigned long es,  unsigned long ds,
                          unsigned long eip, unsigned long cs,
                          unsigned long eflags, unsigned long esp,
                          unsigned long ss)
{
    struct task_struct *p;
    struct file *f;
    int i;

    p = (struct task_struct *)get_free_page();
    if (!p)
        return -EAGAIN;

    task[nr] = p;

    /* Note! the following statement now work with gcc 4.3.2 now, and you
     * must compile _THIS_ memory without no -O of gcc.#ifndef GCC4.3 
     */
    *p = *current;
    p->state          = TASK_UNINTERRUPTIBLE;
    p->pid            = last_pid;
    p->father         = current->pid;
    p->counter        = p->priority;
    p->signal         = 0;
    p->alarm          = 0;
    p->leader         = 0;
    p->utime          = p->stime = 0;
    p->cutime         = p->cstime = 0;
    p->start_time     = jiffies;
    p->tss.back_link  = 0;
    p->tss.esp0       = PAGE_SIZE + (long)p;
    p->tss.ss0        = 0x10;  /* kernel code segment */
    p->tss.eip        = eip;
    p->tss.eflags     = eflags;
    p->tss.eax        = 0;
    p->tss.ecx        = ecx;
    p->tss.edx        = edx;
    p->tss.ebx        = ebx;
    p->tss.esp        = esp;
    p->tss.ebp        = ebp;
    p->tss.esi        = esi;
    p->tss.edi        = edi;
    p->tss.es         = es & 0xffff;
    p->tss.cs         = cs & 0xffff;
    p->tss.ss         = ss & 0xffff;
    p->tss.ds         = ds & 0xffff;
    p->tss.fs         = fs & 0xffff;
    p->tss.gs         = gs & 0xffff;
    p->tss.ldt        = _LDT(nr); /* LDT on GDT */
    p->tss.trace_bitmap = 0x80000000;
    if (last_task_used_math == current)
        __asm__("clts ; fnsave %0" :: "m" (p->tss.i387));
    if (d_copy_mem(nr, p)) {
        task[nr] = NULL;
        free_page((unsigned long)p);
        return -EAGAIN;
    }
    for (i = 0; i < NR_OPEN; i++)
        if ((f = p->filp[i]))
            f->f_count++;
    if (current->pwd)
        current->pwd->i_count++;
    if (current->root)
        current->root->i_count++;
    if (current->executable)
        current->executable->i_count++;

    set_tss_desc(gdt + (nr << 1) + FIRST_TSS_ENTRY, &(p->tss));
    set_ldt_desc(gdt + (nr << 1) + FIRST_LDT_ENTRY, &(p->ldt));
    p->state = TASK_RUNNING; /* do this last, just in case. */
    return last_pid;
}

int sys_d_fork(void)
{
    __asm__ volatile ("call d_find_empty_process\n\r"
                      "testl %%eax, %%eax\n\r"
                      "js 1f\n\r"
                      "push %%gs\n\r"
                      "pushl %%esi\n\r"
                      "pushl %%edi\n\r"
                      "pushl %%ebp\n\r"
                      "pushl %%eax\n\r"
                      "call d_copy_process\n\r"
                      "addl $20, %%esp\n\r"
                      "1: ret" ::);
    /* nothing to do, only ignre woring message on compile */
    if (0) {
        d_find_empty_process();
        d_copy_process(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    }
    return 0;
}

/* Invoke by system call: int $0x80 */
int debug_syscall_fork0(void)
{
    if (!d_fork())
        printf("Child process\n");
    return 0;
}
