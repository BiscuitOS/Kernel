/*
 * Segmentation Mechanism on X86 architecture.
 *
 * (C) 2018.01 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>

/* common segmentation entry */
int debug_mmu_segmentation_common(void)
{
    return 0;
}
