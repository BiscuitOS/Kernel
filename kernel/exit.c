/*
 *   linux/kernel/exit.c
 *
 *   (C) 1991  Linus Torvalds
 */
#include <linux/sched.h>
#include <linux/tty.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/fs.h>

#include <sys/types.h>
#include <sys/wait.h>

#include <asm/segment.h>

#include <errno.h>

extern int sys_close(unsigned int);

void release(struct task_struct *p)
{
    int i;

    if (!p)
        return;
    for (i = 1; i < NR_TASKS; i++)
        if (task[i] == p) {
            task[i] = NULL;
            free_page((long)p);
            schedule();
            return;
        }
    panic("Trying to release non-existent task");
}

static inline int send_sig(long sig, struct task_struct *p, int priv)
{
    if (!p || sig < 1 || sig > 32)
        return -EINVAL;
    if (priv || (current->euid == p->euid) || suser())
        p->signal |= (1 << (sig - 1));
    else
        return -EPERM;
    return 0;
}

static void kill_session(void)
{
    struct task_struct **p = NR_TASKS + task;

    while (--p > &FIRST_TASK) {
        if (*p && (*p)->session == current->session)
            (*p)->signal |= 1 << (SIGHUP - 1);
    }
}

static void tell_father(int pid)
{
    int i;

    if (pid)
        for (i = 0; i < NR_TASKS; i++) {
            if (!task[i])
                continue;
            if (task[i]->pid != pid)
                continue;
            task[i]->signal |= (1 << (SIGCHLD - 1));
            return;
        }
    /* if we don't find any fathers, we just release ourselves */
    /* This is not really OK. Must change it to make father 1 */
    printk("BAD BAD - no father found\n\r");
    release(current);
}

int do_exit(long code)
{
    int i;

    free_page_tables(get_base(current->ldt[1]), get_limit(0x0f));
    free_page_tables(get_base(current->ldt[2]), get_limit(0x17));
    for (i = 0; i < NR_TASKS; i++)
        if (task[i] && task[i]->father == current->pid) {
            task[i]->father = 1;
            if (task[i]->state == TASK_ZOMBIE)
                /* assumption task[1] is always init */
                (void) send_sig(SIGCHLD, task[1], 1);
        }
    for (i= 0; i < NR_OPEN; i++)
        if (current->filp[i])
            sys_close(i);
    iput(current->pwd);
    current->pwd = NULL;
    iput(current->root);
    current->root = NULL;
    iput(current->executable);
    current->executable = NULL;
    if (current->leader && current->tty >= 0)
        tty_table[current->tty].pgrp = 0;
    if (last_task_used_math == current)
        last_task_used_math = NULL;
    if (current->leader)
        kill_session();
    current->state = TASK_ZOMBIE;
    current->exit_code = code;
    tell_father(current->father);
    schedule();
    return -1;  /* just to suppress warnings */
}

int sys_exit(int error_code)
{
    return do_exit((error_code & 0xff) << 8);
}

int sys_waitpid(pid_t pid, unsigned long *stat_addr, int options)
{
    int flag, code;
    struct task_struct **p;

    verify_area(stat_addr, 4);
repeat:
    flag = 0;
    for (p = &LAST_TASK; p > &FIRST_TASK; --p) {
        if (!*p || *p == current)
            continue;
        if ((*p)->father != current->pid)
            continue;
        if (pid > 0) {
            if ((*p)->pid != pid)
                continue;
        } else if (!pid) {
            if ((*p)->pgrp != current->pgrp)
                continue;
        } else if (pid != -1) {
            if ((*p)->pgrp != -pid)
                continue;
        }
        switch ((*p)->state) {
        case TASK_STOPPED:
            if (!(options & WUNTRACED))
                continue;
            put_fs_long(0x7f, stat_addr);
            return (*p)->pid;
        case TASK_ZOMBIE:
            current->cutime += (*p)->utime;
            current->cstime += (*p)->stime;
            flag = (*p)->pid;
            code = (*p)->exit_code;
            release(*p);
            put_fs_long(code, stat_addr);
            return flag;
        default:
            flag = 1;
            continue;
        }
    }
    if (flag) {
        if (options & WNOHANG)
            return 0;
        current->state = TASK_INTERRUPTIBLE;
        schedule();
        if (!(current->signal &= ~(1 << (SIGCHLD - 1))))
            goto repeat;
        else
            return -EINTR;
    }
    return -ECHILD;
}
