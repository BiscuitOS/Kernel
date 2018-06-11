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

#include <asm/segment.h>

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

    /*
     * Verify whether filename over TASK_SIZE, user use virtual address
     * less than 0xC0000000.
     */
    i = (unsigned long)filename;
    if (!i || i >= TASK_SIZE)
        return -EFAULT;
    i = TASK_SIZE - i;
    error = -EFAULT;
    if (i > PAGE_SIZE) {
        i = PAGE_SIZE;
        error = -ENAMETOOLONG;
    }
    c = get_fs_byte(filename++);
    if (!c)
        return -ENOENT;
    if (!(page = __get_free_page(GFP_KERNEL)))
        return -ENOMEM;
    *result = tmp = (char *)page;
    while (--i) {
        /* Copy string from userland to kernel space. */
        *(tmp++) = c;
        c = get_fs_byte(filename++);
        if (!c) {
            *tmp = '\0';
            return 0;
        }
    }
    free_page(page);
    return error;
}
#endif

#ifdef CONFIG_DEBUG_LOOKUP_SPECIAL

#ifdef CONFIG_MINIX_FS
static int minix_lookups(struct )
#endif

static int special_lookup(struct inode *dir, const char *name, 
                             int len, struct inode **result)
{
#ifdef CONFIG_MINIX_FS
    minix_lookups(dir, name, len, result);
#endif
    return 0;
}
#endif

#ifdef CONFIG_DEBUG_INODE_LOOKUP
/*
 * lookup() looks up one part of a pathname, using the fs-dependent
 * routines (currently minix_lookup) for it. It also checks for fathers
 * (pseudo-roots, mount-points)
 */
static int lookups(struct inode *dir, const char *name, int len,
              struct inode **result)
{
    struct super_block *sb;
    int perm;

    *result = NULL;
    if (!dir)
        return -ENOENT;
    /* Check permissions before traversing mount-points */
    perm = permission(dir, MAY_EXEC);
    if (len == 2 && name[0] == '.' && name[1] == '.') {
        if (dir == current->root) {
            *result = dir;
            return 0;
        } else if ((sb = dir->i_sb) && (dir == sb->s_mounted)) {
            sb = dir->i_sb;
            iput(dir);
            dir = sb->s_covered;
            if (!dir)
                return -ENOENT;
            dir->i_count++;
        }
    }
    if (!dir->i_op || !dir->i_op->lookup) {
        iput(dir);
        return -ENOTDIR;
    }
    if (!perm) {
        iput(dir);
        return -EACCES;
    }
    if (!len) {
        *result = dir;
        return 0;
    }
#ifdef CONFIG_DEBUG_LOOKUP_SPECIAL
    return special_lookup(dir, name, len, result);
#else
    return dir->i_op->lookup(dir, name, len, result);
#endif
}

static int lookup_dir_by_name(char *pathname)
{
    struct inode *base;
    char c;
    const char *thisname;
    int len, error;
    struct inode *inode;

    base = current->pwd;
    base->i_count++;

    if ((c = *pathname) == '/') {
        iput(base);
        base = current->root;
        pathname++;
        base->i_count++;
    }
    while (1) {
        thisname = pathname;
        for (len = 0; (c = *(pathname++)) && (c != '/'); len++)
            /* nothing */
        if (!c)
            break;
        base->i_count++;
#ifdef CONFIG_DEBUG_INODE_LOOKUP
        error = lookups(base, thisname, len, &inode);
#else
        error = lookup(base, thisname, len, &inode);
#endif
        if (error) {
            iput(base);
            return error;
        }
    }
    if (!base->i_op || !base->i_op->lookup) {
        iput(base);
        return -ENOTDIR;
    }
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
    if (error < 0) {
        printk("Unable obtain file name.\n");
        return -1;
    }

    lookup_dir_by_name(tmp);

    return 0;
}

static int debug_namei(void)
{
    printf("Cover name to inode\n");
    vfs_namei("/etc/rc");
    return 0;
}
user1_debugcall_sync(debug_namei);
