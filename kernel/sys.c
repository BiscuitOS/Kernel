/*
 * linux/kernel/sys.c
 *
 * (C) 1991 Linus Torvalds
 */

#include <linux/sched.h>
#include <linux/kernel.h>

#include <asm/segment.h>

#include <errno.h>

int sys_time(long *tloc)
{
    int i;

    i = CURRENT_TIME;
    if (tloc) {
        verify_area(tloc, 4);
        put_fs_long(i, (unsigned long *)tloc);
    }
    return i;
}

int sys_break()
{
    return -ENOSYS;
}

/*
 * Unprivileged users may change the real user id to the effective uid
 * or vice versa.
 */
int sys_setreuid(int ruid, int euid)
{
    int old_ruid = current->uid;

    if (ruid > 0) {
        if ((current->euid == ruid) ||
            (old_ruid == ruid) ||
            suser())
            current->uid = ruid;
        else
            return -EPERM;
    }
    if (euid > 0) {
        if ((old_ruid == euid) ||
            (current->euid == euid) ||
            suser())
            current->euid = euid;
        else {
            current->uid = old_ruid;
            return -EPERM;
        }
    }
    return 0;
}

int sys_setuid(int uid)
{
    return (sys_setreuid(uid, uid));
}

int sys_stime(long *tptr)
{
    if (!suser())
        return -EPERM;
    startup_time = get_fs_long((unsigned long *)tptr) - jiffies / HZ;
    return 0;
}
