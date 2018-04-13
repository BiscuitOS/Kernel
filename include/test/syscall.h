#ifndef _DEBUG_SYSCALL_H
#define _DEBUG_SYSCALL_H

#ifdef CONFIG_DEBUG_SYSCALL
extern int debug_syscall_common(void);
extern int debug_syscall_common_userland(void);

#ifdef CONFIG_DEBUG_SYSCALL_ROUTINE
extern void system_call_rountine(void);
#endif 

#ifdef CONFIG_DEBUG_SYSCALL_STACK
extern int debug_syscall_stack(void);
#endif

#ifdef CONFIG_DEBUG_SYSCALL_OPEN
extern int debug_syscall_open_common_userland(void);

#ifdef CONFIG_DEBUG_SYSCALL_OPEN0
extern int debug_syscall_open0(void);
extern struct m_inode *d_iget(int dev, int nr);
extern struct m_inode *d_new_inode(int dev);

#endif
#endif // CONFIG_DEBUG_SYSCALL_OPEN

#ifdef CONFIG_DEBUG_SYSCALL_CLOSE
extern int debug_syscall_close_common_userland(void);
#ifdef CONFIG_DEBUG_SYSCALL_CLOSE0
extern int debug_syscall_close0(void);
#endif

#endif // CONFIG_DEBUG_SYSCALL_CLOSE

#ifdef CONFIG_DEBUG_SYSCALL_READ
extern int debug_syscall_read_common_userland(void);
#ifdef CONFIG_DEBUG_SYSCALL_READ0
extern int debug_syscall_read0(void);
#endif
#endif // CONFIG_DEBUG_SYSCALL_READ

#ifdef CONFIG_DEBUG_SYSCALL_WRITE
extern int debug_syscall_write_common_userland(void);
#ifdef CONFIG_DEBUG_SYSCALL_WRITE0
extern int debug_syscall_write0(void);
#endif
#endif // CONFIG_DEBUG_SYSCALL_WRITE

#ifdef CONFIG_DEBUG_SYSCALL_DUP
extern int debug_syscall_dup_common_userland(void);
#ifdef CONFIG_DEBUG_SYSCALL_DUP0
extern int debug_syscall_dup0(void);
extern int dupfd(unsigned int fd, unsigned int arg);
#endif
#endif // CONFIG_DEBUG_SYSCALL_DUP

#ifdef CONFIG_DEBUG_SYSCALL_FORK
extern int debug_syscall_fork_common_userland(void);
#ifdef CONFIG_DEBUG_SYSCALL_FORK0
extern int debug_syscall_fork0(void);
extern int d_copy_page_tables(unsigned long from, unsigned long to, long size);
extern int d_free_page_tables(unsigned long from, unsigned long size);
#endif
#endif // CONFIG_DEBUG_SYSCALL_FORK

#ifdef CONFIG_DEBUG_SYSCALL_EXECVE
extern int debug_syscall_execve_common_userland(void);
#ifdef CONFIG_DEBUG_SYSCALL_EXECVE0
extern int debug_syscall_execve0(void);
extern int d_do_execve(unsigned long *eip, long tmp, char *filename,
              char **argv, char **envp);
#endif
#endif // CONFIG_DEBUG_SYSCALL_DUP

#ifdef CONFIG_DEBUG_SYSCALL_CREAT
extern int debug_syscall_creat_common_userland(void);
#ifdef CONFIG_DEBUG_SYSCALL_CREAT0
extern int debug_syscall_creat0(void);
#endif
#endif // CONFIG_DEBUG_SYSCALL_CREAT

#ifdef CONFIG_DEBUG_SYSCALL_LINK
extern int debug_syscall_link_common_userland(void);
#ifdef CONFIG_DEBUG_SYSCALL_LINK0
extern int debug_syscall_link0(void);
#endif
#endif // CONFIG_DEBUG_SYSCALL_LINK

#ifdef CONFIG_DEBUG_SYSCALL_UNLINK
extern int debug_syscall_unlink_common_userland(void);
#ifdef CONFIG_DEBUG_SYSCALL_UNLINK0
extern int debug_syscall_unlink0(void);
#endif
#endif // CONFIG_DEBUG_SYSCALL_UNLINK

#ifdef CONFIG_DEBUG_SYSCALL_MKDIR
extern int debug_syscall_mkdir_common_userland(void);
#ifdef CONFIG_DEBUG_SYSCALL_MKDIR0
extern int debug_syscall_mkdir0(void);
#endif
#endif // CONFIG_DEBUG_SYSCALL_MKDIR

#ifdef CONFIG_DEBUG_SYSCALL_RMDIR
extern int debug_syscall_rmdir_common_userland(void);
#ifdef CONFIG_DEBUG_SYSCALL_RMDIR0
extern int debug_syscall_rmdir0(void);
#endif
#endif // CONFIG_DEBUG_SYSCALL_RMDIR

#ifdef CONFIG_DEBUG_SYSCALL_MKNOD
extern int debug_syscall_mknod_common_userland(void);
#ifdef CONFIG_DEBUG_SYSCALL_MKNOD0
extern int debug_syscall_mknod0(void);
#endif
#endif // CONFIG_DEBUG_SYSCALL_MKNOD

#ifdef CONFIG_DEBUG_SYSCALL_ACCESS
extern int debug_syscall_access_common_userland(void);
#ifdef CONFIG_DEBUG_SYSCALL_ACCESS0
extern int debug_syscall_access0(void);
#endif
#endif // CONFIG_DEBUG_SYSCALL_ACCESS

#ifdef CONFIG_DEBUG_SYSCALL_ACCT
extern int debug_syscall_acct_common_userland(void);
#ifdef CONFIG_DEBUG_SYSCALL_ACCT0
extern int debug_syscall_acct0(void);
#endif
#endif // CONFIG_DEBUG_SYSCALL_ACCT

#ifdef CONFIG_DEBUG_SYSCALL_ALARM
extern int debug_syscall_alarm_common_userland(void);
#ifdef CONFIG_DEBUG_SYSCALL_ALARM0
extern int debug_syscall_alarm0(void);
#endif
#endif // CONFIG_DEBUG_SYSCALL_ALARM

#ifdef CONFIG_DEBUG_SYSCALL_CHDIR
extern int debug_syscall_chdir_common_userland(void);
#ifdef CONFIG_DEBUG_SYSCALL_CHDIR0
extern int debug_syscall_chdir0(void);
#endif
#endif // CONFIG_DEBUG_SYSCALL_CHDIR

#ifdef CONFIG_DEBUG_SYSCALL_CHMOD
extern int debug_syscall_chmod_common_userland(void);
#ifdef CONFIG_DEBUG_SYSCALL_CHMOD0
extern int debug_syscall_chmod0(void);
#endif
#endif // CONFIG_DEBUG_SYSCALL_CHMOD

#ifdef CONFIG_DEBUG_SYSCALL_CHOWN
extern int debug_syscall_chown_common_userland(void);
#ifdef CONFIG_DEBUG_SYSCALL_CHOWN0
extern int debug_syscall_chown0(void);
#endif
#endif // CONFIG_DEBUG_SYSCALL_CHOWN

#ifdef CONFIG_DEBUG_SYSCALL_UTIME
extern int debug_syscall_utime_common_userland(void);
#ifdef CONFIG_DEBUG_SYSCALL_UTIME0
extern int debug_syscall_utime0(void);
#endif
#endif // CONFIG_DEBUG_SYSCALL_UTIME

#ifdef CONFIG_DEBUG_SYSCALL_CHROOT
extern int debug_syscall_chroot_common_userland(void);
#ifdef CONFIG_DEBUG_SYSCALL_CHROOT0
extern int debug_syscall_chroot0(void);
#endif
#endif // CONFIG_DEBUG_SYSCALL_CHROOT

#ifdef CONFIG_DEBUG_SYSCALL_USTAT
extern int debug_syscall_ustat_common_userland(void);
#ifdef CONFIG_DEBUG_SYSCALL_USTAT0
extern int debug_syscall_ustat0(void);
#endif
#endif // CONFIG_DEBUG_SYSCALL_USTAT

#ifdef CONFIG_DEBUG_SYSCALL_FSTAT
extern int debug_syscall_fstat_common_userland(void);
#ifdef CONFIG_DEBUG_SYSCALL_FSTAT0
extern int debug_syscall_fstat0(void);
#endif
#endif // CONFIG_DEBUG_SYSCALL_FSTAT

#ifdef CONFIG_DEBUG_SYSCALL_STAT
extern int debug_syscall_stat_common_userland(void);
#ifdef CONFIG_DEBUG_SYSCALL_STAT0
extern int debug_syscall_stat0(void);
#endif
#endif // CONFIG_DEBUG_SYSCALL_STAT

#ifdef CONFIG_DEBUG_SYSCALL_GETPID
extern int debug_syscall_getpid_common_userland(void);
#ifdef CONFIG_DEBUG_SYSCALL_GETPID0
extern int debug_syscall_getpid0(void);
#endif
#endif // CONFIG_DEBUG_SYSCALL_GETPID

#ifdef CONFIG_DEBUG_SYSCALL_GETUID
extern int debug_syscall_getuid_common_userland(void);
#ifdef CONFIG_DEBUG_SYSCALL_GETUID0
extern int debug_syscall_getuid0(void);
#endif
#endif // CONFIG_DEBUG_SYSCALL_GETUID

#ifdef CONFIG_DEBUG_SYSCALL_PAUSE
extern int debug_syscall_pause_common_userland(void);
#ifdef CONFIG_DEBUG_SYSCALL_PAUSE0
extern int debug_syscall_pause0(void);
#endif
#endif // CONFIG_DEBUG_SYSCALL_PAUSE

#ifdef CONFIG_DEBUG_SYSCALL_NICE
extern int debug_syscall_nice_common_userland(void);
#ifdef CONFIG_DEBUG_SYSCALL_NICE0
extern int debug_syscall_nice0(void);
#endif
#endif // CONFIG_DEBUG_SYSCALL_NICE

#ifdef CONFIG_DEBUG_SYSCALL_GETGID
extern int debug_syscall_getgid_common_userland(void);
#ifdef CONFIG_DEBUG_SYSCALL_GETGID0
extern int debug_syscall_getgid0(void);
#endif
#endif // CONFIG_DEBUG_SYSCALL_GETGID

#ifdef CONFIG_DEBUG_SYSCALL_GETEUID
extern int debug_syscall_geteuid_common_userland(void);
#ifdef CONFIG_DEBUG_SYSCALL_GETEUID0
extern int debug_syscall_geteuid0(void);
#endif
#endif // CONFIG_DEBUG_SYSCALL_GETEUID

#ifdef CONFIG_DEBUG_SYSCALL_GETPPID
extern int debug_syscall_getppid_common_userland(void);
#ifdef CONFIG_DEBUG_SYSCALL_GETPPID0
extern int debug_syscall_getppid0(void);
#endif
#endif // CONFIG_DEBUG_SYSCALL_GETPPID

#ifdef CONFIG_DEBUG_SYSCALL_TIME
extern int debug_syscall_time_common_userland(void);
#ifdef CONFIG_DEBUG_SYSCALL_TIME0
extern int debug_syscall_time0(void);
#endif
#endif // CONFIG_DEBUG_SYSCALL_TIME

#ifdef CONFIG_DEBUG_SYSCALL_STIME
extern int debug_syscall_stime_common_userland(void);
#ifdef CONFIG_DEBUG_SYSCALL_STIME0
extern int debug_syscall_stime0(void);
#endif
#endif // CONFIG_DEBUG_SYSCALL_STIME

#ifdef CONFIG_DEBUG_SYSCALL_TIMES
extern int debug_syscall_times_common_userland(void);
#ifdef CONFIG_DEBUG_SYSCALL_TIMES0
extern int debug_syscall_times0(void);
#endif
#endif // CONFIG_DEBUG_SYSCALL_TIMES

#ifdef CONFIG_DEBUG_SYSCALL_FTIME
extern int debug_syscall_ftime_common_userland(void);
#ifdef CONFIG_DEBUG_SYSCALL_FTIME0
extern int debug_syscall_ftime0(void);
#endif
#endif // CONFIG_DEBUG_SYSCALL_FTIME

#ifdef CONFIG_DEBUG_SYSCALL_ULIMIT
extern int debug_syscall_ulimit_common_userland(void);
#ifdef CONFIG_DEBUG_SYSCALL_ULIMIT0
extern int debug_syscall_ulimit0(void);
#endif
#endif // CONFIG_DEBUG_SYSCALL_ULIMIT

#ifdef CONFIG_DEBUG_SYSCALL_MPX
extern int debug_syscall_mpx_common_userland(void);
#ifdef CONFIG_DEBUG_SYSCALL_MPX0
extern int debug_syscall_mpx0(void);
#endif
#endif // CONFIG_DEBUG_SYSCALL_MPX

#ifdef CONFIG_DEBUG_SYSCALL_LOCK
extern int debug_syscall_lock_common_userland(void);
#ifdef CONFIG_DEBUG_SYSCALL_LOCK0
extern int debug_syscall_lock0(void);
#endif
#endif // CONFIG_DEBUG_SYSCALL_LOCK

#ifdef CONFIG_DEBUG_SYSCALL_PHYS
extern int debug_syscall_phys_common_userland(void);
#ifdef CONFIG_DEBUG_SYSCALL_PHYS0
extern int debug_syscall_phys0(void);
#endif
#endif // CONFIG_DEBUG_SYSCALL_PHYS

#ifdef CONFIG_DEBUG_SYSCALL_PROF
extern int debug_syscall_prof_common_userland(void);
#ifdef CONFIG_DEBUG_SYSCALL_PROF0
extern int debug_syscall_prof0(void);
#endif
#endif // CONFIG_DEBUG_SYSCALL_PROF

#ifdef CONFIG_DEBUG_SYSCALL_GTTY
extern int debug_syscall_gtty_common_userland(void);
#ifdef CONFIG_DEBUG_SYSCALL_GTTY0
extern int debug_syscall_gtty0(void);
#endif
#endif // CONFIG_DEBUG_SYSCALL_GTTY

#ifdef CONFIG_DEBUG_SYSCALL_STTY
extern int debug_syscall_stty_common_userland(void);
#ifdef CONFIG_DEBUG_SYSCALL_STTY0
extern int debug_syscall_stty0(void);
#endif
#endif // CONFIG_DEBUG_SYSCALL_STTY

#ifdef CONFIG_DEBUG_SYSCALL_PTRACE
extern int debug_syscall_ptrace_common_userland(void);
#ifdef CONFIG_DEBUG_SYSCALL_PTRACE0
extern int debug_syscall_ptrace0(void);
#endif
#endif // CONFIG_DEBUG_SYSCALL_PTRACE

#ifdef CONFIG_DEBUG_SYSCALL_GETPGRP
extern int debug_syscall_getpgrp_common_userland(void);
#ifdef CONFIG_DEBUG_SYSCALL_GETPGRP0
extern int debug_syscall_getpgrp0(void);
#endif
#endif // CONFIG_DEBUG_SYSCALL_GETPGRP

#ifdef CONFIG_DEBUG_SYSCALL_SETSID
extern int debug_syscall_setsid_common_userland(void);
#ifdef CONFIG_DEBUG_SYSCALL_SETSID0
extern int debug_syscall_setsid0(void);
#endif
#endif // CONFIG_DEBUG_SYSCALL_SETSID

#ifdef CONFIG_DEBUG_SYSCALL_UMASK
extern int debug_syscall_umask_common_userland(void);
#ifdef CONFIG_DEBUG_SYSCALL_UMASK0
extern int debug_syscall_umask0(void);
#endif
#endif // CONFIG_DEBUG_SYSCALL_UMASK

#endif // CONFIG_DEBUG_SYSCALL

#endif
