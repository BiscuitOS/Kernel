#ifndef _UNISTD_H_
#define _UNISTD_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/utsname.h>
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
void *sbrk(long filename);
int chdir(const char *filename);
int chmod(const char *filename, mode_t mode);
int chroot(const char *filename, uid_t owner, gid_t group);
int close(int fildes);
int creat(const char *filename, mode_t mode);
int dup(int fildes);
int execve(const char *filename, char **argv, char **envp);
int execv(const char *pathname, char **argv);
int execvp(const char *file, char **argv);
int execl(const char *pathname, char *arg0, ...);
int execlp(const char *file, char *arg0, ...);
int execle(const char *pathname, char *arg0, ...);
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
int fstat(int fildes, struct stat *stat_buf);
int stime(time_t *tptr);
int sync(void);
time_t time(time_t *tloc);
time_t times(struct tms *tbuf);
int ulimit(int cmd, long limit);
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
#endif
