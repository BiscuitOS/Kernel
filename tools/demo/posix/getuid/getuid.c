/*
 * System Call: getuid
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

#include <asm/segment.h>

#include <sys/types.h>
#include <test/debug.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

int sys_d_getuid(void)
{
    return current->uid;
}

/* Invoke by system call: int $0x80 */
int debug_syscall_getuid0(void)
{
    int uid;

    uid = d_getuid();
    printf("UID %d\n", uid);
    return 0;
}
