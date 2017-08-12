/*
 * linux/kernel/sys.c
 *
 * (C) 1991 Linus Torvalds
 */

#include <linux/sched.h>
#include <linux/kernel.h>

#include <sys/times.h>

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

int sys_times(struct tms *tbuf)
{
    if (tbuf) {
        verify_area(tbuf, sizeof(*tbuf));
        put_fs_long(current->utime, (unsigned long *)&tbuf->tms_utime);
        put_fs_long(current->stime, (unsigned long *)&tbuf->tms_stime);
        put_fs_long(current->cutime, (unsigned long *)&tbuf->tms_cutime);
        put_fs_long(current->cstime, (unsigned long *)&tbuf->tms_cstime);
    }
    return jiffies;
}

int sys_brk(unsigned long end_data_seg)
{
    if (end_data_seg >= current->end_code &&
        end_data_seg <  current->start_stack - 16384)
        current->brk = end_data_seg;
    return current->brk;
}

int sys_setregid(int rgid, int egid)
{
    if (rgid > 0) {
        if ((current->gid == rgid) ||
             suser())
            current->gid = rgid;
        else
            return -EPERM;
    }
    if (egid > 0) {
        if ((current->gid == egid) ||
            (current->egid == egid) ||
            (current->sgid == egid) ||
             suser())
            current->egid = egid;
        else
            return -EPERM;
    }
    return 0;
}

int sys_setgid(int gid)
{
    return sys_setregid(gid, gid);
}

/*
 * This need some heave checking ...
 * I just haven't get the stomach for it. I also don't fully
 * understand sessions/pgrp etc. Let somebody who does explain it.
 */
int sys_setpgid(int pid, int pgid)
{
    int i;

    if (!pid)
        pid = current->pid;
    if (!pgid)
        pgid = current->pid;
    for (i = 0; i < NR_TASKS; i++)
        if (task[i] && task[i]->pid == pid) {
            if (task[i]->leader)
                return -EPERM;
            if (task[i]->session != current->session)
                return -EPERM;
            task[i]->pgrp = pgid;
            return 0;
        }
    return -ESRCH;
}

int sys_ptrace()
{
    return -ENOSYS;
}

int sys_stty()
{
    return -ENOSYS;
}

int sys_gtty()
{
    return -ENOSYS;
}

int sys_rename()
{
    return -ENOSYS;
}

int sys_prof()
{
    return -ENOSYS;
}

int sys_acct()
{
    return -ENOSYS;
}

int sys_phys()
{
    return -ENOSYS;
}

int sys_lock()
{
    return -ENOSYS;
}

int sys_mpx()
{
    return -ENOSYS;
}

int sys_ulimit()
{
    return -ENOSYS;
}

int sys_ftime()
{
    return -ENOSYS;
}
