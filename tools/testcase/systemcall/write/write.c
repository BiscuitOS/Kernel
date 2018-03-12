/*
 * System Call: write
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

int sys_d_write(unsigned int fd, char *buf, int count)
{
}

/* Invoke by system call: int $0x80 */
int debug_syscall_write0(void)
{
    int fd;
    char buf[20] = "BiscuitOS";

    fd = open("/etc/biscuitos.cnf", O_RDWR | O_CREAT, 0);
    if (!fd) {
        d_printf("Unable to open /etc/biscuitos.conf\n");
        return -1;
    }

    /* write data into file */
    write(fd, buf, strlen(buf));

    /* Close file */
    close(fd);
    return 0;
}
