/*
 * System Call: access
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
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

/* Values for the second argument to access.
   These may be OR'd together.  */
#define R_OK    4               /* Test for read permission.  */
#define W_OK    2               /* Test for write permission.  */
#define X_OK    1               /* Test for execute permission.  */
#define F_OK    0               /* Test for existence.  */


/*
 * XXX should we use the real or effective uid? BSD uses the real uid
 * so as to make this call useful to setuid programs.
 */
int sys_d_access(const char *filename, mode_t mode)
{
    struct m_inode *inode;
    int res, i_mode;

    mode &= 0007;
    if (!(inode = namei(filename)))
        return -EACCES;
    i_mode = res = inode->i_mode & 0777;
    iput(inode);
    if (current->uid == inode->i_uid)
        res >>= 6;
    else if (current->gid == inode->i_gid)
        res >>= 6;
    if ((res & 0007 & mode) == mode)
        return 0;

    /* XXX we are doing this test last because we really should be
     * swapping the effective with the real user id (temporarily),
     * and then calling suser() routine. If we do call the suser()
     * routine, it needs to be called last.
     */
    if ((!current->uid) && 
        (!(mode & 1) || (i_mode & 0111)))
        return 0;
    return -EACCES;
}

/* Invoke by system call: int $0x80 */
int debug_syscall_access0(void)
{
    /* Verify Read permission */
    if ((d_access("/etc/rc", R_OK)) == 0)
        d_printf("Access Read permission\n");

    /* verify write permission */
    if ((d_access("/etc/rc", W_OK)) == 0)
        d_printf("Access Write permission\n");

    /* verify execute permission */
    if ((d_access("/usr/bin/ls", X_OK)) == 0)
        d_printf("Access Execute permission\n");
    return 0;
}
