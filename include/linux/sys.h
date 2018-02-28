extern int sys_setup(void);
extern int sys_exit(void);
extern int sys_fork(void);
extern int sys_read(void);
extern int sys_write(void);
extern int sys_open(void);
extern int sys_close(void);
extern int sys_waitpid(void);
extern int sys_creat(void);
extern int sys_link(void);
extern int sys_unlink(void);
extern int sys_execve(void);
extern int sys_chdir(void);
extern int sys_time(void);
extern int sys_mknod(void);
extern int sys_chmod(void);
extern int sys_chown(void);
extern int sys_break(void);
extern int sys_stat(void);
extern int sys_lseek(void);
extern int sys_getpid(void);
extern int sys_mount(void);
extern int sys_umount(void);
extern int sys_setuid(void);
extern int sys_getuid(void);
extern int sys_stime(void);
extern int sys_ptrace(void);
extern int sys_alarm();
extern int sys_fstat(void);
extern int sys_pause(void);
extern int sys_utime(void);
extern int sys_stty(void);
extern int sys_gtty(void);
extern int sys_access(void);
extern int sys_nice();
extern int sys_ftime(void);
extern int sys_sync(void);
extern int sys_kill(void);
extern int sys_rename(void);
extern int sys_mkdir(void);
extern int sys_rmdir(void);
extern int sys_dup(void);
extern int sys_pipe(void);
extern int sys_times(void);
extern int sys_prof(void);
extern int sys_brk(void);
extern int sys_setgid(void);
extern int sys_getgid(void);
extern int sys_signal(void);
extern int sys_geteuid(void);
extern int sys_getegid(void);
extern int sys_acct(void);
extern int sys_phys(void);
extern int sys_lock(void);
extern int sys_ioctl(void);
extern int sys_fcntl(void);
extern int sys_mpx(void);
extern int sys_setpgid(void);
extern int sys_ulimit(void);
extern int sys_uname(void);
extern int sys_umask(void);
extern int sys_chroot(void);
extern int sys_ustat(void);
extern int sys_dup2(void);
extern int sys_getppid(void);
extern int sys_getpgrp(void);
extern int sys_setsid(void);
extern int sys_sigaction(void);
extern int sys_sgetmask(void);
extern int sys_ssetmask(void);
extern int sys_setreuid(void);
extern int sys_setregid(void);

fn_ptr sys_call_table[] = {
sys_setup, /* system setup */
sys_exit,  /* terminate the current process */
sys_fork,  /* create a child process */
sys_read,  /* read from a file descriptor */
sys_write, /* write to a file descriptor */
sys_open, /* open and possibly create a file */
sys_close, /* closes a file on the host system */
sys_waitpid, /* wait for process termination */
sys_creat, /* create a file or device */
sys_link, /* make a new name for a file */
sys_unlink, /* delete a name and possibly the file it refers to */
sys_execve, /* execute program */
sys_chdir, /* change working directory */
sys_time, /* get time in seconds */
sys_mknod, /* create a directory or special or ordinary file */
sys_chmod, /* change permissions of a file */
sys_chown, /* change ownership of a file */
sys_break, /* call exists only for compatibility */
sys_stat, /* --- */
sys_lseek, /* reposition read/write file offset */
sys_getpid, /* get process identification */
sys_mount, /* mount filesystems */
sys_umount, /* unmount filesystem */
sys_setuid, /* set user identity */
sys_getuid, /* get user identity */
sys_stime, /* set time */
sys_ptrace, /* process trace */
sys_alarm, /* set an alarm clock for delivery of a signal */
sys_fstat, /* */
sys_pause, /* wait for signal */
sys_utime, /* change access and/or modification times of an inode */
sys_stty, /* set mode of typewriter */
sys_gtty, /* get typewriter status */
sys_access, /* check user's permissions for a file */
sys_nice, /* change process priority */
sys_ftime, /* --- */
sys_sync, /* write buffer into disk */
sys_kill, /* send a signal to a process  */
sys_rename, /* Renames a specified file */
sys_mkdir, /* Create a directory */
sys_rmdir, /* Remove a directory */
sys_dup, /* duplicate an open file descriptor */
sys_pipe, /* create an interprocess channel */
sys_times, /* file access and modification times structure */
sys_prof, /* profiling library */
sys_brk, /* allocates memory right behind application image in memory */
sys_setgid, /* Set the numerical group id */
sys_getgid, /* Get the numerical group id */
sys_signal, /* signal handling */
sys_geteuid, /* get current task euid */
sys_getegid, /* get current task egid */
sys_acct, /* enable/disable process accounting */
sys_phys, /* --- */
sys_lock, /* --- */
sys_ioctl, /* control device */
sys_fcntl, /* manipulate file descriptor */
sys_mpx, /* -- */
sys_setpgid, /* simplify pid/ns interaction */
sys_ulimit, /* -- */
sys_uname, /* get name and information about current kernel */
sys_umask, /* get current task umask */
sys_chroot, /* change root directory */
sys_ustat, /* -- */
sys_dup2, /* duplicate a file descriptor */
sys_getppid, /* get current father pid */
sys_getpgrp, /* get pgrp of current task */
sys_setsid, /* creates a session and sets the process group ID */
sys_sigaction, /* examine and change a signal action */
sys_sgetmask, /* manipulation of signal mask */
sys_ssetmask, /* -- */
sys_setreuid, /* set real and/or effective user or group ID */
sys_setregid, /* Set the numerical group id */
};
