/*
 * ESI: Pointer to data in the segment pointed to by the DS register
 *      or source pointer for string operation.
 *
 * (C) 2018.08.10 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <demo/debug.h>

static int debug_esi(void)
{
    return 0;
}
late_debugcall(debug_esi);
