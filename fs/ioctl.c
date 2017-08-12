/*
 * linux/fs/ioctl.c
 *
 * (C) 1991 Linus Torvalds
 */

#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/sched.h>

#include <sys/stat.h>

#include <errno.h>

extern int tty_ioctl(int dev, int cmd, int arg);
typedef int (*ioctl_ptr)(int dev, int cmd, int arg);

#define NRDEVS ((sizeof(ioctl_table))/(sizeof(ioctl_ptr)))

static ioctl_ptr ioctl_table[] = {
    NULL,          /* nodev */
    NULL,          /* /dev/mem */
    NULL,          /* /dev/fd */
    NULL,          /* /dev/hd */
    tty_ioctl,     /* /dev/ttyx */
    tty_ioctl,     /* /dev/tty */
    NULL,          /* /dev/lp */
    NULL,          /* named pipes */
};

int sys_ioctl(unsigned int fd, unsigned int cmd, unsigned long arg)
{
    struct file *filp;
    int dev, mode;

    if (fd >= NR_OPEN || !(filp = current->filp[fd]))
        return -EBADF;
    mode = filp->f_inode->i_mode;
    if (!S_ISCHR(mode) && !S_ISBLK(mode))
        return -EINVAL;
    dev = filp->f_inode->i_zone[0];
    if (MAJOR(dev) >= NRDEVS)
        return -ENODEV;
    if (!ioctl_table[MAJOR(dev)])
        return -ENOTTY;
    return ioctl_table[MAJOR(dev)](dev, cmd, arg);
}
