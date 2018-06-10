/*
 * Super Block
 *
 * (C) 2018.06.10 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/major.h>
#include <linux/sched.h>
#include <linux/locks.h>

#include <demo/debug.h>

/*
 * The definition of file_systems that used to be here is now in
 * filesystems.c. Nore super.c contains no fs specific code. -- jrs
 */

extern struct file_system_type file_systems[];

extern struct super_block super_blocks[NR_SUPER];

extern void fcntl_init_locks(void);
extern void wait_for_keypress(void);

extern int root_mountflags;
#ifdef CONFIG_DEBUG_MOUNT_ROOTFS

static struct super_block *get_super(dev_t dev)
{
    struct super_block *s;

    if (!dev)
        return NULL;
    s = 0 + super_blocks;
    while (s < NR_SUPER + super_blocks) {
        if (s->s_dev == dev) {
            wait_on_super(s);
            if (s->s_dev == dev)
                return s;
            s = 0 + super_blocks;
        } else
            s++;
    }
    return NULL;
}

static struct file_system_type *get_fs_types(char *name)
{
    int a;

    if (!name)
        return &file_systems[0];
    for (a = 0; file_systems[a].read_super; a++)
        if (!strcmp(name, file_systems[a].name))
            return (&file_systems[a]);
    return NULL;
}

static struct super_block *read_super(dev_t dev, char *name, int flags,
                                  void *data, int silent)
{
    struct super_block *s;
    struct file_system_type *type;

    if (!dev)
        return NULL;
    check_disk_change(dev);
    s = get_super(dev);
    if (s)
        return s;
    if (!(type = get_fs_types(name))) {
        printk("VFS: on device %d/%d: get_fs_type(%s) failed\n",
                MAJOR(dev), MINOR(dev), name);
        return NULL;
    }
    for (s = 0 + super_blocks; ; s++) {
        if (s >= NR_SUPER + super_blocks)
            return NULL;
        if (!s->s_dev)
            break;
    }
    s->s_dev = dev;
    s->s_flags = flags;
    if (!type->read_super(s, data, silent)) {
        s->s_dev = 0;
        return NULL;
    }
    s->s_dev = dev;
    s->s_covered = NULL;
    s->s_rd_only = 0;
    s->s_dirt = 0;
    return s;
}

/*
 * Mount rootfs
 *   The 'file_systems' hold basic filesystem information that contain 
 *   'read_super' to obtain special filesystem. The 'super_blocks' 
 */
static int debug_mount_rootfs(void)
{
    struct file_system_type * fs_type;
    struct super_block *sb;
    struct inode *inode;

    memset(super_blocks, 0, sizeof(super_blocks));
    fcntl_init_locks();
    if (MAJOR(ROOT_DEV) == FLOPPY_MAJOR) {
        printk(KERN_NOTICE "VFS: Insert root floppy and press ENTER\n");
        wait_for_keypress();
    }
    for (fs_type = file_systems; fs_type->read_super; fs_type++) {
        if (!fs_type->requires_dev)
            continue;
        sb = read_super(ROOT_DEV, fs_type->name, root_mountflags, NULL, 1);
        if (sb) {
            inode = sb->s_mounted;
            /* NOTE! it is logically used 4 times, not 1 */
            inode->i_count += 3;
            sb->s_covered = inode;
            sb->s_flags   = root_mountflags;
            current->pwd  = inode;
            current->root = inode;
            printk("VFS: Mounted root (%s filesystem)%s.\n",
                   fs_type->name,
                   (sb->s_flags & MS_RDONLY) ? "readonly" : "");
            return 0;
        }
    }
    panic("VFS: Unable to mount root");
    return 0;
}
rootfs_debugcall(debug_mount_rootfs);
#endif
