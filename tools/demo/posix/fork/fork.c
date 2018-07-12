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

#include <demo/debug.h>

asmlinkage int sys_demo_fork(void)
{
    return 9;
}

/* build a system call entry for write */
inline _syscall0(int, demo_fork);

/* userland code */
static int debug_fork(void)
{
    int pid;

    pid = demo_fork();

    printf("pid: %d\n", pid);
    return 0;
}
user1_debugcall_sync(debug_fork);
