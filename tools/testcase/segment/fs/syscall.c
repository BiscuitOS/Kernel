/*
 * FS: exchange data between kernel and userland
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

static char *argv_rc[] = { "-/bin/sh", "/usr/bin/bash", NULL };
static char *envp_rc[] = { "HOME=/root", "PATH=/usr/bin:/bin", NULL};

int sys_d_fs(const char *file, char **argv, char **envp)
{
    printk("Hello World\n");
    return 0;
}

/* Invoke by system call: int $0x80 */
int debug_binary_aout_exec(void)
{
    d_fs("/bin/sh", argv_rc, envp_rc);
    return 0;
}
