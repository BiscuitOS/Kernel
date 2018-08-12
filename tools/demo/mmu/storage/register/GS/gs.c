/*
 * GS: Data selector Register for sharing with another program.
 *
 * (C) 2018.08.12 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>

#include <demo/debug.h>

static int debug_gs(void)
{
    return 0;
}
late_debugcall(debug_gs);
