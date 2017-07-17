extern int sys_setup();
extern int sys_exit();

fn_ptr sys_call_table[] = { sys_setup, sys_exit };
