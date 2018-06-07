/*
 * System Call: times
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

#include <sys/times.h>
#include <sys/types.h>
#include <test/debug.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

int sys_d_times(struct tms *tbuf)
{
    if (tbuf) {
        verify_area(tbuf, sizeof(*tbuf));
        put_fs_long(current->utime, (unsigned long *)&tbuf->tms_utime);
        put_fs_long(current->stime, (unsigned long *)&tbuf->tms_stime);
        put_fs_long(current->cutime, (unsigned long *)&tbuf->tms_cutime);
        put_fs_long(current->cstime, (unsigned long *)&tbuf->tms_cstime);
    }
    return jiffies;
}

/* Invoke by system call: int $0x80 */
int debug_syscall_times0(void)
{
    struct tms tbuf;

    d_times(&tbuf);
    printf("Utime:  %d\n", tbuf.tms_utime);
    printf("Stime:  %d\n", tbuf.tms_stime);
    printf("Cutime: %d\n", tbuf.tms_cutime);
    printf("Cstime: %d\n", tbuf.tms_cstime);
    
    return 0;
}
