# ----------------------------------------------------------------------
# Global Makefile
# Maintainer: Buddy <buddy.zhang@aliyun.com>
#
# Copyright (C) 2017 BiscuitOS
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.

VERSION = 0.0.2
NAME = BiscuitOS

DEBUG := 1
VERSION_TEST := 1
export DEBUG VERSION_TEST
VERSION_FILE = $(srctree)/include/version.h
# Beautify output
ifeq ("$(origin V)", "command line")
  KBUILD_VERBOSE = $(V)
endif
ifndef KBUILD_VERBOSE
  KBUILD_VERBOSE = 0
endif

ifeq ($(KBUILD_VERBOSE),)
  Q =
else
  Q = @
endif
export Q

# Set PATH
srctree := $(CURDIR)
objtree := $(CURDIR)
src     := $(CURDIR)
obj     := $(CURDIR)

export srctree objtree

# Machine and compiler
ARCH    := x86
CROSS_COMPILE :=

AS      = $(CROSS_COMPILE)as
LD      = $(CROSS_COMPILE)ld
CC      = $(CROSS_COMPILE)gcc
OBJCOPY = $(CROSS_COMPILE)objcopy
NM      = $(CROSS_COMPILE)nm
STRIP   = $(CROSS_COMPILE)strip
AR      = $(CROSS_COMPILE)ar
CPP     = $(CROSS_COMPILE)cpp

# Compile flags
ASFLAGS  =
CFLAGS   =
LDFLAGS  =
CPPFLAGS =
EXTRA_CFLAGS =

ifeq ($(ARCH), x86)
  ASFLAGS += --32
  LDFLAGS += -m elf_i386 --traditional-format
  CFLAGS  += -m32 -fno-stack-protector -fgnu89-inline -fomit-frame-pointer
  CFLAGS  += -fno-builtin
endif

ifneq ($(DEBUG),)
  ASFLAGS += -ggdb -am
  # Debug option
  CFLAGS  += -g
  # Highest level warning
  CFLAGS  +=  -Wall -Wunused
endif

# Header
CFLAGS += -I$(srctree)/include
LDFLAGS += -Ttext 0
CPPFLAGS += -I$(srctree)/include -nostdinc

# Emulate Kbuild
-include $(srctree)/.config
CFLAGS += $(EXTRA_CFLAGS)

export VERSION NAME
export ARCH CROSS_COMPILE AS LD CC OBJCOPY NM AR
export STRIP
export ASFLAGS CFLAGS LDFLAGS EXTRA_FLAGS
export CPP CPPFLAGS

# Subdir
SUBDIR += init lib drivers kernel fs mm kernel/math
SUBDIR += $(srctree)/arch/$(ARCH)/boot

ifneq ($(VERSION_TEST),)
	SUBDIR += tools/test
endif

ARCHIVES := $(srctree)/kernel/kernel.o
DRIVERS  := $(srctree)/drivers/drivers.o
LIBS     := $(srctree)/lib/lib.o
MATH     := $(srctree)/kernel/math/math.o
FS       := $(srctree)/fs/fs.o
MM       := $(srctree)/mm/mm.o

ifneq ($(VERSION_TEST),)
TESTCODE := $(srctree)/tools/test/testcode.o
endif

IMAGE_PACKAGE := $(ARCHIVES) $(DRIVERS) $(LIBS) $(MATH) $(FS) $(MM)

ifneq ($(VERSION_TEST),)
IMAGE_PACKAGE += $(TESTCODE)
endif

export SUBDIR IMAGE_PACKAGE

# General Rule
.c.o:
	$(Q)$(CC) $(CFLAGS) -c -o $*.o $<
.s.o:
	$(Q)$(AS) $(ASFLAGS) -o $*.o $<


# To do compile
all: CHECK_START version $(SUBDIR) Image config
	$(Q)figlet "BiscuitOS"

CHECK_START:

$(SUBDIR): ECHO
	$(Q)make -s -C $@

ECHO:

config:
	$(Q)if [ ! -f $(srctree)/.config ]; then \
		cp $(srctree)/arch/x86/configs/BiscuitOS_defconfig \
			$(srctree)/.config; \
		fi

Image:
	$(Q)make -s -C $(srctree)/tools/build

start:
	$(Q)figlet "BiscuitOS"
	$(Q)make -s -C $(srctree)/tools/build start
	$(Q)figlet "BiscuitOS"

debug:
	$(Q)make -s -C $(srctree)/tools/build debug

version:
	@echo -n "#define KERNEL_VERSION \"Kernel Version: " > $(VERSION_FILE); \
	echo -n $(shell $(SHELL) $(srctree)/scripts/setlocalversion \
		 $(TOPDIR)) >> $(VERSION_FILE); \
	echo "\"" >> $(VERSION_FILE)

draw:
	$(Q)$(srctree)/tools/callgraph/call.sh $(srctree) $(FUN)

help:
	@echo "<<< BiscuitOS Help >>>"
	@echo ""
	@echo "Usage:"
	@echo "	make       -- Generate a kernel Image"
	@echo "	make start -- Start the kernel on qemu"
	@echo "	make debug -- Debug the kernel in qemu & gdb at port 1234"
	@echo "	make clean -- Clean the object files"

.PHONY: clean
clean: $(SUBDIR)
	$(Q)for i in $(SUBDIR); do make clean -s -C $$i; done
	$(Q)make clean -s -C $(srctree)/tools/build
	$(Q)rm -rf $(srctree)/tools/callgraph/*.svg
	$(Q)rm -rf $(srctree)/include/version.h

