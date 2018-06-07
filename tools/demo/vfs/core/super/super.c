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

#include <demo/debug.h>

static struct inode_hash_entry {
    struct inode *inode;
    int updating;
} hash_table[NR_IHASH];

static struct inode *first_inode;

#ifdef CONFIG_DEBUG_INODE_INIT

static void inode_inits(void)
{
    memset(hash_table, 0, sizeof(hash_table));
    first_inode = NULL;
    printk("Init inode hash table.\n");
}

static int debug_inode(void)
{
    inode_inits();
    return 0;
}
subsys_debugcall(debug_inode);
#endif
