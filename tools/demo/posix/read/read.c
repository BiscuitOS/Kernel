/*
 * POSIX system call: read
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
#include <linux/mm.h>
#include <linux/stat.h>

#include <demo/debug.h>

#define NBUF      32

static inline _syscall3(int, open, const char *, file, int, flag, int, mode);
static inline _syscall1(int, close, int, fd);

#ifdef CONFIG_MINIX_FS

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
    }
    return NULL;
}

struct buffer_head *minix_getblks(struct inode *inode, int block,
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

static int file_read_minix(struct inode *inode, struct file *filp,
           char *buf, int count)
{
    int read, left, chars;
    int block, blocks, offset;
    int bhrequest, uptodate;
    struct buffer_head **bhb, **bhe;
    struct buffer_head *bhreq[NBUF];
    struct buffer_head *buflist[NBUF];
    unsigned int size;

    if (!inode) {
        printk(KERN_ERR "minix_file_read: inode = NULL\n");
        return -EINVAL;
    }
    if (!S_ISREG(inode->i_mode)) {
        printk(KERN_ERR "minix_file_read: mode = %07o\n", inode->i_mode);
        return -EINVAL;
    }
    offset = filp->f_pos;
    size = inode->i_size;
    if (offset > size)
        left = 0;
    else
        left = size - offset;
    if (left > count)
        left = count;
    if (left <= 0)
        return 0;
    read = 0;
    block = offset >> BLOCK_SIZE_BITS;
    offset &= BLOCK_SIZE - 1;
    size = (size + (BLOCK_SIZE - 1)) >> BLOCK_SIZE_BITS;
    blocks = (left + offset + BLOCK_SIZE - 1) >> BLOCK_SIZE_BITS;
    bhb = bhe = buflist;
    if (filp->f_reada) {
        blocks += read_ahead[MAJOR(inode->i_dev)] / (BLOCK_SIZE >> 9);
        if (block + blocks > size)
            blocks = size - block;
    }

    /* We do this in a two stage process. We first try and request
     * as many blocks as we can, then we wait for the first one to
     * complete, and then we try and wrap up as many as are actually
     * done. This routine is rather generic, in that it can be used
     * in a filesystem by substituting the appropriate function in
     * for getblk.
     *
     * This routine is optimized to make maximum use of the various
     * buffers and caches. */

    do {
        bhrequest = 0;
        uptodate = 1;
        while (blocks) {
            --blocks;
            *bhb = minix_getblks(inode, block++, 0);
        }
    } while (left > 0);

}
#endif

#ifdef CONFIG_DEBUG_READ_ROOTFS
static int rootfs_read(struct inode *inode, struct file *filp, char *buf,
                     unsigned int count)
{
#ifdef CONFIG_MINIX_FS
    file_read_minix(inode, filp, buf, count);
#endif
    return 0;
}
#endif

/* This may be used only once, enforced by 'static int callable' */
asmlinkage int sys_demo_read(unsigned int fd, char *buf, unsigned int count)
{
    int error;
    struct file *file;
    struct inode *inode;

    /*
     * Each task structure manage all opened file descriptor on filp,
     * we can get special file structure via fd. Unique inode structure
     * hold on file structure that points to special inode. NOTE!
     * each task only can manage 'NR_OPEN' file structure.
     */
    if (fd >= NR_OPEN || !(file = current->filp[fd]) ||
                         !(inode = file->f_inode))
        return -EBADF;
    /*
     * Note that while the flag value (low two bits) for sys_open means:
     *     00 - read-only
     *     01 - write-only
     *     10 - read-write
     *     11 - special
     * On file descriptor it is changed into 
     *     00 - no permission needed
     *     01 - read-permission
     *     10 - write-permission
     *     11 - read-write
     * So on sys_read, f_mode must need read-permission..
     */
    if (!(file->f_mode & 1))
        return -EBADF;
    if (!file->f_op || !file->f_op->read)
        return -EINVAL;
    if (!count)
        return 0;
    /*
     * Verify whether userland address is safe that doesn't over TASK_SIZE.
     */
    error = verify_area(VERIFY_WRITE, buf, count);
    if (error)
        return error;
#ifdef CONFIG_DEBUG_READ_ROOTFS
    return rootfs_read(inode, file, buf, count);
#else
    return file->f_op->read(inode, file, buf, count);
#endif
}

/* build a system call entry for read */
inline _syscall3(int, demo_read, unsigned int, fd, char *, buf,
                                         unsigned int, count);

/* userland code */
static int debug_read(void)
{
    int fd;
    char buf[20];

    fd = open("/etc/rc", O_RDONLY, 0);
    if (fd < 0) {
        printf("Unable open '/etc/rc'\n");
        return -1;
    }

    demo_read(fd, buf, 10);
    printf("Read: %s\n", buf);
    close(fd);
    return 0;
}
user1_debugcall_sync(debug_read);
