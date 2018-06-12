/*
 * Inode: Describe a filesystem object file or dirent.
 *
 * (C) 2018.06.07 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/unistd.h>
#include <linux/fcntl.h>
#include <linux/errno.h>
#include <linux/sched.h>

#include <demo/debug.h>

static struct inode_hash_entry {
    struct inode *inode;
    int updating;
} hash_table[NR_IHASH];

static int nr_inodes = 0, nr_free_inodes = 0;

static inline int const hashfn(dev_t dev, unsigned int i)
{
    return (dev ^ i) % NR_IHASH;
}

static inline struct inode_hash_entry * const hash(dev_t dev, int i)
{
    return hash_table + hashfn(dev, i);
}

#ifdef CONFIG_DEBUG_INODE_INIT
static struct inode *first_inode;

static void inode_inits(void)
{
    memset(hash_table, 0, sizeof(hash_table));
    first_inode = NULL;
}

static int debug_inode_init(void)
{
    inode_inits();
    return 0;
}
subsys_debugcall(debug_inode_init);
#endif

#ifdef CONFIG_DEBUG_INODE_IGET

static void grow_inode(void)
{
    panic("Grow_inode\n");
}

static struct inode *get_empty_inodes(void)
{
    struct inode *inode, *best;
    int i;

    if (nr_inodes < NR_INODE && nr_free_inodes < (nr_inodes >> 2))
        grow_inode();
repeat:
    inode
    return NULL;
}

static int debug_iget(int nr)
{
    struct super_block *sb = current->pwd->i_sb;
    struct inode_hash_entry *h;
    struct inode *inode;
    struct inode *empty = NULL;

    if (!sb)
        panic("VFS: iget with sb==NULL");
    h = hash(sb->s_dev, nr);
repeat:
    for (inode = h->inode; inode; inode = inode->i_hash_next)
        if (inode->i_dev == sb->s_dev && inode->i_ino == nr)
            goto found_it;

    if (!empty) {
        h->updating++;
        empty = get_empty_inodes();
    }
    return 0;
found_it:
    printk("Found it\n");
    return 0;
}
#endif

/* The entry for systemcall */
asmlinkage int sys_vfs_inode(const char *filename)
{
    char *tmp;
    int error;
    struct inode *inode;

    error = getname(filename, &tmp);
    if (error)
        return error;
    error = open_namei(filename, O_RDWR, 0, &inode, NULL);
    if (error)
        return error;
    inode->i_count++;
#ifdef CONFIG_DEBUG_INODE_IGET
    debug_iget(inode->i_ino);
#endif
    iput(inode);
    putname(tmp);
    return error;
}

static int debug_inode(void)
{
    vfs_inode("/etc/rc");
    return 0;
}
user1_debugcall(debug_inode);
