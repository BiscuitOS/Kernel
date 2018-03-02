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
 * find_entry()
 *
 * finds an entry in the specified directory with the wanted name. It
 * returns the cache buffer in which the entry wa found, and the entry
 * itself (as a parameter - res_dir). It does not read the inode of the 
 * entry - you'll have to do that yourself if you want to.
 *
 * This also takes care of the few special cases due to '..' - traversal
 * over a pseudo-root and a mount point.
 */
static struct buffer_head *find_entry(struct m_inode **dir, 
              const char *name, int namelen, struct dir_entry **res_dir)
{
    int entries, i;
    int block;
    struct super_block *sb;
    struct buffer_head *bh;
    struct dir_entry *de;

    entries = (*dir)->i_size / (sizeof (struct dir_entry));
    *res_dir = NULL;
    if (!namelen)
        return NULL;
    /* check for '..', as we might have to do some 'magic' for it */
    if (namelen == 2 && get_fs_byte(name) == '.' &&
        get_fs_byte(name + 1) == '.') {
        /* '..' in a pseudo-root results in a faked '.' 
           (just change namelen) */
        if ((*dir) == current->root)
            namelen = 1;
        else if ((*dir)->i_num == ROOT_INO) {
            /* '..' over a mount-point result in 'dir' being exchange
               for the mount */
            sb = get_super((*dir)->i_dev);
            if (sb->s_imount) {
                iput(*dir);
                (*dir) = sb->s_imount;
                (*dir)->i_count++;
            }
        }
    }
    if (!(block = (*dir)->i_zone[0]))
        return NULL;
    if (!(bh = bread((*dir)->i_dev, block)))
        return NULL;
    i = 0;
    de = (struct dir_entry *)bh->b_data;
    while (i < entries) {
        if ((char *)de >= BLOCK_SIZE + bh->b_data) {
            brelse(bh);
            bh = NULL;
            if (!(block = bmp(*dir, i / DIR_ENTRIES_PER_BLOCK)) ||
                !(bh = bread((*dir)->i_dev, block))) {
                i += DIR_ENTRIES_PER_BLOCK;
                continue;
            }
            de = (struct dir_entry *)bh->b_data;
        }
    }

    return NULL;
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
    struct buffer_head *bh;
    struct dir_entry *de;
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
