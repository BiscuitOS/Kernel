/*
 * POSIX system call: fork
 *
 * (C) 2018.07.12 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/unistd.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/stat.h>
#include <linux/ptrace.h>

#include <demo/debug.h>

extern long last_pid;

static int find_empty_process(void)
{
    int free_task;
    int i, tasks_free;
    int this_user_tasks;

repeat:
    if ((++last_pid) & 0xffff8000)
        last_pid = 1;
    this_user_tasks = 0;
}

asmlinkage int sys_demo_fork(struct pt_regs regs)
{
    struct pt_regs *childregs;
    struct task_struct *p;
    int i, nr;
    struct file *f;
    unsigned long clone_flags = COPYVM | SIGCHLD;

    if (!(p = (struct task_struct *) __get_free_page(GFP_KERNEL)))
        goto bad_fork;

    nr = find_empty_process();
    return 9;

bad_fork:
    return -EAGAIN;
}

/* build a system call entry for write */
inline _syscall0(int, demo_fork);
#ifdef CONFIG_DEBUG_FORK_ORIG
static inline _syscall0(int, fork);
#endif

/* userland code */
static int debug_fork(void)
{
    int pid;

#ifdef CONFIG_DEBUG_FORK_ORIG
    pid = fork();
#endif
    pid = demo_fork();

    printf("pid: %d\n", pid);
    return 0;
}
user1_debugcall_sync(debug_fork);
