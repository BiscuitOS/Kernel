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
#include <demo/debug.h>

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
/* Userspace */
extern debugcall_t debugcall8_start[];
extern debugcall_t debugcall8s_start[];
extern debugcall_t debugcall9_start[];
extern debugcall_t debugcall9s_start[];
extern debugcall_t debugcalla_start[];
extern debugcall_t debugcallas_start[];

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
    debugcall8_start,
    debugcall8s_start,
    debugcall9_start,
    debugcall9s_start,
    debugcalla_start,
    debugcallas_start,
};

void do_debugcall(int levels)
{
    debugcall_t *fn;

    for (fn = debugcall_levels[levels]; fn < debugcall_levels[levels + 1]; fn++)
        debugcall(*fn);
}
