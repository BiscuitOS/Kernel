/*
 * POSIX system call: chdir
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
#include <linux/fs.h>

#include <demo/debug.h>

asmlinkage int sys_demo_chdir(const char *filename)
{
    struct inode *inode;
    int error;

    error = namei(filename, &inode);
    if (error)
        return error;
    if (!S_ISDIR(inode->i_mode)) {
        iput(inode);
        return -ENOTDIR;
    }
    if (!permission(inode, MAY_EXEC)) {
        iput(inode);
        return -EACCES;
    }
    iput(current->pwd);
    current->pwd = inode;
    return 0;
}

/* build a system call entry */
inline _syscall1(int, demo_chdir, const char *, filename);

/* userland code */
static int debug_chdir(void)
{
    demo_chdir("/usr");
    return 0;
}
user1_debugcall_sync(debug_chdir);
