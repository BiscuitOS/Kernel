/*
 * System Call: unlink
 *
 * (C) 2018.04 BiscuitOS <buddy.zhang@aliyun.com>
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

#include <test/debug.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define MAY_EXEC    1
#define MAY_WRITE   2
#define MAY_READ    4

/*
 * permission()
 *
 * is used to check for read/write/execute permission on a file.
 * I don't know if we should look at just the euid or both euid and 
 * uid, but that should be easily changed.
 */
static int permission(struct m_inode *inode, int mask)
{
    int mode = inode->i_mode;

    /* special case: not even root can read/write a deleted file */
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
 * ok, we cannot use strncmp, as the name is not in our data space
 * Thus we'll have to use match. No big problem. Match also makes
 * some sanity tests.
 *
 * NOTE!! unlike strncmp, match returns 1 for success, 0 for failure
 */
static int match(int len, const char *name, struct dir_entry *de)
{
    register int same;

    if (!de || !de->inode || len > NAME_LEN)
        return 0;
    if (len < NAME_LEN && de->name[len])
        return 0;
    __asm__ ("cld\n\t"
             "fs; repe ; cmpsb\n\t"
             "setz %%al"
             : "=a" (same)
             : "0" (0), "S" ((long)name), 
               "D" ((long)de->name), "c" (len));
    return same;
}

/*
 * find_entry()
 *
 * finds an entry in the specified directory with the wanted name. It
 * returns the cache buffer in which the netry as found, and the entry
 * itself (as a parameter - res_dir). It does NOT read the inode of the
 * entry - you'll have to do that yourself if you want to.
 *
 * This also takes care of the few special cases due to '..' - travesal
 * over a pseudo-root and a mount point.
 */
static struct buffer_head *find_entry(struct m_inode **dir,
       const char *name, int namelen, struct dir_entry **res_dir)
{
    int entries;
    int block, i;
    struct buffer_head *bh;
    struct dir_entry *de;
    struct super_block *sb;

#ifdef NO_TRUNCATE
    if (namelen > NAME_LEN)
        return NULL;
#else
    if (namelen > NAME_LEN)
        namelen = NAME_LEN;
#endif
    entries = (*dir)->i_size / (sizeof(struct dir_entry));
    *res_dir = NULL;
    if (!namelen)
        return NULL;
    /* check for '..', as we might have to do some "magic" for it */
    if (namelen == 2 && get_fs_byte(name) == '.' &&
       get_fs_byte(name + 1) == '.') {
        /* '..' in a pseudo-root results in a faked '.' (just change namelen) */
        if ((*dir) == current->root)
            namelen = 1;
        else if ((*dir)->i_num == ROOT_INO) {
            /* '..' over a mount-point result in 'dir' being exchanged
             * for the mount */
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
            /* I don't care this case! */;
        }
        if (match(namelen, name, de)) {
            printk("FNAME %s\n", de->name);
            *res_dir = de;
            return bh;
        }
        de++;
        i++;
    }
    brelse(bh);
    return NULL;
}

/*
 * get_dir()
 *
 * getdir traverses the pathname untile it hits the topmost directory.
 * It returns NULL on failure.
 */
static struct m_inode *get_dir(const char *pathname)
{
    char c;
    const char *thisname;
    struct m_inode *inode;
    struct buffer_head *bh;
    int namelen, inr, idev;
    struct dir_entry *de;

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
                        namelen++)
            /* nothing */;
        if (!c)
            return inode;
        if (!(bh = find_entry(&inode, thisname, namelen, &de))) {
            iput(inode);
            return NULL;
        }
        inr = de->inode;
        idev = inode->i_dev;
        brelse(bh);
        iput(inode);
        if (!(inode = iget(idev, inr)))
            return NULL;
    }
}

/*
 * dir_namei()
 *
 * dir_namei() returns the inode of directory of the 
 * specified name, and the name within that directory.
 */
static struct m_inode *dir_namei(const char *pathname,
       int *namelen, const char **name)
{
    char c;
    const char *basename;
    struct m_inode *dir;

    if (!(dir = get_dir(pathname)))
        return NULL;
    basename = pathname;
    while ((c = get_fs_byte(pathname++)))
        if (c == '/')
            basename = pathname;
    *namelen = pathname - basename - 1;
    *name = basename;
    return dir;
}

int sys_d_unlink(const char *name)
{
    const char *basename;
    int namelen;
    struct m_inode *dir, *inode;
    struct buffer_head *bh;
    struct dir_entry *de;

    if (!(dir = dir_namei(name, &namelen, &basename)))
        return -ENOENT;
    if (!namelen) {
        iput(dir);
        return -ENOENT;
    }
    if (!permission(dir, MAY_WRITE)) {
        iput(dir);
        return -EPERM;
    }
    bh = find_entry(&dir, basename, namelen, &de);
    if (!bh) {
        iput(dir);
        return -ENOENT;
    }
    if (!(inode = iget(dir->i_dev, de->inode))) {
        iput(dir);
        brelse(bh);
        return -ENOENT;
    }
    if ((dir->i_mode & S_ISVTX) && !suser() &&
         current->euid != inode->i_uid &&
         current->euid != dir->i_uid) {
        iput(dir);
        iput(inode);
        brelse(bh);
        return -EPERM;
    }
    if (S_ISDIR(inode->i_mode)) {
        iput(inode);
        iput(dir);
        brelse(bh);
        return -EPERM;
    }
    if (!inode->i_nlinks) {
        printk("Deleting nonexistent file (%04x:%d), %d\n",
             inode->i_dev, inode->i_num, inode->i_nlinks);
        inode->i_nlinks = 1;
    }
    de->inode = 0;
    bh->b_dirt = 1;
    brelse(bh);
    inode->i_nlinks--;
    inode->i_dirt = 1;
    inode->i_ctime = CURRENT_TIME;
    iput(dir);
    return 0;
}

/* Invoke by system call: int $0x80 */
int debug_syscall_unlink0(void)
{
    /* Create a software link */
    link("/etc/rc", "/etc/rc2");

    /* Remove special software link */
    d_unlink("/etc/rc2");
    return 0;
}
