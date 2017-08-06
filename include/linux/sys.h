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
};
