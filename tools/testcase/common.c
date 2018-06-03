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

extern debugcall_t debugcallearly_start[];
extern debugcall_t debugcall0_start[];
extern debugcall_t debugcall1_start[];
extern debugcall_t debugcall1s_start[];
extern debugcall_t debugcall2_start[];
extern debugcall_t debugcall2s_start[];
extern debugcall_t debugcall3_start[];
extern debugcall_t debugcall3s_start[];
extern debugcall_t debugcall4_start[];
extern debugcall_t debugcall4s_start[];
extern debugcall_t debugcall5_start[];
extern debugcall_t debugcall5s_start[];
extern debugcall_t debugcallrootfs_start[];
extern debugcall_t debugcall6_start[];
extern debugcall_t debugcall6s_start[];
extern debugcall_t debugcall7_start[];
extern debugcall_t debugcall7s_start[];

static debugcall_t *debugcall_levels[] = {
    debugcallearly_start,
    debugcall0_start,
    debugcall1_start,
    debugcall1s_start,
    debugcall2_start,
    debugcall2s_start,
    debugcall3_start,
    debugcall3s_start,
    debugcall4_start,
    debugcall4s_start,
    debugcall5_start,
    debugcall5s_start,
    debugcallrootfs_start,
    debugcall6_start,
    debugcall6s_start,
    debugcall7_start,
    debugcall7s_start,
};

void do_debugcall(int levels)
{
    debugcall_t *fn;

    for (fn = debugcall_levels[levels]; fn < debugcall_levels[levels + 1]; fn++)
        debugcall(*fn);
}

/* Empty debugcall */
static int __debug_empty_early(void)
{
    return 0;
}
early_debugcall(__debug_empty_early);

static int __debug_empty_pure(void)
{
    return 0;
}
pure_debugcall(__debug_empty_pure);

static int __debug_empty_core(void)
{
    return 0;
}
core_debugcall(__debug_empty_core);

static int __debug_empty_core_sync(void)
{
    return 0;
}
core_debugcall_sync(__debug_empty_core_sync);

static int __debug_empty_postcore(void)
{
    return 0;
}
postcore_debugcall(__debug_empty_postcore);

static int __debug_empty_postcore_sync(void)
{
    return 0;
}
postcore_debugcall_sync(__debug_empty_postcore_sync);

static int __debug_empty_arch(void)
{
    return 0;
}
arch_debugcall(__debug_empty_arch);

static int __debug_empty_arch_sync(void)
{
    return 0;
}
arch_debugcall_sync(__debug_empty_arch_sync);

static int __debug_empty_subsys(void)
{
    return 0;
}
subsys_debugcall(__debug_empty_subsys);

static int __debug_empty_subsys_sync(void)
{
    return 0;
}
subsys_debugcall_sync(__debug_empty_subsys_sync);

static int __debug_empty_fs(void)
{
    return 0;
}
fs_debugcall(__debug_empty_fs);

static int __debug_empty_fs_sync(void)
{
    return 0;
}
fs_debugcall_sync(__debug_empty_fs_sync);

static int __debug_empty_rootfs(void)
{
    return 0;
}
rootfs_debugcall(__debug_empty_rootfs);

static int __debug_empty_device(void)
{
    return 0;
}
device_debugcall(__debug_empty_device);

static int __debug_empty_device_sync(void)
{
    return 0;
}
device_debugcall_sync(__debug_empty_device_sync);

static int __debug_empty_late(void)
{
    return 0;
}
late_debugcall(__debug_empty_late);

static int __debug_empty_late_sync(void)
{
    return 0;
} 
late_debugcall_sync(__debug_empty_late_sync);
