/*
 * System Call: execve
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

#include <test/debug.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

static char *argv_rc[] = { "/bin/sh", NULL };
static char *envp_rc[] = { "HOME=/", NULL };

int sys_d_execve(const char *file, char **argv, char **envp)
{
    printk("Hello World\n");
    return 0;
}

/* Invoke by system call: int $0x80 */
int debug_syscall_execve0(void)
{
    d_execve("/bin/sh", argv_rc, envp_rc);
    return 0;
}
