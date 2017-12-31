#ifndef _DEBUG_SYSCALL_H
#define _DEBUG_SYSCALL_H

#ifdef CONFIG_TESTCASE_SYSCALL
extern int common_system_call_entry(void);
#endif

#ifdef CONFIG_TESTCASE_SYSCALL_ROUTINE
extern void common_system_call_rountine(void);
#endif

#endif
