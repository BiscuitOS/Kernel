/*
 * namei: name to inode
 *
 * (C) 2018.06.11 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/unistd.h>
#include <linux/sched.h>

#include <demo/debug.h>

#ifdef CONFIG_DEBUG_NAMEI_FROM_USER
/*
 * In order to reduce some races, while at the same time doing additional
 * checking and hopefully speeding things up, we copy filename to the 
 * kernel data spece before using them..
 *
 * POSIX.1 2.4: an empty pathname is invalid (ENOENT)
 */
static int obtain_name_from_user(const char *filename, char **result)
{
    int error;
    unsigned long i, page;
    char *tmp, c;

    i = (unsigned long)filename;
    if (!i || i >= TASK_SIZE)
        return -EFAULT;

    printk("I %#x\n", i);

    return 0;
}
#endif

asmlinkage int sys_vfs_namei(const char *name)
{
    char *tmp;
    int error;

#ifdef CONFIG_DEBUG_NAMEI_FROM_USER
    error = obtain_name_from_user(name, &tmp);
#else
    error = getname(name, &tmp);
#endif

    return 0;
}

static int debug_namei(void)
{
    printf("Cover name to inode\n");
    vfs_namei("/etc/rc");
    return 0;
}
user1_debugcall_sync(debug_namei);
