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

#endif // CONFIG_DEBUG_SYSCALL

#endif
