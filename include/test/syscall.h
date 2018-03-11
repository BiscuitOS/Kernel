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

#endif // CONFIG_DEBUG_SYSCALL

#endif
