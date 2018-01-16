#ifndef _TESTCASE_DEBUG_H_
#define _TESTCASE_DEBUG_H_

#ifdef CONFIG_DEBUG_SCHED
#include <test/task.h>
#endif

#ifdef CONFIG_DEBUG_INTERRUPT
#include <test/interrupt.h>
#endif

#ifdef CONFIG_DEBUG_MMU
#include <test/mm.h>
#endif

#ifdef CONFIG_TESTCASE_TIMER
#include <test/timer.h>
#endif

#ifdef CONFIG_TESTCASE_IOPORT
#include <test/ioports.h>
#endif

#ifdef CONFIG_DEBUG_SYSCALL
#include <test/syscall.h>
#endif

#ifdef CONFIG_DEBUG_VFS
#include <test/vfs.h>
#endif

#ifdef CONFIG_DEBUG_KERNEL_LATER
extern void debug_on_kernel_later(void);
#endif

#ifdef CONFIG_DEBUG_KERNEL_EARLY
extern void debug_on_kernel_early(void);
#endif

#ifdef CONFIG_DEBUG_USERLAND_EARLY
extern void debug_on_userland_early(void);
#endif

#ifdef CONFIG_DEBUG_USERLAND_SHELL
extern void debug_on_shell_stage(void);
#endif

#if defined (CONFIG_DEBUG_USERLAND_EARLY) || \
    defined (CONFIG_DEBUG_USERLAND_SHELL)
extern void debug_kernel_on_userland_stage(void);
#endif

#endif
