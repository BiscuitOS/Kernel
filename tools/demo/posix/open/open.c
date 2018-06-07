/*
 * System Call: open
 *
 * (C) 2018.03 BiscuitOS <buddy.zhang@aliyun.com>
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

#include <test/debug.h>

#include <errno.h>
#include <fcntl.h>

extern int d_open_namei(const char *pathname, int flag, int mode,
                 struct m_inode **res_inode);

/* system call: d_open - Open or create a file.
 *
 * @filename: The pathname to the file.
 * @flag:     The kind of access requested on the file (read, write, append 
 *            etc.).
 * @mode:     The initial file permission is requested using the third 
 *            argument called mode. This argument is relevant only when 
 *            a new file is being created.
 *
 * @return: file descriptor.
 *
 * After using the file, the process should close the file using close call, 
 * which takes the file descriptor of the file to be closed. Some filesystems
 * include a disposition to permit releasing the file.
 * 
 * Some computer languages include run-time libraries which include additional
 * functionality for particular filesystems. The open (or some auxiliary 
 * routine) may include specifications for key size, record size, connection 
 * speed. Some open routines include specification of the program code to be 
 * executed in the event of an error.
 *
 * Open flag: (flag argument) 
 *   O_RDONLY: Only read file
 *   O_WRONLY: Only write file
 *   O_ORDWR:  Read or Write file
 *   O_CREAT:  Create the file if it does not exist; otherwise the open fails
 *             setting errno to ENOENT.
 *   O_EXCL:   Used with O_CREAT if the file already exists, then fail, setting
 *             errno to EEXIST.
 *   O_APPEND: data written will be appended to the end of the file. The file
 *             operations will always adjust the position pointer to the end 
 *             of the file.
 *   O_TRUNC:  If the file already exists then discard its previous contents, 
 *             reducing it to an empty file. Not applicable for a device or 
 *             named pipe.
 *
 * mode:
 *   Optional and relevant only when creating a new file, defines the file 
 *   permissions. These include read, write or execute the file by the owner, 
 *   group or all users. The mode is masked by the calling process's umask: 
 *   bits set in the umask are cleared in the mode.
 *
 *   S_IRUSR: User readable.
 *   S_IRGRP: Group readable.
 *   S_IROTH: Other readable.
 *
 *   S_IWUSR: User writeable.
 *   S_IWGRP: Group writeable.
 *   S_IWOTH: Other writeable.
 *
 *   S_IXUSR: User executable.
 *   S_IXGRP: Group executable.
 *   S_IXOTH: Other executable.
 *
 * Error Code:
 *   EINTR:    The system call was interrupted.
 *   EIO:      Low-level errors, often concerned with hardware read/write 
 *             operations.
 *   EBADF:    The file descriptor fd is not valid, or an attempt is being 
 *             made to write into a file opened in 'read-only' mode.
 *   EACCES:   The user does not have the necessary permissions to write into
 *             the file.
 *   EFAULT:   The address specified in the function is an invalid address.
 *   EINVAL:   The argument(s) passed with the function is(are) invalid.
 *   EFBIG:    The file size specified in nbytes is too large, and is greater 
 *             than that allowed by the system.
 *   ENOSPC:   No space available for writing onto the storage device.
 *   EPIPE:    The pipe is either broken, or the file at the other end of the
 *             pipe is not open for I/O purposes (most processes giving this 
 *             type of error also generate the SIGPIPE signal).
 */
int sys_d_open(const char *filename, int flag, int mode)
{
    struct m_inode *inode;
    struct file *f;
    int fd, i;
 
    mode &= 0777 & ~current->umask;
    /* Obtain a valid fd from current task */
    for (fd = 0; fd < NR_OPEN; fd++)
        if (!current->filp[fd])
            break;
    if (fd >= NR_OPEN)
        return -EINVAL;

    /* Set close mask */
    current->close_on_exec &= ~(1 << fd);
    /* Obtain a valid structure for file */
    f = 0 + file_table;
    for (i = 0; i < NR_FILE; i++, f++)
        if (!f->f_count)
            break;
    if (i >= NR_FILE)
        return -EINVAL;

    (current->filp[fd] = f)->f_count++;
    /* Obtain special inode structure for file */
    if ((i = d_open_namei(filename, flag, mode, &inode)) < 0) {
        current->filp[fd] = NULL;
        f->f_count = 0;
        return i;
    }

    /* ttys are somewhat special (ttyxx major == 4, tty major == 5) */
    if (S_ISCHR(inode->i_mode)) {
        if (MAJOR(inode->i_zone[0]) == 4) {
            if (current->leader && current->tty < 0) {
                /* Nothing routine */
            }
        } else if (MAJOR(inode->i_zone[0]) == 5)
            if (current->tty < 0) {
                iput(inode);
                current->filp[fd] = NULL;
                f->f_count = 0;
                return -EPERM;
            }
    }
    /* Likewise with block-devices: check for floppy_change */
    if (S_ISBLK(inode->i_mode))
        /* Noting to do this routine */;
    f->f_mode   = inode->i_mode;
    f->f_flags  = flag;
    f->f_count  = 1;
    f->f_inode  = inode;
    f->f_pos    = 0;
    return (fd);
}

/* Invoke by system call: int $0x80 */
int debug_syscall_open0(void)
{
    int fd;
    int fd0, fd1, fd2, fd3;

    /* Open a exist file */
    fd = d_open("/etc/profile", O_RDWR, 0);
    /* Open '.' */
    fd0 = d_open(".", O_RDWR, 0);
    /* Open ".." */
    fd1 = d_open("..", O_RDWR, 0);
    /* Create a file */
    fd2 = d_open("/etc/biscuitos.conf", O_CREAT | O_RDWR);
    /* Open /dev/ttyX */
    fd3 = d_open("/dev/tty0", O_RDONLY, 0);

    /* Other operation */
    write(fd2, "BiscuitOS\n", 10);
    printf("Open /etc/profile");

    /* close file */
    close(fd3);
    close(fd2);
    close(fd1);
    close(fd0);
    close(fd);
    return 0;
}
