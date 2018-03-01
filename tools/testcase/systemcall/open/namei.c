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

#include <errno.h>

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
        if (!S_ISDIR(inode->i_mode) || !permission)
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
