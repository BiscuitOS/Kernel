#
# arch/x86/Makefile
#
# This file is include by the global makefile so that you can add your own
# architecture-specific flags and dependencies.
#
# This file is subject to the terms and conditions of the GNU General Public
# License. See the file "COPYING" in the main directory of this achive
# for more details.
#
# Copyright (C) 2017- by Buddy <buddy.zhang@aliyun.com>


KBUILD_DEFCONFIG := defconfig

# ==========================================
# Specify compile flags

-include include/config/auto.conf

ifdef CONFIG_DEBUG_KERNEL
  KBUILD_CFLAGS += $(call cc-option,-g -Wall -Wunused)
  KBUILD_AFLAGS_KERNEL += -ggdb  -am
endif

KBUILD_AFLAGS_KERNEL += --32
KBUILD_CFLAGS += $(call cc-option,-m32 -fno-stack-protector -fgnu89-inline)
KBUILD_CFLAGS += $(call cc-option,-fomit-frame-pointer -fno-builtin)
LDFLAGS += -m elf_i386 --traditional-format
CPPFLAGS += -I$(srctree)/include -nostdinc
