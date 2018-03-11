/*
 * System Call: read
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


int sys_d_read(unsigned int fd, char *buf, int count)
{
    /* Main-routine: read */
    printk("sys_d_read: Hello World\n");
    return 0;
}

/* Invoke by system call: int $0x80 */
int debug_syscall_read0(void)
{
    int fd;
    char buff[20];

    /* open a file */
    fd = open("/etc/rc", O_RDONLY, 0);
    if (!fd) {
        d_printf("Can't open special file\n");
        return -1;
    }

    /* Read data from file */
    d_read(fd, buff, 10);
    buff[10] = '\0';
    d_printf("READ: %s\n", buff);

    /* close fd */
    close(fd);
    return 0;
}
