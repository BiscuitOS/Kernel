/*
 * POSIX system call: creat
 *
 * (C) 2018.07.12 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/unistd.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/stat.h>

#include <demo/debug.h>

static inline _syscall1(int, close, int, fd);
static inline _syscall3(int, read, unsigned int, fd, char *, buf,
                             unsigned int, count);
static inline _syscall3(int, lseek, unsigned int, fd, off_t, offset,
                             unsigned int, origin);
static inline _syscall3(int, write, unsigned int, fd, char *, buf,
                             unsigned int, count);

#ifdef CONFIG_DEBUG_CREAT_ORIG
static inline _syscall2(int, creat, const char *, filename, int, mode);
#endif


asmlinkage int sys_demo_creat(const char *pathname, int mode)
{
    /* Detail information see toos/demo/posix/open/open.c */
    return sys_open(pathname, O_CREAT | O_WRONLY | O_TRUNC, mode);
}

/* build a system call entry for write */
inline _syscall2(int, demo_creat, const char *, filename, int, mode);

/* userland code */
static int debug_creat(void)
{
    int fd;
    char buf[40];

#ifdef CONFIG_DEBUG_CREAT_ORIG
    fd = creat("/etc/BiscuitOS.rc2", 0);
#else
    fd = demo_creat("/etc/BiscuitOS.rc2", 0);
#endif
    if (fd) {
        write(fd, "Hello World", 10);
        lseek(fd, 0, 0);
        read(fd, buf, 10);
        printf("Read: %s\n", buf);
        close(fd);
    } else {
        printf("invalid fd handler\n");
        return -EINVAL;
    }
    
    return 0;
}
user1_debugcall_sync(debug_creat);
