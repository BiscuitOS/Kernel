/*
 * namei.c
 *
 * (C) 2018.03 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/sched.h>

#include <asm/segment.h>
#include <sys/stat.h>

#include <errno.h>

#define MAY_EXEC   1

/*
 * permission()
 *
 * is used to check for read/write/execute permission on a file.
 */
static int permission(struct m_inode *inode, int mask)
{
    int mode = inode->i_mode;

    /* Special case: not even root can read/write a delete file */
    if (inode->i_dev && !inode->i_nlinks)
        return 0;
    else if (current->euid == inode->i_uid)
        mode >>= 6;
    else if (current->egid == inode->i_gid)
        mode >>= 3;
    if (((mode & mask & 0007) == mask) || suser())
        return 1;
    return 0;
}

/*
 * get_dir()
 *
 *   Getdir traverses the pathname until it hits the topmost directory.
 *   It returns NULL on failure.
 */
static struct m_inode *get_dir(const char *pathname)
{
    char c;
    const char *thisname;
    struct m_inode *inode;
    int namelen;

    if (!current->root || !current->root->i_count)
        panic("No root inode");
    if (!current->pwd || !current->pwd->i_count)
        panic("No pwd inode");
    if ((c = get_fs_byte(pathname)) == '/') {
        inode = current->root;
        pathname++;
    } else if (c)
        inode = current->pwd;
    else
        return NULL; /* empty name is bad */
    inode->i_count++;
    while (1) {
        thisname = pathname;
        if (!S_ISDIR(inode->i_mode) || !permission(inode, MAY_EXEC)) {
            iput(inode);
            return NULL;
        }
        for (namelen = 0; (c = get_fs_byte(pathname++)) && (c != '/');
                 namelen++);
            /* nothing */;
        if (!c)
            return inode;
         if (!(bh = find_entry(&inode, thisname, namelen, &de))) {
             iput(inode);
             return NULL;
         }
    }

    return NULL;
}

/*
 * dir_namei()
 */
static struct m_inode *dir_namei(const char *pathname,
      int *namelen, const char **name)
{
    struct m_inode *dir;

    if (!(dir = get_dir(pathname)))
        return NULL;
    return NULL;
}

/*
 * d_open_namei
 */
int d_open_namei(const char *pathname, int flag, int mode,
                 struct m_inode **res_inode)
{
    const char *basename;
    int namelen;
    struct m_inode *dir;

    if (!(dir = dir_namei(pathname, &namelen, &basename)))
        return -ENOENT;
    return 0;
}
