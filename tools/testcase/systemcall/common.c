/*
 * Common System Call
 *
 * (C) 2018.03 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <test/debug.h>

int debug_syscall_common(void)
{
#ifdef CONFIG_DEBUG_SYSCALL_ROUTINE
    system_call_rountine();
#endif
    return 0;
}

/* Debug on userland */
int debug_syscall_common_userland(void)
{
#ifdef CONFIG_DEBUG_SYSCALL_OPEN
    debug_syscall_open_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_CLOSE
    debug_syscall_close_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_READ
    debug_syscall_read_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_WRITE
    debug_syscall_write_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_DUP
    debug_syscall_dup_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_EXECVE
    debug_syscall_execve_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_FORK
    debug_syscall_fork_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_EXECVE
    debug_syscall_execve_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_CREAT
    debug_syscall_creat_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_STACK
    debug_syscall_stack();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_LINK
    debug_syscall_link_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_UNLINK
    debug_syscall_unlink_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_MKDIR
    debug_syscall_mkdir_common_userland();
#endif

#ifdef CONFIG_DEBUG_SYSCALL_RMDIR
    debug_syscall_rmdir_common_userland();
#endif

    return 0;
}
