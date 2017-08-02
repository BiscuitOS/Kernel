extern int sys_setup();
extern int sys_exit();
extern int sys_fork();
extern int sys_read();
extern int sys_write();

fn_ptr sys_call_table[] = {
sys_setup, /* system setup */
sys_exit,  /* terminate the current process */
sys_fork,  /* create a child process */
sys_read,  /* read from a file descriptor */
sys_write, /* write to a file descriptor */
};
