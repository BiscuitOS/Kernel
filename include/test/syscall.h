#ifndef _DEBUG_SYSCALL_H
#define _DEBUG_SYSCALL_H

#ifdef CONFIG_DEBUG_SYSCALL
extern int debug_syscall_common(void);
#endif

#ifdef CONFIG_DEBUG_SYSCALL_ROUTINE
extern void system_call_rountine(void);
#endif

#endif
