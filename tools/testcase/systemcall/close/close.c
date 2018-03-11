/*
 * System Call: open
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

int sys_d_close(const char *filename, int flag, int mode)
{
    printk("Hello world\n");
    return 0;
}

/* Invoke by system call: int $0x80 */
int debug_syscall_close0(void)
{
    int fd;

    fd = open("/etc/biscuitos.conf", O_RDWR | O_CREAT);

    d_close(fd);
    return 0;
}
