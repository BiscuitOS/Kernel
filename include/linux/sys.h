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
};
