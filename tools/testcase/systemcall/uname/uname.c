/*
 * System Call: uname
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

int sys_d_uname(struct utsname *name)
{
    static struct utsname thisname = {
        "linux .0", "nodename", "release", "version", "machine"
    };
    int i;

    if (!name)
        return -ERROR;
    verify_area(name, sizeof(*name));
    for (i = 0; i < sizeof(*name); i++)
        put_fs_byte(((char *)&thisname)[i], i + (char *)name);
    return 0;
}

/* Invoke by system call: int $0x80 */
int debug_syscall_uname0(void)
{
    struct utsname name;

    d_uname(&name);
    printf("sysname:   %s\n", name.sysname);
    printf("nodename:  %s\n", name.nodename);
    printf("release:   %s\n", name.release);
    printf("version:   %s\n", name.version);
    printf("machine:   %s\n", name.machine);
    return 0;
}
