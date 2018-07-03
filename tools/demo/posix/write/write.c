/*
 * POSIX system call: write
 *
 * (C) 2018.06.18 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/unistd.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/stat.h>

#include <asm/bitops.h>

#include <demo/debug.h>

#define clear_block(addr) \
    __asm__("cld\n\t" \
            "rep\n\t" \
            "stosl" \
            : \
            : "a" (0), "c" (BLOCK_SIZE/4), "D" ((long)(addr)))

#define find_first_zero(addr) ({ \
    int __res; \
    __asm__("cld\n" \
            "1:\tlodsl\n\t" \
            "notl %%eax\n\t" \
            "bsfl %%eax,%%edx\n\t" \
            "jne 2f\n\t" \
            "addl $32,%%ecx\n\t" \
            "cmpl $8192,%%ecx\n\t" \
            "jl 1b\n\t" \
            "xorl %%edx,%%edx\n" \
            "2:\taddl %%edx,%%ecx" \
            : "=c" (__res) : "0" (0), "S" (addr)); \
__res;})

static inline _syscall3(int, open, const char *, file, int, flag, int, mode);
static inline _syscall3(int, read, unsigned int, fd, char *, buf, 
                            unsigned int, count);
static inline _syscall1(int, close, int, fd);

#ifdef CONFIG_DEBUG_WRITE_ORIG
static inline _syscall3(int, write, unsigned int, fd, char *, buf, 
                            unsigned int, count);
#endif

#ifdef CONFIG_MINIX_FS

static int minix_new_block(struct super_block *sb)
{
    struct buffer_head *bh;
    int i, j;

    if (!sb) {
        printk("trying to get new block from nonexistent device\n");
        return 0;
    }
repeat:
    j = 8192;
    for (i = 0; i < 8; i++)
        if ((bh = sb->u.minix_sb.s_zmap[i]) != NULL)
            if ((j = find_first_zero(bh->b_data)) < 8192)
                break;
    if (i >= 8 || !bh || j >= 8192)
        return 0;
    if (set_bit(j, bh->b_data)) {
        printk("new_block: bit already set");
        goto repeat;
    }
    bh->b_dirt = 1;
    j += i * 8192 + sb->u.minix_sb.s_firstdatazone - 1;
    if (j < sb->u.minix_sb.s_firstdatazone ||
                  j >= sb->u.minix_sb.s_nzones)
        return 0;
    if (!(bh = getblk(sb->s_dev, j, BLOCK_SIZE))) {
        printk("new_block: cannot get block");
        return 0;
    }
    clear_block(bh->b_data);
    bh->b_uptodate = 1;
    bh->b_dirt = 1;
    brelse(bh);
    return j;
}

static struct buffer_head *inode_getblk(struct inode *inode, int nr, 
                             int create)
{
    int tmp;
    unsigned short *p;
    struct buffer_head *result;

    p = inode->u.minix_i.i_data + nr;
repeat:
    tmp = *p;
    if (tmp) {
        result = getblk(inode->i_dev, tmp, BLOCK_SIZE);
        if (tmp == *p)
            return result;
        brelse(result);
        goto repeat;
    }
    if (!create)
        return NULL;
    tmp = minix_new_block(inode->i_sb);
}

static struct buffer_head *minix_getblks(struct inode *inode, int block,
                    int create)
{
    struct buffer_head *bh;

    if (block < 0) {
        printk("minix_getblk: block < 0");
        return NULL;
    }
    if (block >= 7 + 512 + 512 * 512) {
        printk("minix_getblk: block > big");
        return NULL;
    }
    if (block < 7)
        return inode_getblk(inode, block, create);
}

static int minix_file_write(struct inode *inode, struct file *filp,
          char *buf, int count)
{
    off_t pos;
    int written, c;
    struct buffer_head *bh;
    char *p;

    if (!inode) {
        printk("minix_file_write: inode = NULL\n");
        return -EINVAL;
    }
    if (!S_ISREG(inode->i_mode)) {
        printk("minix_file_write: mode = %07o\n", inode->i_mode);
        return -EINVAL;
    }
    /*
     * OK, append may not work when many processes are writing at the
     * same time but so what. That way leads to madness anyway.
     */
    if (filp->f_flags & O_APPEND)
        pos = inode->i_size;
    else
        pos = filp->f_pos;
    written = 0;
    while (written < count) {
        bh = minix_getblks(inode, pos / BLOCK_SIZE, 1);
    }
}
#endif

static int write_rootfs(struct inode *inode, struct file *filp, 
                 char *buf, int count)
{
#ifdef CONFIG_MINIX_FS
    minix_file_write(inode, filp, buf, count);
#endif
}

asmlinkage int sys_demo_write(unsigned int fd, char *buf, 
                                 unsigned int count)
{
    int error;
    struct file *file;
    struct inode *inode;

    if (fd >= NR_OPEN || !(file = current->filp[fd]) ||
                         !(inode = file->f_inode)) 
        return -EBADF;
    if (!(file->f_mode & 2))
        return -EBADF;
    if (!file->f_op || !file->f_op->write)
        return -EINVAL;
    error = verify_area(VERIFY_READ, buf, count);
    if (error)
        return error;
#ifdef CONFIG_DEBUG_WRITE_ROOTFS
    return write_rootfs(inode, file, buf, count);
#else
    return file->f_op->write(inode, file, buf, count);
#endif
}

/* build a system call entry for write */
inline _syscall3(int, demo_write, unsigned int, fd, char *, buf,
                                         unsigned int, count);

/* userland code */
static int debug_write(void)
{
    int fd;
    char buf[20] = "BiscuitOS";

    fd = open("/etc/BiscuitOS.rc", O_RDWR | O_CREAT, 0);
    if (fd < 0) {
        printf("Unable open '/etc/BiscuitOS.rc'\n");
        return -1;
    }

#ifdef CONFIG_DEBUG_WRITE_ORIG
    write(fd, buf, 10);
#else
    demo_write(fd, buf, 10);
#endif
    memset(buf, 0, 10);
    read(fd, buf, 10);
    printf("Read: %s\n", buf);
    close(fd);
    return 0;
}
user1_debugcall_sync(debug_write);
