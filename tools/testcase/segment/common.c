/*
 * Segment mechanism: the cluster of segment.
 *
 * (C) 2018.03 BiscuitOS <buddy.zhang@aliyun.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <test/debug.h>

int debug_segment_common(void)
{
    return 0;
}

/* debug on userland */
int debug_segment_common_userland(void)
{
#ifdef CONFIG_DEBUG_SEGMENT_FS
    debug_segment_fs_common_userland();
#endif
    return 0;
}
