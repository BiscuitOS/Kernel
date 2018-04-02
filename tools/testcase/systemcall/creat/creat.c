/*
 * System Call: creat
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

extern int sys_open(const char *filename, int flag, int mode);

int sys_d_creat(const char *pathname, int mode)
{
    return sys_open(pathname, O_CREAT | O_TRUNC, mode);
}

/* Invoke by system call: int $0x80 */
int debug_syscall_creat0(void)
{
    int fd;

    fd = d_creat("/etc/nm.conf", 0);

    /* Create file */
    close(fd);
    return 0;
}
