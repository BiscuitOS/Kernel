/*
 * Kernel virtual space on i386
 *
 * Copyright (C) 2018.12.16 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.
 */
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>

/* Initialization entence */
static __init int kern_space_init(void)
{
    /* Must return 0 for here! */
    return 0;
}

/* Exit entence */
static __exit void kern_space_exit(void)
{
}

/* Module information */
module_init(kern_space_init);
module_exit(kern_space_exit);

MODULE_LICENSE("GPL v2");
