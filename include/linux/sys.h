extern int sys_setup();
extern int sys_exit();
extern int sys_fork();
extern int sys_read();
extern int sys_write();
extern int sys_open();
extern int sys_close();
extern int sys_waitpid();
extern int sys_creat();
extern int sys_link();
extern int sys_unlink();
extern int sys_execve();
extern int sys_chdir();
extern int sys_time();
extern int sys_mknod();
extern int sys_chmod();
extern int sys_chown();
extern int sys_break();
extern int sys_stat();
extern int sys_lseek();
extern int sys_getpid();
extern int sys_mount();
extern int sys_umount();
extern int sys_setuid();
extern int sys_getuid();
extern int sys_stime();
extern int sys_ptrace();
extern int sys_alarm();
extern int sys_fstat();
extern int sys_pause();
extern int sys_utime();
extern int sys_stty();
extern int sys_gtty();
extern int sys_access();
extern int sys_nice();
extern int sys_ftime();
extern int sys_sync();
extern int sys_kill();
extern int sys_rename();
extern int sys_mkdir();
extern int sys_rmdir();
extern int sys_dup();
extern int sys_pipe();
extern int sys_times();
extern int sys_prof();
extern int sys_brk();
extern int sys_setgid();
extern int sys_getgid();
extern int sys_signal();
extern int sys_geteuid();
extern int sys_getegid();
extern int sys_acct();
extern int sys_phys();
extern int sys_lock();
extern int sys_ioctl();
extern int sys_fcntl();
extern int sys_mpx();
extern int sys_setpgid();
extern int sys_ulimit();
extern int sys_uname();
extern int sys_umask();
extern int sys_setreuid();
extern int sys_setregid();

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
sys_setreuid, /* set real and/or effective user or group ID */
sys_setregid, /* Set the numerical group id */
};
