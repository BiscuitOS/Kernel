/*
 * System Call: dup
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

int sys_d_dup(unsigned int fd, char *buf, int count)
{
    return 0;
}

/* Invoke by system call: int $0x80 */
int debug_syscall_dup0(void)
{
    /* dumplicate standard error */
    dup(0);

    /* close standard error */
    close(0);
    return 0;
}
