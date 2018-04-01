/*
 * Testcae main entry
 *
 * (C) BiscuitOS 2017.10 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <test/debug.h>

#ifdef CONFIG_DEBUG_USERLAND_SYSCALL
/*
 * Note! Broken the number of system call on linux 0.11, we should modify
 * 'nr_system_calls' on kernel/system_call.c. Default value is 72, so the 
 * number of system call over it will directly return -1.
 */
#define __LIBRARY__
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>

static char printbuf[1024];

extern int vsprintf(char *buf, const char *fmt, va_list args);
#endif

#ifdef CONFIG_DEBUG_KERNEL_LATER
/*
 * debug on kernel last before userland launch.
 */
void debug_on_kernel_later(void)
{
#ifdef CONFIG_DEBUG_INTERRUPT
    debug_interrupt_common();
#endif

#ifdef CONFIG_DEBUG_SCHED
    debug_task_scheduler_common();
#endif

#ifdef CONFIG_DEBUG_MMU
    debug_mmu_common();
#endif

#ifdef CONFIG_DEBUG_SYSCALL
    debug_syscall_common();
#endif

#ifdef CONFIG_DEBUG_VFS
    debug_vfs_common();
#endif

#ifdef CONFIG_DEBUG_BLOCK_DEV
    debug_block_common();
#endif

#ifdef CONFIG_DEBUG_BINARY
    debug_binary_common();
#endif

#ifdef CONFIG_DEBUG_SEGMENT
    debug_segment_common();
#endif
}
#endif // CONFIG_DEBUG_KERNEL_LATER

#ifdef CONFIG_DEBUG_KERNEL_EARLY
/*
 * debug on kernel early
 */
void debug_on_kernel_early(void)
{

}
#endif // CONFIG_DEBUG_KERNEL_EARLY

#ifdef CONFIG_DEBUG_USERLAND
/* debug kernel on userland */
int debug_kernel_on_userland(void)
{
    printk("Debug kernel on userland :-)\n");
#ifdef CONFIG_DEBUG_VFS
    debug_vfs_common_userland();
#endif
    return 0;
}
#endif // CONFIG_DEBUG_USERLAND

#ifdef CONFIG_DEBUG_USERLAND_SYSCALL
/******* Userland *********/
int d_printf(const char *fmt, ...)
{
    va_list args;
    int i;

    va_start(args, fmt);
    write(1, printbuf, i = vsprintf(printbuf, fmt, args));
    va_end(args);
    return i;
}
/*
 * Debug on userland.
 */
int debug_on_userland_syscall(void)
{
#ifdef CONFIG_DEBUG_SYSCALL
    debug_syscall_common_userland();
#endif

#ifdef CONFIG_DEBUG_BINARY
    debug_binary_common_userland();
#endif

#ifdef CONFIG_DEBUG_SEGMENT
    debug_segment_common_userland();
#endif

#ifdef CONFIG_DEBUG_MMU
    debug_mmu_common_userland();
#endif
    return 0;
}
#endif // CONFIG_DEBUG_USERLAND_SYSCALL
