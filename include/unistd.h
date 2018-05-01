#ifndef _UNISTD_H_
#define _UNISTD_H_

/* ok, this may be a joke, but I'm working on it */
#define _POSIX_VERSION 198808L

#define _POSIX_CHOWN_RESTRICTED	1    /* only root can do a chown (I think..) */
#define _POSIX_NO_TRUNC		1    /* no pathname truncation (but see kernel) */
#define _POSIX_VDISABLE		'\0' /* character to disable things like ^C */
#define _POSIX_JOB_CONTROL	1
#define _POSIX_SAVED_IDS	1    /* Implemented, for whatever good it is */

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

#include <linux/unistd.h>
#ifdef __LIBRARY__
#endif

/* XXX - illegal. */
extern int errno;

/* XXX - several non-POSIX functions here, and POSIX functions that are
 * supposed to be declared elsewhere.  Non-promotion of short types in
 * prototypes may cause trouble.  Arg names should be prefixed by
 * underscores.
 */
int access(const char *filename, mode_t mode); 	/* XXX - short type */
int acct(const char *filename);
int alarm(int sec);
/* XXX - POSIX says unsigned alarm(unsigned sec) */
int brk(void *end_data_segment);
void *sbrk(ptrdiff_t increment);
int chdir(const char *filename);
int chmod(const char *filename, mode_t mode); /* XXX - short type */
int chown(const char *filename, uid_t owner, gid_t group);  /* XXX - shorts */
int chroot(const char *filename);
int close(int fildes);
int creat(const char *filename, mode_t mode); /* XXX - short type */
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
pid_t fork(void);
pid_t getpid(void);
uid_t getuid(void);
uid_t geteuid(void);
gid_t getgid(void);
gid_t getegid(void);
int ioctl(int fildes, int cmd, ...);
int kill(pid_t pid, int signal);
int link(const char *filename1, const char *filename2);
off_t lseek(int fildes, off_t offset, int origin);
int mknod(const char *filename, mode_t mode, dev_t dev);  /* XXX - shorts */
int mount(const char *specialfile, const char *dir, const char *type, int rwflag);
int nice(int val);
int open(const char *filename, int flag, ...);
int pause(void);
int pipe(int *fildes);
int read(int fildes, char *buf, off_t count);
int setpgrp(void);
int setpgid(pid_t pid,pid_t pgid);	/* XXX - short types */
int setuid(uid_t uid);		/* XXX - short type */
int setgid(gid_t gid);		/* XXX - short type */
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
/* XXX**2 - POSIX says unsigned count */
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
int swapon(const char * specialfile);
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
