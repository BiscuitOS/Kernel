#ifndef _UNISTD_H_
#define _UNISTD_H_

/* ok, this may be a joke, but I'm working on it */
#define _POSIX_VERSION 198808L

#define _POSIX_CHOWN_RESTRICTED	/* only root can do a chown (I think..) */
#define _POSIX_NO_TRUNC		/* no pathname truncation (but see in kernel) */
#define _POSIX_VDISABLE '\0'	/* character to disable things like ^C */
#define _POSIX_JOB_CONTROL
#define _POSIX_SAVED_IDS	/* Implemented, for whatever good it is */

#ifdef CONFIG_DEBUG_USERLAND_SYSCALL
#define DEBUG_SYSCALL_NR     87
#endif

#define STDIN_FILENO	0
#define STDOUT_FILENO	1
#define STDERR_FILENO	2

#ifndef NULL
#define NULL    ((void *)0)
#endif

/* access */
#define F_OK	0
#define X_OK	1
#define W_OK	2
#define R_OK	4

/* lseek */
#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2

/* _SC stands for System Configuration. We don't use them much */
#define _SC_ARG_MAX		1
#define _SC_CHILD_MAX		2
#define _SC_CLOCKS_PER_SEC	3
#define _SC_NGROUPS_MAX		4
#define _SC_OPEN_MAX		5
#define _SC_JOB_CONTROL		6
#define _SC_SAVED_IDS		7
#define _SC_VERSION		8

/* more (possibly) configurable things - now pathnames */
#define _PC_LINK_MAX		1
#define _PC_MAX_CANON		2
#define _PC_MAX_INPUT		3
#define _PC_NAME_MAX		4
#define _PC_PATH_MAX		5
#define _PC_PIPE_BUF		6
#define _PC_NO_TRUNC		7
#define _PC_VDISABLE		8
#define _PC_CHOWN_RESTRICTED	9

#include <sys/stat.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/utsname.h>
#include <sys/resource.h>
#include <utime.h>

#ifdef __LIBRARY__

#define __NR_setup    0 /* used only by init, to get system going */
#define __NR_exit     1
#define __NR_fork     2
#define __NR_read     3
#define __NR_write    4
#define __NR_open     5
#define __NR_close    6
#define __NR_waitpid  7
#define __NR_creat    8
#define __NR_link     9
#define __NR_unlink   10
#define __NR_execve   11
#define __NR_chdir    12
#define __NR_time     13
#define __NR_mknod    14
#define __NR_chmod    15
#define __NR_chown    16
#define __NR_break    17
#define __NR_stat     18
#define __NR_lseek    19
#define __NR_getpid   20
#define __NR_mount    21
#define __NR_umount   22
#define __NR_setuid   23
#define __NR_getuid   24
#define __NR_stime    25
#define __NR_ptrace   26
#define __NR_alarm    27
#define __NR_fstat    28
#define __NR_pause    29
#define __NR_utime    30
#define __NR_stty     31
#define __NR_gtty     32
#define __NR_access   33
#define __NR_nice     34
#define __NR_ftime    35
#define __NR_sync     36
#define __NR_kill     37
#define __NR_rename   38
#define __NR_mkdir    39
#define __NR_rmdir    40
#define __NR_dup      41
#define __NR_pipe     42
#define __NR_times    43
#define __NR_prof     44
#define __NR_brk      45
#define __NR_setgid   46
#define __NR_getgid   47
#define __NR_signal   48
#define __NR_geteuid  49
#define __NR_getegid  50
#define __NR_acct     51
#define __NR_phys     52
#define __NR_lock     53
#define __NR_ioctl    54
#define __NR_fcntl    55
#define __NR_mpx      56
#define __NR_setpgid  57
#define __NR_ulimit   58
#define __NR_uname    59
#define __NR_umask    60
#define __NR_chroot   61
#define __NR_ustat    62
#define __NR_dup2     63
#define __NR_getppid  64
#define __NR_getpgrp  65
#define __NR_setsid   66
#define __NR_sigaction 67
#define __NR_sgetmask 68
#define __NR_ssetmask 69
#define __NR_setreuid 70
#define __NR_setregid 71
#define __NR_sigsuspend	72
#define __NR_sigpending 73
#define __NR_sethostname 74
#define __NR_setrlimit	75
#define __NR_getrlimit	76
#define __NR_getrusage	77
#define __NR_gettimeofday 78
#define __NR_settimeofday 79
#define __NR_getgroups	80
#define __NR_setgroups	81
#define __NR_select	82
#define __NR_symlink	83
#define __NR_lstat	84
#define __NR_readlink	85
#define __NR_uselib	86

#ifdef CONFIG_DEBUG_SYSCALL_OPEN0
#define __NR_d_open   DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_SYSCALL_CLOSE0
#define __NR_d_close  DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_SYSCALL_READ0
#define __NR_d_read   DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_SYSCALL_WRITE0
#define __NR_d_write  DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_SYSCALL_DUP0
#define __NR_d_dup    DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_SYSCALL_EXECVE0
#define __NR_d_execve DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_SYSCALL_FORK0
#define __NR_d_fork   DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_BINARY_AOUT
#define __NR_d_execve DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_BINARY_ELF
#define __NR_d_parse_elf DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_SYSCALL_STACK
#define __NR_d_stack  DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_SEGMENT_FS
#define __NR_d_fs     DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_MMU
#define __NR_d_mmu    DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_SYSCALL_CREAT
#define __NR_d_creat  DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_SYSCALL_LINK
#define __NR_d_link   DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_SYSCALL_UNLINK
#define __NR_d_unlink DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_SYSCALL_MKDIR
#define __NR_d_mkdir  DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_SYSCALL_RMDIR
#define __NR_d_rmdir  DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_SYSCALL_MKNOD
#define __NR_d_mknod  DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_SYSCALL_ACCESS
#define __NR_d_access DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_SYSCALL_ACCT
#define __NR_d_acct   DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_SYSCALL_ALARM
#define __NR_d_alarm  DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_SYSCALL_CHDIR
#define __NR_d_chdir  DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_SYSCALL_CHMOD
#define __NR_d_chmod  DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_SYSCALL_CHOWN
#define __NR_d_chown  DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_SYSCALL_UTIME
#define __NR_d_utime  DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_SYSCALL_CHROOT
#define __NR_d_chroot DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_SYSCALL_USTAT
#define __NR_d_ustat  DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_SYSCALL_FSTAT
#define __NR_d_fstat  DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_SYSCALL_STAT
#define __NR_d_stat   DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_SYSCALL_GETPID
#define __NR_d_getpid DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_SYSCALL_GETUID
#define __NR_d_getuid DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_SYSCALL_PAUSE
#define __NR_d_pause  DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_SYSCALL_NICE
#define __NR_d_nice   DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_SYSCALL_GETGID
#define __NR_d_getgid DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_SYSCALL_GETEUID
#define __NR_d_geteuid DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_SYSCALL_GETPPID
#define __NR_d_getppid DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_SYSCALL_TIME
#define __NR_d_time    DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_SYSCALL_STIME
#define __NR_d_stime   DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_SYSCALL_TIMES
#define __NR_d_times   DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_SYSCALL_FTIME
#define __NR_d_ftime   DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_SYSCALL_ULIMIT
#define __NR_d_ulimit  DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_SYSCALL_MPX
#define __NR_d_mpx     DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_SYSCALL_LOCK
#define __NR_d_lock    DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_SYSCALL_PHYS
#define __NR_d_phys    DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_SYSCALL_PROF
#define __NR_d_prof    DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_SYSCALL_GTTY
#define __NR_d_gtty    DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_SYSCALL_STTY
#define __NR_d_stty    DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_SYSCALL_PTRACE
#define __NR_d_ptrace  DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_SYSCALL_GETPGRP
#define __NR_d_getpgrp DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_SYSCALL_SETSID
#define __NR_d_setsid  DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_SYSCALL_UMASK
#define __NR_d_umask   DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_SYSCALL_UNAME
#define __NR_d_uname   DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_SYSCALL_SETPGID
#define __NR_d_setpgid DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_SYSCALL_SETGID
#define __NR_d_setgid  DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_SYSCALL_SETREGID
#define __NR_d_setregid  DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_SYSCALL_BRK
#define __NR_d_brk       DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_SYSCALL_SETUID
#define __NR_d_setuid    DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_SYSCALL_SETREUID
#define __NR_d_setreuid  DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_SYSCALL_BREAK
#define __NR_d_break     DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_SYSCALL_RENAME
#define __NR_d_rename    DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_SYSCALL_FCNTL
#define __NR_d_fcntl     DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_SYSCALL_DUP2
#define __NR_d_dup2      DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_SYSCALL_LSEEK
#define __NR_d_lseek     DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_SYSCALL_MOUNT
#define __NR_d_mount     DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_SYSCALL_UMOUNT
#define __NR_d_umount    DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_SYSCALL_SETUP
#define __NR_d_setup     DEBUG_SYSCALL_NR
#endif
#ifdef CONFIG_DEBUG_SYSCALL_SIGNAL
#define __NR_d_signal    DEBUG_SYSCALL_NR
#endif

#define _syscall0(type, name) \
	type name(void) \
{ \
    long __res; \
    __asm__ volatile ("int $0x80" \
                      : "=a" (__res) \
                      : "0"  (__NR_##name)); \
    if (__res >= 0) \
        return (type) __res; \
    errno = -__res; \
    return -1; \
}

#define _syscall1(type, name, atype, a) \
type name(atype a) \
{ \
long __res; \
__asm__ volatile ("int $0x80" \
        : "=a" (__res) \
        : "0" (__NR_##name), "b" ((long)(a))); \
if (__res >= 0) \
        return (type) __res; \
errno = -__res; \
return -1; \
}

#define _syscall2(type, name, atype, a, btype, b) \
type name(atype a, btype b) \
{ \
long __res; \
__asm__ volatile ("int $0x80" \
        : "=a" (__res) \
        : "0" (__NR_##name), "b" ((long)(a)), "c" ((long)(b))); \
if (__res >= 0) \
        return (type) __res; \
errno = -__res; \
return -1; \
}

#define _syscall3(type, name, atype, a, btype, b, ctype, c) \
type name(atype a, btype b, ctype c) \
{ \
long __res; \
__asm__ volatile ("int $0x80" \
        : "=a" (__res) \
        : "0" (__NR_##name),"b" ((long)(a)),"c" ((long)(b)),"d" ((long)(c))); \
if (__res >= 0) \
        return (type) __res; \
errno = -__res; \
return -1; \
}

#endif /* __LIBRARY__ */

extern int errno;

int access(const char *filename, mode_t mode);
int acct(const char *filename);
int alarm(int sec);
int brk(void *end_data_segment);
void *sbrk(ptrdiff_t increment);
int chdir(const char *filename);
int chmod(const char *filename, mode_t mode);
int chown(const char *filename, uid_t owner, gid_t group);
int chroot(const char *filename);
int close(int fildes);
int creat(const char *filename, mode_t mode);
int dup(int fildes);
int execve(const char *filename, char **argv, char **envp);
int execv(const char *pathname, char **argv);
int execvp(const char *file, char **argv);
int execl(const char *pathname, char *arg0, ...);
int execlp(const char *file, char *arg0, ...);
int execle(const char *pathname, char *arg0, ...);
void exit(int status);
void _exit(int status);
int fcntl(int fildes, int cmd, ...);
int fork(void);
int getpid(void);
int getuid(void);
int geteuid(void);
int getgid(void);
int getegid(void);
int ioctl(int fildes, int cmd, ...);
int kill(pid_t pid, int signal);
int link(const char *filename1, const char *filename2);
int lseek(int fildes, off_t offset, int origin);
int mknod(const char *filename, mode_t mode, dev_t dev);
int mount(const char *specialfile, const char *dir, int rwflag);
int nice(int val);
int open(const char *filename, int flag, ...);
int pause(void);
int pipe(int *fildes);
int read(int fildes, char *buf, off_t count);
int setpgrp(void);
int setpgid(pid_t pid, pid_t pgid);
int setuid(uid_t uid);
int setgid(gid_t gid);
void (*signal(int sig, void (*fn)(int)))(int);
int stat(const char *filename, struct stat *stat_buf);
int fstat(int fildes, struct stat *stat_buf);
int stime(time_t *tptr);
int sync(void);
time_t time(time_t *tloc);
time_t times(struct tms *tbuf);
int ulimit(int cmd, long limit);
mode_t umask(mode_t mask);
int umount(const char *specialfile);
int uname(struct utsname *name);
int unlink(const char *filename);
int ustat(dev_t dev, struct ustat *ubuf);
int utime(const char *filename, struct utimbuf *times);
pid_t waitpid(pid_t pid, int *wait_stat, int options);
pid_t wait(int *wait_stat);
int write(int fildes, const char *buf, off_t count);
int dup2(int oldfd, int newfd);
int getppid(void);
pid_t getpgrp(void);
pid_t setsid(void);
int sethostname(char *name, int len);
int setrlimit(int resource, struct rlimit *rlp);
int getrlimit(int resource, struct rlimit *rlp);
int getrusage(int who, struct rusage *rusage);
int gettimeofday(struct timeval *tv, struct timezone *tz);
int settimeofday(struct timeval *tv, struct timezone *tz);
int getgroups(int gidsetlen, gid_t *gidset);
int setgroups(int gidsetlen, gid_t *gidset);
int select(int width, fd_set * readfds, fd_set * writefds,
	fd_set * exceptfds, struct timeval * timeout);
#ifdef CONFIG_DEBUG_SYSCALL_OPEN0
int d_open(const char *filename, int flag, ...);
#endif
#ifdef CONFIG_DEBUG_SYSCALL_CLOSE0
int d_close(int fildes);
#endif
#ifdef CONFIG_DEBUG_SYSCALL_READ0
int d_read(int fildes, char *buf, off_t count);
#endif
#ifdef CONFIG_DEBUG_SYSCALL_WRITE0
int d_write(int fildes, const char *buf, off_t count);
#endif
#ifdef CONFIG_DEBUG_SYSCALL_DUP0
int d_dup(int fildes);
#endif
#ifdef CONFIG_DEBUG_SYSCALL_FORK0
int d_fork(void);
#endif
#ifdef CONFIG_DEBUG_SYSCALL_EXECVE0
int d_execve(const char *filename, char **argv, char **envp);
#endif
#ifdef CONFIG_DEBUG_BINARY_AOUT
int d_execve(const char *filename, char **argv, char **envp);
#endif
#ifdef CONFIG_DEBUG_BINARY_ELF
int d_parse_elf(const char *filename, char **argv, char **envp);
#endif
#ifdef CONFIG_DEBUG_SYSCALL_STACK
int d_stack(int fildes);
#endif
#ifdef CONFIG_DEBUG_SEGMENT_FS
int d_fs(const char *filename, char **argv, char *buffer);
#endif
#ifdef CONFIG_DEBUG_MMU
int d_mmu(const char *filename, char **argv, char *buffer);
#endif
#ifdef CONFIG_DEBUG_SYSCALL_CREAT
int d_creat(const char *filename, int mode);
#endif
#ifdef CONFIG_DEBUG_SYSCALL_LINK
int d_link(const char *filename1, const char *filename2);
#endif
#ifdef CONFIG_DEBUG_SYSCALL_UNLINK
int d_unlink(const char *filename1);
#endif
#ifdef CONFIG_DEBUG_SYSCALL_MKDIR
int d_mkdir(const char *filename, int mode);
#endif
#ifdef CONFIG_DEBUG_SYSCALL_RMDIR
int mkdir(const char *filename, int mode);
int d_rmdir(const char *filename);
#endif
#ifdef CONFIG_DEBUG_SYSCALL_MKNOD
int d_mknod(const char *filename, mode_t mode, dev_t dev);
#endif
#ifdef CONFIG_DEBUG_SYSCALL_ACCESS
int d_access(const char *filename, mode_t mode);
#endif
#ifdef CONFIG_DEBUG_SYSCALL_ACCT
int d_acct(const char *filename);
#endif
#ifdef CONFIG_DEBUG_SYSCALL_ALARM
int d_alarm(long seconds);
#endif
#ifdef CONFIG_DEBUG_SYSCALL_CHDIR
int d_chdir(const char *filename);
#endif
#ifdef CONFIG_DEBUG_SYSCALL_CHMOD
int d_chmod(const char *filename, int mode);
#endif
#ifdef CONFIG_DEBUG_SYSCALL_CHOWN
int d_chown(const char *filename, int uid, int gid);
#endif
#ifdef CONFIG_DEBUG_SYSCALL_UTIME
int d_utime(const char *filename, struct utimbuf *times);
#endif
#ifdef CONFIG_DEBUG_SYSCALL_CHROOT
int d_chroot(const char *filename);
#endif
#ifdef CONFIG_DEBUG_SYSCALL_USTAT
int d_ustat(int dev, struct ustat *ubuf);
#endif
#ifdef CONFIG_DEBUG_SYSCALL_FSTAT
int d_fstat(unsigned int fd, struct stat *statbuf);
#endif
#ifdef CONFIG_DEBUG_SYSCALL_STAT
int d_stat(unsigned int fd, struct stat *statbuf);
#endif
#ifdef CONFIG_DEBUG_SYSCALL_GETPID
int d_getpid(void);
#endif
#ifdef CONFIG_DEBUG_SYSCALL_GETUID
int d_getuid(void);
#endif
#ifdef CONFIG_DEBUG_SYSCALL_PAUSE
int d_pause(void);
#endif
#ifdef CONFIG_DEBUG_SYSCALL_NICE
int d_nice(long increment);
#endif
#ifdef CONFIG_DEBUG_SYSCALL_GETGID
int d_getgid(void);
#endif
#ifdef CONFIG_DEBUG_SYSCALL_GETEUID
int d_geteuid(void);
#endif
#ifdef CONFIG_DEBUG_SYSCALL_GETPPID
int d_getppid(void);
#endif
#ifdef CONFIG_DEBUG_SYSCALL_TIME
int d_time(long *tloc);
#endif
#ifdef CONFIG_DEBUG_SYSCALL_STIME
int d_stime(long *tptr);
#endif
#ifdef CONFIG_DEBUG_SYSCALL_TIMES
int d_times(struct tms *tbuf);
#endif
#ifdef CONFIG_DEBUG_SYSCALL_FTIME
int d_ftime(void);
#endif
#ifdef CONFIG_DEBUG_SYSCALL_ULIMIT
int d_ulimit(void);
#endif
#ifdef CONFIG_DEBUG_SYSCALL_MPX
int d_mpx(void);
#endif
#ifdef CONFIG_DEBUG_SYSCALL_LOCK
int d_lock(void);
#endif
#ifdef CONFIG_DEBUG_SYSCALL_PHYS
int d_phys(void);
#endif
#ifdef CONFIG_DEBUG_SYSCALL_PROF
int d_prof(void);
#endif
#ifdef CONFIG_DEBUG_SYSCALL_GTTY
int d_gtty(void);
#endif
#ifdef CONFIG_DEBUG_SYSCALL_STTY
int d_stty(void);
#endif
#ifdef CONFIG_DEBUG_SYSCALL_PTRACE
int d_ptrace(void);
#endif
#ifdef CONFIG_DEBUG_SYSCALL_GETPGRP
int d_getpgrp(void);
#endif
#ifdef CONFIG_DEBUG_SYSCALL_SETSID
int d_setsid(void);
#endif
#ifdef CONFIG_DEBUG_SYSCALL_UMASK
int d_umask(int umask);
#endif
#ifdef CONFIG_DEBUG_SYSCALL_UNAME
int d_uname(struct utsname *name);
#endif
#ifdef CONFIG_DEBUG_SYSCALL_SETPGID
int d_setpgid(int pid, int pgid);
#endif
#ifdef CONFIG_DEBUG_SYSCALL_SETGID
int d_setgid(int gid);
#endif
#ifdef CONFIG_DEBUG_SYSCALL_SETREGID
int d_setregid(int gid, int egid);
#endif
#ifdef CONFIG_DEBUG_SYSCALL_BRK
int d_brk(unsigned long end_data_seg);
#endif
#ifdef CONFIG_DEBUG_SYSCALL_SETUID
int d_setuid(int uid);
#endif
#ifdef CONFIG_DEBUG_SYSCALL_SETREUID
int d_setreuid(int ruid, int euid);
#endif
#ifdef CONFIG_DEBUG_SYSCALL_BREAK
int d_break(void);
#endif
#ifdef CONFIG_DEBUG_SYSCALL_RENAME
int d_rename(void);
#endif
#ifdef CONFIG_DEBUG_SYSCALL_FCNTL
int d_fcntl(unsigned int fd, unsigned int cmd, unsigned long arg);
#endif
#ifdef CONFIG_DEBUG_SYSCALL_DUP2
int d_dup2(unsigned int oldfd, unsigned int newfd);
#endif
#ifdef CONFIG_DEBUG_SYSCALL_LSEEK
int d_lseek(unsigned int fd, off_t offset, int origin);
#endif
#ifdef CONFIG_DEBUG_SYSCALL_MOUNT
int d_mount(void);
#endif
#ifdef CONFIG_DEBUG_SYSCALL_UMOUNT
int d_umount(void);
#endif
#ifdef CONFIG_DEBUG_SYSCALL_SETUP
int d_setup(void);
#endif
#ifdef CONFIG_DEBUG_SYSCALL_SIGNAL
int d_signal(int signum, long handler, long restorer);
#endif

#endif
