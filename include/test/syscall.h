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

#endif // CONFIG_DEBUG_SYSCALL

#endif
