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

    return 0;
}
