/*
 * POSIX system call: read
 *
 * (C) 2018.06.17 BiscuitOS <buddy.zhang@aliyun.com>
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

#include <demo/debug.h>

static inline _syscall3(int, open, const char *, file, int, flag, int, mode);
static inline _syscall1(int, close, int, fd);

#ifdef CONFIG_DEBUG_READ_ROOTFS
static int rootfs_read(struct inode *inode, struct file *filp, char *buf,
                     unsigned int count)
{
#ifdef CONFIG_MINIX_FS
    
#endif
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
