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
#include <linux/stat.h>
#include <linux/fcntl.h>
#ifdef CONFIG_MINIX_FS
#include <linux/minix_fs.h>
#endif

#include <asm/segment.h>

#include <demo/debug.h>

#define ACC_MODE(x) ("\000\004\002\006"[(x)&O_ACCMODE])

extern int lookup(struct inode * dir,const char * name, int len,
        struct inode ** result);
extern int follow_link(struct inode * dir, struct inode * inode,
        int flag, int mode, struct inode ** res_inode);

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

#ifdef CONFIG_DEBUG_INODE_LOOKUP_ROOTFS

#ifdef CONFIG_MINIX_FS

/*
 * Comment out this line if you want names > info->s_namelen chars to be
 * truncated. Else they will be disallowed (ENAMETOOLONG).
 */
/* #define NO_TRUNCATE */

static inline int namecompare(int len, int maxlen, const char *name,
            const char *buffer)
{
    if (len >= maxlen || !buffer[len]) {
        unsigned char same;

        __asm__ ("repe ; cmpsb ; setz %0"
                 : "=q" (same)
                 : "S" ((long) name), "D" ((long) buffer), "c" (len));
        return same;
    }
    return 0;
}

/*
 * OK, we cannot use strncmp, as the name is not in our data space.
 * Thus we'll have to use minix_match. No big problem. Match also makes
 * some sanity tests.
 *
 * NOTE! unlike strncmp, minix_match returns 1 for success, 0 for failure
 */
static int minix_match(int len, const char *name, 
        struct buffer_head *bh, unsigned long *offset,
        struct minix_sb_info *info)
{
    struct minix_dir_entry *de;

    de = (struct minix_dir_entry *)(bh->b_data + *offset);
    *offset += info->s_dirsize;
    if (!de->inode || len > info->s_namelen)
        return 0;
    /* "" means "." ---> so paths like "/usr/lib//libc.a" work */
    if (!len && (de->name[0] == '.') && (de->name[1] == '\0'))
        return 0;
    return namecompare(len, info->s_namelen, name, de->name);
}

/*
 * minix_find_entry()
 *   finds an entry in the specified directory with the wanted name. It
 *   returns the cache buffer in which the entry was found, and the entry
 *   itself (as a parameter - res_dir). It does NOT read the inode of the
 *   entry - you'll have to do that yourself if you want to.
 */
static struct buffer_head *minix_find_entry(struct inode *dir,
     const char *name, int namelen, struct minix_dir_entry **res_dir)
{
    struct minix_sb_info *info;
    struct buffer_head *bh;
    unsigned long block, offset;

    *res_dir = NULL;
    if (!dir || !dir->i_sb)
        return NULL;
    info = &dir->i_sb->u.minix_sb;
    if (namelen > info->s_namelen) {
#ifdef NO_TRUNCATE
        return NULL;
#else
        namelen = info->s_namelen;
#endif
    }
    bh = NULL;
    block = offset = 0;
    while (block * BLOCK_SIZE + offset < dir->i_size) {
        if (!bh) {
            bh = minix_bread(dir, block, 0);
            if (!bh) {
                block++;
                continue;
            }
        }
        *res_dir = (struct minix_dir_entry *)(bh->b_data + offset);
        if (minix_match(namelen, name, bh, &offset, info))
            return bh;
        if (offset < bh->b_size)
            continue;
        brelse(bh);
        bh = NULL;
        offset = 0;
        block++;
    }
    brelse(bh);
    *res_dir = NULL;
    return NULL;
}

static int minix_lookups(struct inode *dir, const char *name,
                            int len, struct inode **result)
{
    int ino;
    struct minix_dir_entry *de;
    struct buffer_head *bh;

    *result = NULL;
    if (!dir)
        return -ENOENT;
    if (!S_ISDIR(dir->i_mode)) {
        iput(dir);
        return -ENOENT;
    }
    if (!(bh = minix_find_entry(dir, name, len, &de))) {
        iput(dir);
        return -ENOENT;
    }
    ino = de->inode;
    brelse(bh);
    if (!(*result = iget(dir->i_sb, ino))) {
        iput(dir);
        return -EACCES;
    }
    iput(dir);
    return 0;
}
#endif

static int special_lookup(struct inode *dir, const char *name, 
                             int len, struct inode **result)
{
#ifdef CONFIG_MINIX_FS
    return minix_lookups(dir, name, len, result);
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
#ifdef CONFIG_DEBUG_INODE_LOOKUP_ROOTFS
    return special_lookup(dir, name, len, result);
#else
    return dir->i_op->lookup(dir, name, len, result);
#endif
}
#endif

#ifdef CONFIG_DEBUG_INODE_SYMLINK_ROOTFS
static int follow_link_rootfs(struct inode *dir, struct inode *inode,
             int flag, int mode, struct inode **res_inode)
{
    /* no implement */
    return 0;
}
#endif

#ifdef CONFIG_DEBUG_INODE_SYMLINK
static int follow_links(struct inode *dir, struct inode *inode,
           int flag, int mode, struct inode **res_inode)
{
    if (!dir || !inode) {
        iput(dir);
        iput(inode);
        *res_inode = NULL;
        return -ENOENT;
    }
    if (!inode->i_op || !inode->i_op->follow_link) {
        iput(dir);
        *res_inode = inode;
        return 0;
    }
#ifdef CONFIG_DEBUG_INODE_SYMLINK_ROOTFS
    return follow_link_rootfs(dir, inode, flag, mode, res_inode);
#else
    return inode->i_op->follow_link(dir, inode, flag, mode, res_inode);
#endif
}
#endif

/*
 * dir_namei()
 *   dir_namei() returns the inode of the direntory of the 
 *   specified name, and the name within that directory.
 */
static int dir_namei(const char *pathname, int *namelen, const char **name,
              struct inode *base, struct inode **res_inode)
{
    char c;
    const char *thisname;
    int len, error;
    struct inode *inode;

    *res_inode = NULL;
    /* Verify current path.
     *   if 'base' is empty, obtain current path from current task_struct 
     *   and incream usage of current inode. 
     */
    if (!base) {
        base = current->pwd;
        base->i_count++;
    }
    /*
     * If first pathname[0] is '/', we directly point root dirent.
     * Note! please decream usage of 'base'.
     */
    if ((c = *pathname) == '/') {
        iput(base);
        base = current->root;
        pathname++;
        base->i_count++;
    }
    while (1) {
        thisname = pathname;
        /* Continue to lookup '/' on 'pathname', if found it and reut */
        for (len = 0; (c = *(pathname++)) && (c != '/'); len++)
            /* nothing */;
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
#ifdef CONFIG_DEBUG_INODE_SYMLINK
        error = follow_links(base, inode, 0, 0, &base);
#else
        error = follow_link(base, inode, 0, 0, &base);
#endif
        if (error)
            return error;
    }
    if (!base->i_op || !base->i_op->lookup) {
        iput(base);
        return -ENOTDIR;
    }
    *name = thisname;
    *namelen = len;
    *res_inode = base;
    return 0;
}

#ifdef CONFIG_DEBUG_OPEN_NAMEI
/*
 * do_open_namei()
 *   namei for open - this is in fact almost the whole open-routine.
 *
 *   Note that tht low bits of 'flag' aren't the same as in the open
 *   system call - they are:
 *                      00 - no permissions needed
 *                      01 - read permissions needed
 *                      10 - write permission needed
 *                      11 - read/write permission needed
 *   Which is a lot more logical, and also allows the "no perm" needed
 *   for symlinks (where the permissions are checked later).
 */
static int do_open_namei(const char *pathname, int flag, int mode,
           struct inode **res_inode, struct inode *base)
{
    const char *basename;
    int namelen, error;
    struct inode *dir, *inode;
    
    /* S_IALLUGO: (S_ISUID | S_ISGID | S_ISVTX | S_IRWXUGO) */
    mode &= S_IALLUGO & ~current->umask;
    /* S_IFREG: File is a regular file */
    mode |= S_IFREG;
    error = dir_namei(pathname, &namelen, &basename, base, &dir);
    if (error)
        return error;
    if (!namelen) {   /* special case: '/usr/' etc */
        if (flag & 2) {
            iput(dir);
            return -EISDIR;
        }
        /* thanks to Paul Pluzhnikow for noticing this was missing.. */
        if (!permission(dir, ACC_MODE(flag))) {
            iput(dir);
            return -EACCES;
        }
        *res_inode = dir;
        return 0;
    }
    dir->i_count++;  /* lookup eats the dir */
    if (flag & O_CREAT) {
        down(&dir->i_sem);
#ifdef CONFIG_DEBUG_INODE_LOOKUP
        error = lookups(dir, basename, namelen, &inode);
#else
        error = lookup(dir, basename, namelen, &inode);
#endif
        if (!error) {
            if (flag & O_EXCL) {
                iput(inode);
                error = -EEXIST;
            }
        } else if (!permission(dir, MAY_WRITE | MAY_EXEC))
            error = -EACCES;
        else if (!dir->i_op || !dir->i_op->create)
            error = -EACCES;
        else if (IS_RDONLY(dir))
            error = -EROFS;
        else {
            dir->i_count++;   /* create eats the dir */
            error = dir->i_op->create(dir, basename, namelen, mode,
                              res_inode);
            up(&dir->i_sem);
            iput(dir);
            return error;
        }
            up(&dir->i_sem);
    } else 
#ifdef CONFIG_DEBUG_INODE_LOOKUP
        error = lookups(dir, basename, namelen, &inode);
#else
        error = lookup(dir, basename, namelen, &inode);
#endif
    if (error) {
        iput(dir);
        return error;
    }
#ifdef CONFIG_DEBUG_INODE_SYMLINK
    error = follow_links(dir, inode, flag, mode, &inode);
#else
    error = follow_link(dir, inode, flag, mode, &inode);
#endif
    if (error) {
        iput(dir);
        return error;
    }
    if (S_ISDIR(inode->i_mode) && (flag & 2)) {
        iput(inode);
        return -EISDIR;
    }
    if (!permission(inode, ACC_MODE(flag))) {
        iput(inode);
        return -EACCES;
    }
    if (S_ISBLK(inode->i_mode) || S_ISCHR(inode->i_mode)) {
        if (IS_NODEV(inode)) {
            iput(inode);
            return -EACCES;
        }
    } else {
        if (IS_RDONLY(inode) && (flag & 2)) {
            iput(inode);
            return -EROFS;
        }
    }

    return 0;
}
#endif

/*
 * do_namei
 *  common namei entry. Note that while the flag value (low two bits)
 *  for do_namei means:
 *          00 - read-only
 *          01 - write-only
 *          10 - read-write
 *          11 - special
 *  it is changed into
 *          00 - no permissions needed
 *          01 - read-permission
 *          10 - write-permission
 *          11 - read-write
 */
static int do_namei(const char *filename, int flags, int mode)
{
    int flag, modes, error;
    struct inode *inode;

    flag = flags;
    modes = (flag + 1) & O_ACCMODE;
    if (modes)
        flag++;
    if (flag & (O_TRUNC | O_CREAT))
        flag |= 2;
#ifdef CONFIG_DEBUG_OPEN_NAMEI
    error = do_open_namei(filename, flag, mode, &inode, NULL);
#else
    error = open_namei(filename, flag, mode, &inode, NULL);
#endif
    return 0;
}

/* system call entry on kernel */
asmlinkage int sys_vfs_namei(const char *name, int flag, int mode)
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
        return error;
    }
    error = do_namei(tmp, flag, mode);
    return error;
}

static int debug_namei(void)
{
    printf("Cover name to inode\n");
    vfs_namei("/etc/BiscuitOS0.rc", O_CREAT, 0);
    return 0;
}
user1_debugcall_sync(debug_namei);
