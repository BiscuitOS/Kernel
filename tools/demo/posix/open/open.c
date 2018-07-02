/*
 * POSIX system call: open
 *
 * (C) 2018.06.11 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/unistd.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/stat.h>
#include <linux/locks.h>

#ifdef CONFIG_MINIX_FS
#include <linux/minix_fs.h>
#endif

#include <asm/bitops.h>

#include <demo/debug.h>

#define find_first_zero(addr) ({ \
    int __res; \
    __asm__("cld\n" \
            "1:\tlodsl\n\t" \
            "notl %%eax\n\t" \
            "bsfl %%eax, %%edx\n\t" \
            "jne 2f\n\t" \
            "addl $32, %%ecx\n\t" \
            "cmpl $8192,%%ecx\n\t" \
            "jl 1b\n\t" \
            "xorl %%edx,%%edx\n" \
            "2:\taddl %%edx,%%ecx" \
            :"=c" (__res):"0" (0), "S" (addr)); \
    __res;})

static inline _syscall1(int, close, int, fd);
static inline _syscall3(int, read, unsigned int, fd, char *, buf,
                         unsigned int, count);

#ifdef CONFIG_DEBUG_OPEN_ORIG
static inline _syscall3(int, open, const char *, file, int, flag, int, mode);
#endif

#ifdef CONFIG_DEBUG_FILE_DESCRIBE
static void parse_file_descriptor(struct file *f)
{
    /* Verify access mode */
    switch (f->f_mode & O_ACCMODE) {
    case O_RDONLY:
        /* Open for reading only */
        printk("Open for reading only.\n");
        break;
    case O_WRONLY:
        /* Oren for writing only */
        printk("Open for writing only.\n");
        break;
    case O_RDWR:
        /* Open for reading and writing. The result is undefined if this
         * flag is applied to a FIFO. */
        printk("Open for reading and writing.\n");
        break;
    }
    /* Verify whether create new file. If the file exists, this flag has 
     * no effect except as noted under O_EXCL below. Otherwise, the file 
     * shall be created; the user ID of the file shall be set to the 
     * effective user ID of the process; the group ID of the file shall 
     * be set to the group ID of the file's parent directory or to the
     * effective group ID of the process; and the access permission bits 
     * (see <sys/stat.h>) of the file mode shall be set to the value of 
     * the third argument taken as type mode_t modified as follows: a 
     * bitwise AND is performed on the file-mode bits and the corresponding
     * bits in the complement of the process' file mode creation mask. 
     * Thus, all bits in the file mode whose corresponding bit in the 
     * file mode creation mask is set are cleared. When bits other than 
     * the file permission bits are set, the effect is unspecified. 
     * The third argument does not affect whether the file is open for 
     * reading, writing, or for both. Implementations shall provide a way
     * to initialize the file's group ID to the group ID of the parent
     * directory. Implementations may, but need not, provide an 
     * implementation-defined way to initialize the file's group ID to 
     * the effective group ID of the calling process.
     */
    if (f->f_mode & O_CREAT)
        printk("File doesn't exist and create new file.\n");
    /* Verify file whether executable, If O_CREAT and O_EXCL are set, 
     * open() shall fail if the file exists. The check for the existence 
     * of the file and the creation of the file if it does not exist shall 
     * be atomic with respect to other threads executing open() naming the
     * same filename in the same directory with O_EXCL and O_CREAT set. 
     * If O_EXCL and O_CREAT are set, and path names a symbolic link, open()
     * shall fail and set errno to [EEXIST], regardless of the contents 
     * of the symbolic link. If O_EXCL is set and O_CREAT is not set,
     * the result is undefined. 
     */
    if (f->f_mode & O_EXCL)
        printk("File can be executed\n");
    /* If set and path identifies a terminal device, open() shall not 
     * cause the terminal device to become the controlling terminal for
     * the process.
     */
    if (f->f_mode & O_NOCTTY)
        printk("TTY is not a control terminal.\n");
    /* If the file exists and is a regular file, and the file is successfully
     * opened O_RDWR or O_WRONLY, its length shall be truncated to 0, 
     * and the mode and owner shall be unchanged. It shall have no effect 
     * on FIFO special files or terminal device files. Its effect on other
     * file types is implementation-defined. The result of using O_TRUNC 
     * with O_RDONLY is undefined.
     * If O_CREAT is set and the file did not previously exist, upon 
     * successful completion, open() shall mark for update the st_atime,
     * st_ctime, and st_mtime fields of the file and the st_ctime and 
     * st_mtime fields of the parent directory.
     */
    if (f->f_mode & O_TRUNC)
        printk("File Truncate.\n");
    /* If set, the file offset shall be set to the end of the file prior 
     * to each write.
     */
    if (f->f_mode & O_APPEND)
        printk("Append to file tail.\n");
    /* See README.md */
    if (f->f_mode & O_NONBLOCK)
        printk("File Open with NONBLOCK\n");
    /* Write I/O operations on the file descriptor shall complete as 
     * defined by synchronized I/O data integrity completion.
     */
    if (f->f_mode & O_SYNC)
        printk("Sync file.\n");
}
#endif

#ifdef CONFIG_DEBUG_OPEN_NAMEI

#define ACC_MODE(x) ("\000\004\002\006"[(x)&O_ACCMODE])

#ifdef CONFIG_MINIX_FS
extern int follow_link(struct inode * dir, struct inode * inode,
        int flag, int mode, struct inode ** res_inode);
extern struct buffer_head * minix_getblk(struct inode * inode, 
                                  int block, int create);

/*
 * bread_minix()
 *  Read a BLOCK from minixfs.
 */
static struct buffer_head *bread_minix(struct inode *inode, int block,
                        int create)
{
    struct buffer_head *bh;

    bh = minix_getblk(inode, block, create);
    if (!bh || bh->b_uptodate)
        return bh;
    ll_rw_block(READ, 1, &bh);
    wait_on_buffer(bh);
    if (bh->b_uptodate)
        return bh;
    brelse(bh);
    return NULL;
}

static inline int namecompare(int len, int maxlen, 
            const char *name, const char *buffer)
{
    /*
     * If name matchs buffer, and 'buffer[len]' must be zero!
     */
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
 * ok, we cannot use strncmp, as the name is not in our data space.
 * Thus we'll have to use 'minix_match'. No big problem. Match also makes
 * some sanity tests.
 *
 * NOTE! unlike strncmp, minix_match returns 1 for success, 0 for failure.
 */
static int minix_match(int len, const char *name,
          struct buffer_head *bh, unsigned long *offset,
          struct minix_sb_info *info)
{
    /*
     * The 'minix_dir_entry' is used to manage a entry for special inode.
     * It's defined in 'include/linux/minix_fs.h', as follow:
     * 
     *   struct minix_dir_entry {
     *       unsigned short inode;
     *       char name[0];
     *   }
     *
     * The 'inode' points a special inode structure, and 'name' is a empty
     * array but it is start address of name. 'name' is used to point a
     * data zera that cotinuely connect after 'name'. 
     *
     *
     * | <------------- s_namelen -------------> |
     * 0-------2---------------------------------+
     * |       |                                 |
     * | inode | name (unknow length)            |
     * |       |                                 |
     * +-------+---------------------------------+
     *         A
     *         |
     *         |
     *         o---- start address of name string
     * 
     * sizeof(struct minix_dir_entry) = 2
     */
    struct minix_dir_entry *de;

    de = (struct minix_dir_entry *)(bh->b_data + *offset);
    /* Points next minix_dir_entry */
    *offset += info->s_dirsize;
    if (!de->inode || len > info->s_namelen)
        return 0;
    /* "" means "." ---> so paths like "/usr/lib//libc.a" work */
    if (!len && (de->name[0] == '.') && (de->name[1] == '\0'))
        return 1;
    return namecompare(len, info->s_namelen, name, de->name);
}

/*
 * minix_find_entry()
 *  finds an entry in the specified directory with the wanted name. It
 *  returns the cache buffer in which the entry was found, and the entry
 *  itself (as a parameter - res_dir). It does NOT read the inode of the
 *  entry - you'll have to do that yourself if you want to.
 */
static struct buffer_head *find_entry_minix(struct inode *dir,
      const char *name, int namelen, struct minix_dir_entry **res_dir)
{
    unsigned long block, offset;
    struct buffer_head *bh;
    struct minix_sb_info *info;

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
            bh = bread_minix(dir, block, 0);
            if (!bh) {
                block++;
                continue;
            }
        }
        /*
         * The 'minix_bread' read special block from MINIX Filesystem.
         * A special 'buffer_head' manage a buffer and 'b_data' points to
         * data zera of buffer. The data zera of direntry inode contains
         * a lot of 'minix_dir_entry' that describes the member information.
         *
         * 
         * +-------------------------------------+
         * |                                     |
         * |         struct buffer_head          |
         * |                                     |
         * +-------------------------------------+ 
         *                   |
         *                   |
         *                   |
         *     b_data        |
         * o-----------------o
         * |
         * |
         * |
         * V                 | <- s_dirsize -> |
         * 0-----------------+-----------------+----+-----------------+
         * |                 |                 |    |                 |
         * | minix_dir_entry | minix_dir_entry | .. | minix_dir_entry |
         * |                 |                 |    |                 |
         * +-----------------+-----------------+----+-----------------+
         * A <-- offset0 --> | <-- offset1 --> |    | <-- offsetn --> |
         * |
         * |
         * o-----res_dir
         *
         */
        *res_dir = (struct minix_dir_entry *)(bh->b_data + offset);
        if (minix_match(namelen, name, bh, &offset, info))
            return bh;
        /* Continue search in current block */
        if (offset < bh->b_size)
            continue;
        /* Search on next block */
        brelse(bh);
        bh = NULL;
        offset = 0;
        block++;
    }
    brelse(bh);
    *res_dir = NULL;
    return NULL;
}

static int lookup_minix(struct inode *dir, const char *name, 
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
    /* find enty on dirent inode */
    if (!(bh = find_entry_minix(dir, name, len, &de))) {
        iput(dir);
        return -ENOENT;
    }
    ino = de->inode;
    brelse(bh);
    /* Find special inode */
    if (!(*result = iget(dir->i_sb, ino))) {
        iput(dir);
        return -EACCES;
    }
    iput(dir);
    return 0;
}
#endif

static __unused int lookup_rootfs(struct inode *inode, const char *name,
            int len, struct inode **result)
{
#ifdef CONFIG_MINIX_FS
    lookup_minix(inode, name, len, result);
#endif
    return 0;
}

/*
 * permission()
 *  is used to check for read/write/execute permissions on a file.
 *  I don't know if we should look at just the euid or both euid and
 *  uid, but that should be easily changed.
 */
static int permissions(struct inode *inode, int mask)
{
    int mode = inode->i_mode;

    if (inode->i_op && inode->i_op->permission)
        return inode->i_op->permission(inode, mask);
    else if (current->euid == inode->i_uid)
        mode >>= 6;
    else if (in_group_p(inode->i_gid))
        mode >>= 3;
    if (((mode & mask & 0007) == mask) || suser())
        return 1;
    return 0;
}

/*
 * lookup() looks up one part of a pathname, using the fs-dependent
 * routines (currently minix_lookup) for it. It also checks for
 * fathers (pseudo-roots, mount-points)
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
    perm = permissions(dir, MAY_EXEC);
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
#ifdef CONFIG_DEBUG_LOOKUP_ROOTFS
    return lookup_rootfs(dir, name, len, result);
#else
    return dir->i_op->lookup(dir, name, len, result);
#endif
}

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
    return inode->i_op->follow_link(dir, inode, flag, mode, res_inode);
}

#ifdef CONFIG_MINIX_FS

/*
 *     minix_add_entry()
 *
 * adds a file entry to the specified directory, returning a possible
 * error value if it fails.
 *
 * NOTE!! The inode part of 'de' is left at 0 - which means you
 * may not sleep between calling this and putting something into
 * the entry, as someone else might have used it while you slept.
 */
static int minix_add_entry(struct inode *dir, const char *name,
      int namelen, struct buffer_head **res_buf,
      struct minix_dir_entry **res_dir)
{
    int i;
    unsigned long block, offset;
    struct buffer_head *bh;
    struct minix_dir_entry *de;
    struct minix_sb_info *info;

    *res_buf = NULL;
    *res_dir = NULL;
    if (!dir || !dir->i_sb)
        return -ENOENT;
    info = &dir->i_sb->u.minix_sb;
    if (namelen > info->s_namelen) {
#ifdef NO_TRUNCATE
        return -ENAMETOOLONG;
#else
        namelen = info->s_namelen;
#endif
    }
    if (!namelen)
        return -ENOENT;
    bh = NULL;
    block = offset = 0;
    while (1) {
        if (!bh) {
            bh = bread_minix(dir, block, 1);
            if (!bh)
                return -ENOSPC;
        }
        de = (struct minix_dir_entry *)(bh->b_data + offset);
        offset += info->s_dirsize;
        if (block * bh->b_size + offset > dir->i_size) {
            de->inode = 0;
            dir->i_size = block * bh->b_size + offset;
            dir->i_dirt = 1;
        }
        if (de->inode) {
            if (namecompare(namelen, info->s_namelen, name, de->name)) {
                brelse(bh);
                return -EEXIST;
            }
        } else {
            dir->i_mtime = dir->i_ctime = CURRENT_TIME;
            for (i = 0; i < info->s_namelen; i++)
                de->name[i] = (i < namelen) ? name[i] : 0;
            bh->b_dirt = 1;
            *res_dir = de;
            break;
        }
        if (offset < bh->b_size)
            continue;
        brelse(bh);
        bh = NULL;
        offset = 0;
        block++;
    }
    *res_buf = bh;
    return 0;
}

static struct inode *new_inode_minix(const struct inode *dir)
{
    struct super_block *sb;
    struct inode *inode;
    struct buffer_head *bh;
    int i, j;

    if (!dir || !(inode = get_empty_inode()))
        return NULL;
    sb = dir->i_sb;
    inode->i_sb = sb;
    inode->i_flags = inode->i_sb->s_flags;
    j = 8192;
    for (i = 0; i < 8; i++)
        if ((bh = inode->i_sb->u.minix_sb.s_imap[i]) != NULL)
            if ((j = find_first_zero(bh->b_data)) < 8192)
                break;
    if (!bh || j >= 8192) {
        iput(inode);
        return NULL;
    }
    if (set_bit(j, bh->b_data)) {  /* shouldn't happen */
        printk("new_inode: bit already set");
        iput(inode);
        return NULL;
    }
    bh->b_dirt = 1;
    j += i * 8192;
    if (!j || j >= inode->i_sb->u.minix_sb.s_ninodes) {
        iput(inode);
        return NULL;
    }
    inode->i_count = 1;
    inode->i_nlink = 1;
    inode->i_dev = sb->s_dev;
    inode->i_uid = current->euid;
    inode->i_gid = (dir->i_mode & S_ISGID) ? dir->i_gid : current->egid;
    inode->i_dirt = 1;
    inode->i_ino  = j;
    inode->i_mtime = inode->i_atime = inode->i_ctime = CURRENT_TIME;
    inode->i_op = NULL;
    inode->i_blocks = inode->i_blksize = 0;
    insert_inode_hash(inode);
    return inode;
}

static int create_minix(struct inode *dir, const char *name, int len,
                             int mode, struct inode **result)
{
    int error;
    struct inode *inode;
    struct buffer_head *bh;
    struct minix_dir_entry *de;

    *result = NULL;
    if (!dir)
        return -ENOENT;
    inode = new_inode_minix(dir);
    if (!inode) {
        iput(dir);
        return -ENOSPC;
    }
    inode->i_op = &minix_file_inode_operations;
    inode->i_mode = mode;
    inode->i_dirt = 1;
    error = minix_add_entry(dir, name, len, &bh, &de);
    if (error) {
        inode->i_nlink--;
        inode->i_dirt = 1;
        iput(inode);
        iput(dir);
        return error;
    }
    de->inode = inode->i_ino;
    bh->b_dirt = 1;
    brelse(bh);
    iput(dir);
    *result = inode;
    return 0;
}
#endif

#ifdef CONFIG_DEBUG_CREAT_ROOTFS
static int create_rootfs(struct inode *dir, const char *name, int len,
                            int mode, struct inode **result)
{
#ifdef CONFIG_MINIX_FS
    create_minix(dir, name, len, mode, result);
#endif
    return 0;
}
#endif

/*
 * dir_namei()
 *   dir_namei() returns the inode for the directory of the 
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
    if (!base) {
        base = current->pwd;
        base->i_count++;
    }
    if ((c = *pathname) == '/') {
        iput(base);
        base = current->root;
        pathname++;
        base->i_count++;
    }
    while (1) {
        thisname = pathname;
        for (len = 0; (c = *(pathname++)) && (c != '/'); len++)
            /* nothing */;
        if (!c)
            break;
        base->i_count++;
        /* Search special inode by name */
        error = lookups(base, thisname, len, &inode);
        if (error) {
            iput(base);
            return error;
        }
        error = follow_links(base, inode, 0, 0, &base);
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

/*
 * parse_open_namei()
 *
 *  namei for open - this is in fact almost the whole open-routinue.
 *
 *  Note that the low bits of "flag" aren't  the same as the open system
 *  call - they are 00 - no permissions needed
 *                  01 - read permission needed
 *                  10 - write permission needed
 *                  11 - read/write permission needed
 *  Which is a lot more logical, and also allows the "no perm" needed
 *  for syslinks (where the permissions are checked later).
 */
static int parse_open_namei(const char *pathname, int flag, int mode,
                struct inode **res_inode, struct inode *base)
{
    const char *basename;
    int namelen, error;
    struct inode *dir, *inode;
    struct task_struct **p;

    mode &= S_IALLUGO & ~current->umask;
    mode |= S_IFREG;
    error = dir_namei(pathname, &namelen, &basename, base, &dir);
    if (error)
        return error;
    if (!namelen) { /* special case: '/usr/' etc */
        if (flag & 2) {
            iput(dir);
            return -EISDIR;
        }
        /* thanks to Paul Pluzhnikow for noticing this was missing.. */
        if (!permissions(dir, ACC_MODE(flag))) {
            iput(dir);
            return -EACCES;
        }
        *res_inode = dir;
        return 0;
    }
    dir->i_count++; /* lookup eats the dir */
    if (flag & O_CREAT) {
        down(&dir->i_sem);
        error = lookups(dir, basename, namelen, &inode);
        if (!error) {
            if (flag & O_EXCL) {
                iput(inode);
                error = -EEXIST;
            }
        } else if (!permissions(dir, MAY_WRITE | MAY_EXEC))
            error = -EACCES;
        else if (!dir->i_op || !dir->i_op->create)
            error = -EACCES;
        else if (IS_RDONLY(dir))
            error = -EROFS;
        else {
            dir->i_count++;
#ifdef CONFIG_DEBUG_CREAT_ROOTFS
            error = create_rootfs(dir, basename, namelen, mode, res_inode);
#else
            error = dir->i_op->create(dir, basename, namelen, 
                                                 mode, res_inode);
#endif
            up(&dir->i_sem);
            iput(dir);
            return error;
        }
            up(&dir->i_sem);
        /* Create a new inode */
    } else
        error = lookups(dir, basename, namelen, &inode);
    if (error) {
        iput(dir);
        return error;
    }
    error = follow_links(dir, inode, flag, mode, &inode);
    if (error)
        return error;
    if (S_ISDIR(inode->i_mode) && (flag & 2)) {
        /* inode is a directory and contain write permission! it's bad! */
        iput(inode);
        return -EISDIR;
    }
    if (!permissions(inode, ACC_MODE(flag))) {
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
    if ((inode->i_count > 1) && (flag & 2)) {
        for (p = &LAST_TASK; p > &FIRST_TASK; --p) {
            struct vm_area_struct *mpnt;

            if (!*p)
                continue;
            if (inode == (*p)->executable) {
                iput(inode);
                return -ETXTBSY;
            }
            for (mpnt = (*p)->mmap; mpnt; mpnt = mpnt->vm_next) {
                if (mpnt->vm_page_prot & PAGE_RW)
                    continue;
                if (inode == mpnt->vm_inode) {
                    iput(inode);
                    return -ETXTBSY;
                }
            }
        }
    }
    *res_inode = inode;
    return 0;
}
#endif

/*
 * Note that while the flag value (low tow bits) for sys_open means:
 *      00 - read-only
 *      01 - write-only
 *      10 - read-write
 *      11 - special
 * it is changed into
 *      00 - no permissions needed
 *      01 - read-permission
 *      10 - write-permission
 *      11 - read-write
 * for the internal routines (ie open_namei()/follow_link() etc). 00 is
 * used by symlinks.
 */
static int do_opens(const char *filename, int flags, int mode)
{
    struct inode *inode;
    struct file *f;
    int flag, error, fd;

    /*
     * Each task manage a file list that hold file descriptor. This structure
     * is a typical array that search by fd.
     *
     * +----------+
     * |          |
     * |   task   |
     * |          |
     * |          |
     * |          |
     * |   filp  -|--->+-----+-----+-----+----------+-------------+
     * |          |    |     |     |     |          |             |
     * |          |    | fd0 | fd1 | fd2 | ...      | fd[NR_OPEN] |
     * |          |    |     |     |     |          |             |
     * |          |    +-----+-----+-----+----------+-------------+
     * |          |
     * |          |
     * +----------+
     */
    for (fd = 0; fd < NR_OPEN; fd++)
        if (!current->filp[fd])
            break;
    if (fd >= NR_OPEN)
        return -EMFILE;
    /*
     * The argument 'close_on_exec' of current task is a opened file Bitmap.
     * The structure type of 'close_on_exec' is 'struct fd_set' that defined
     * on 'include/linux/types.h'. as follow:
     *
     *   #define __FDSET_LONGS 8
     *   typedef struct fd_set {
     *                unsigned long fds_bits[__FDSET_LONGS];
     *   } fd_set;
     * 
     * This allows for 256 file descriptors: if NR_OPEN is ever grown beyond
     * that your'll have to change this too. But 256 fd's seem to enough
     * even for such "real" unices like SunOS, so hopefully this is one
     * limit that doesn't have to be changed.
     *
     * Note that POSIX want the FD_CLEAR(fd,fdsetp) defines to be in 
     * <sys/time.h> (and thus <linux/time.h>) - but this is a more logical
     * place foe them. 
     *
     * FD_CLR() will send bit value for 'fd' to 'CF' bit of EFLAGS register,
     * and then clear this bit on 'close_on_exec'.
     * A file will be close if bit was set on 'close_on_exec' when invoke 
     * "execve()".
     */
    FD_CLR(fd, &current->close_on_exec);
    /*
     * Get a free file descriptor, VFS utilize 'file_table' to manage all
     * file descriptors. The 'file_table' is a typical signal linked list.
     * The 'first_file' points to the first valid free file descriptor on 
     * "file_table", and all used file descriptor also hold on tail of 
     * 'file_table'.
     *
     *  file_table:
     *
     *      (Used)                  (Free)                  (Free)
     *  +------------+  f_prev  +------------+  f_prev  +------------+
     *  |            |<---------|-           |<---------|-           |<-- ..
     *  | file       |          | file       |          | file       |
     *  | descriptor |          | descriptor |          | descriptor |
     *  |           -|--------->|           -|--------->|           -|--> ..
     *  +------------+  f_next  +------------+  f_next  +------------+
     *                          A
     *                          |
     *          first_file------o
     *
     *
     *
     *  more info see: tools/demo/vfs/core/file_table.c
     */
    f = get_empty_filp();
    if (!f)
        return -ENFILE;
    current->filp[fd] = f;
    /*
     * Setup flage and mode for file descriptor. Note that while the flag
     * value (low two bits) for sys_open means:
     *    00 - read-only
     *    01 - write-only
     *    10 - read-write
     *    11 - special
     * This is changed into:
     *    00 - no permission needed
     *    01 - read-permission
     *    10 - write-permission
     *    11 - read-write
     */
    f->f_flags = flag = flags;
    f->f_mode  = (flag + 1) & O_ACCMODE;
    if (f->f_mode)
        flag++;
    /* If flag contain O_TRUNC or O_CREAT, assign write-permission. On 
     * above operation, O_TRUNC and O_CREAT doesn't be changed. */
    if (flag & (O_TRUNC | O_CREAT))
        flag |= 2;
    /*
     * Now, we invoke 'open_namei()' to obtain special inode with 'filename',
     * 'flag' and 'mode'. If success, we will obtain a inode that contain
     * more information for file on special file-system. It's core routine
     * for 'open' operation.
     */
#ifdef CONFIG_DEBUG_OPEN_NAMEI
    error = parse_open_namei(filename, flag, mode, &inode, NULL);
#else
    error = open_namei(filename, flag, mode, &inode, NULL);
#endif
    if (error) {
        current->filp[fd] = NULL;
        f->f_count--;
        return error;
    }

    f->f_inode = inode;
    f->f_pos   = 0;
    f->f_reada = 0;
    f->f_op    = NULL;
    if (inode->i_op)
        f->f_op = inode->i_op->default_file_ops;
    if (f->f_op && f->f_op->open) {
        error = f->f_op->open(inode, f);
        if (error) {
            iput(inode);
            f->f_count--;
            current->filp[fd] = NULL;
            return error;
        }
    }
    f->f_flags &= ~(O_CREAT | O_EXCL | O_NOCTTY | O_TRUNC);

#ifdef CONFIG_DEBUG_PARSE_FILE_STRUCT
    parse_file_descriptor(f);
#endif
    return (fd);
}

/* This may be used only once, enforced by 'static int callable' */
asmlinkage int sys_demo_open(const char *filename, int flags, int mode)
{
    char *tmp;
    int error;

    error = getname(filename, &tmp);
    if (error)
        return error;
    error = do_opens(tmp, flags, mode);
    putname(tmp);
    return error;
}

/* System call entry */
inline _syscall3(int, demo_open, const char *, file, int, flag, int, mode);

static int debug_open(void)
{
    int fd = -1;
    char buf[20] __unused;

#ifdef CONFIG_DEBUG_OPEN_ORIG
    fd = open("/etc/rc", O_RDWR, 0);
    if (fd < 0) {
        printf("Unable open /etc/rc on original open()\n");
        return -1;
    }
    read(fd, buf, 10);
    printf("Orig-Read: %s\n", buf);
    close(fd);
#endif

#ifdef CONFIG_DEBUG_OPEN_FILE
    fd = demo_open("/etc/rc", O_RDWR, 0);
    if (fd < 0) {
        printf("Unable open /etc/rc on demo_open()\n");
        return -1;
    }

    read(fd, buf, 10);
    printf("Read: %s\n", buf);
#endif

    if (fd > 0)
        close(fd);

#ifdef CONFIG_DEBUG_CREAT_FILE
    fd = demo_open("/etc/BiscuitOSxy1.rc", O_RDWR | O_CREAT, 0);
    if (fd < 0) {
        printf("Unable create new file\n");
        return -1;
    }

    if (fd > 0)
        close(fd);
#endif

    return 0;
}
user1_debugcall_sync(debug_open);
