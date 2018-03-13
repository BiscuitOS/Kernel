#ifndef _DEBUG_SYSCALL_H
#define _DEBUG_SYSCALL_H

#ifdef CONFIG_DEBUG_SYSCALL
extern int debug_syscall_common(void);
extern int debug_syscall_common_userland(void);

extern int d_printf(const char *fmt, ...);


#ifdef CONFIG_DEBUG_SYSCALL_ROUTINE
extern void system_call_rountine(void);
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

#endif // CONFIG_DEBUG_SYSCALL

#endif
