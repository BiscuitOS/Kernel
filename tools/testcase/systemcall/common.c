/*
 * Common System Call
 *
 * (C) 2018.03 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <test/debug.h>

int debug_syscall_common(void)
{
#ifdef CONFIG_DEBUG_SYSCALL_ROUTINE
    system_call_rountine();
#endif
    return 0;
}

/* Debug on userland */
int debug_syscall_common_userland(void)
{
#ifdef CONFIG_DEBUG_SYSCALL_OPEN
    debug_syscall_open_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_CLOSE
    debug_syscall_close_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_READ
    debug_syscall_read_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_WRITE
    debug_syscall_write_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_DUP
    debug_syscall_dup_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_EXECVE
    debug_syscall_execve_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_FORK
    debug_syscall_fork_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_EXECVE
    debug_syscall_execve_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_CREAT
    debug_syscall_creat_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_STACK
    debug_syscall_stack();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_LINK
    debug_syscall_link_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_UNLINK
    debug_syscall_unlink_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_MKDIR
    debug_syscall_mkdir_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_RMDIR
    debug_syscall_rmdir_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_MKNOD
    debug_syscall_mknod_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_ACCESS
    debug_syscall_access_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_ACCT
    debug_syscall_acct_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_ALARM
    debug_syscall_alarm_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_CHDIR
    debug_syscall_chdir_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_CHMOD
    debug_syscall_chmod_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_CHOWN
    debug_syscall_chown_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_UTIME
    debug_syscall_utime_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_CHROOT
    debug_syscall_chroot_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_USTAT
    debug_syscall_ustat_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_FSTAT
    debug_syscall_fstat_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_STAT
    debug_syscall_stat_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_GETPID
    debug_syscall_getpid_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_GETUID
    debug_syscall_getuid_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_PAUSE
    debug_syscall_pause_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_NICE
    debug_syscall_nice_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_GETGID
    debug_syscall_getgid_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_GETEUID
    debug_syscall_geteuid_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_GETPPID
    debug_syscall_getppid_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_TIME
    debug_syscall_time_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_STIME
    debug_syscall_stime_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_TIMES
    debug_syscall_times_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_FTIME
    debug_syscall_ftime_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_ULIMIT
    debug_syscall_ulimit_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_MPX
    debug_syscall_mpx_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_LOCK
    debug_syscall_lock_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_PHYS
    debug_syscall_phys_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_PROF
    debug_syscall_prof_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_GTTY
    debug_syscall_gtty_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_STTY
    debug_syscall_stty_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_PTRACE
    debug_syscall_ptrace_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_GETPGRP
    debug_syscall_getpgrp_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_SETSID
    debug_syscall_setsid_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_UMASK
    debug_syscall_umask_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_UNAME
    debug_syscall_uname_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_SETPGID
    debug_syscall_setpgid_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_SETGID
    debug_syscall_setgid_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_SETREGID
    debug_syscall_setregid_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_BRK
    debug_syscall_brk_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_SETUID
    debug_syscall_setuid_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_SETREUID
    debug_syscall_setreuid_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_BREAK
    debug_syscall_break_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_RENAME
    debug_syscall_rename_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_FCNTL
    debug_syscall_fcntl_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_DUP2
    debug_syscall_dup2_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_LSEEK
    debug_syscall_lseek_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_MOUNT
    debug_syscall_mount_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_UMOUNT
    debug_syscall_umount_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_SETUP
    debug_syscall_setup_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_SIGNAL
    debug_syscall_signal_common_userland();
#endif

    return 0;
}
