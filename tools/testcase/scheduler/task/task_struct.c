/*
 * Struct task_struct
 *
 * (C) 2017.12 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/sched.h>
#include <test/task.h>

extern int find_empty_process(void);

/*
 * Obtain last valid pid
 */
extern long last_pid;
static int obtain_pid(void)
{
    unsigned int pid;

    printk("The last pid %#x\n", last_pid);
    /* find a empty pid */
    pid = find_empty_process();
    printk("Empty PID %#x\n", pid);
    return last_pid;
}

/*
 * Dump TSS information
 */
static void dump_tss(struct tss_struct *ts)
{
    printk("ESP0[%#8x]SS0[%#8x]ESP1[%#8x]SS1[%#8x]ESP2[%#8x]SS2[%#x]\n",
            ts->esp0, ts->ss0, ts->esp1, ts->ss1, ts->esp2, ts->ss2);
    printk("CR3 [%#8x]EIP[%#8x]ESP [%#8x]EBP[%#8x]EFLAGS[%#8x]\n",
            ts->cr3, ts->eip, ts->esp, ts->ebp, ts->eflags);
    printk("EAX [%#8x]EBX[%#8x]ECX [%#8x]EDX[%#8x]ESI [%#8x]EDI[%#x]\n",
            ts->eax, ts->ebx, ts->ecx, ts->edx, ts->esi, ts->edi);
    printk("ES  [%#8x]CS [%#8x] DS [%#8x] SS[%#8x] FS [%#8x]GS [%#x]\n",
            ts->es, ts->cs, ts->ds, ts->ss, ts->fs, ts->gs);
    printk("LDT %d trace_bitmap %#x back_link %d\n", ts->ldt, 
            ts->trace_bitmap, ts->back_link);
}

/*
 * Dump Task inforation
 */
static void dump_task(int pid)
{
    struct task_struct *p = task[pid];
    
    printk("===== Task %d information =====\n", pid);
    printk("Task Status %s\n", p->state ? "stop" : "runable");
    printk("Counter %d Priority %d\n", p->counter, p->priority);
    printk("Signal %d blocked %d exit_code %d\n", p->signal, p->blocked,
                          p->exit_code);
    printk("Code [%#8x - %#8x] Data  [%#8x - %#8x]\n", p->start_code,
            p->end_code, p->end_code, p->end_data);
    printk("Brk  [%#8x - %#8x] Stack [%#8x - %#8x]\n", p->brk, p->start_stack,
             p->start_stack, p->end_data);
    printk("PID %d father %d pgrp %d session %d leader %d\n", p->pid,
          p->father, p->pgrp, p->session, p->leader);
    printk("Utime %d Stime %d Cutime %d Cstime %d Start_time %d alarm %d\n",
          p->utime, p->stime, p->cutime, p->cstime, p->start_time, p->alarm);
    printk("tty %d used_match %d\n", p->tty, p->used_math);

    /* Dump TSS */
    dump_tss(&p->tss);
}

/* common task interface */
void debug_task_struct_common(void)
{
    if (1) {
    } else {
        obtain_pid();
        dump_task(0);
    }
}
