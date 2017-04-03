# ----------------------------------------------------------------------
# Global Makefile
# Maintainer: Buddy <buddy.zhang@aliyun.com>
#
# Copyright (C) 2017 BiscuitOS
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.

VERSION = 1
NAME = BiscuitOS

DEBUG := 1
export DEBUG
# Beautify output
ifeq ("$(origin V)", "command line")
  KBUILD_VERBOSE = $(V)
endif
ifndef KBUILD_VERBOSE
  KBUILD_VERBOSE = 0
endif

ifeq ($(KBUILD_VERBOSE), 1)
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
ARCH    := i386
CROSS_COMPILE :=

AS      = $(CROSS_COMPILE)as
LD      = $(CROSS_COMPILE)ld
CC      = $(CROSS_COMPILE)gcc

# Compile flags
ASFLAGS  = 
CFLAGS  = 

ifeq ($(ARCH), i386)
  ASFLAGS += --32
endif

ifneq ($(DEBUG),)
  ASFLAGS += -gstabs -g -g3 -gdwarf-2
endif

export VERSION NAME
export ARCH CROSS_COMPILE AS LD CC
export ASFLAGS CFLAGS

# Subdir 
SUBDIR += boot arch

export SUBDIR

# General Rule
.c.s:
	$(Q)$(CC) $(CFLAGS) -S -o $*.s $<
.s.o:
	$(Q)$(AS) $(ASFLAGS) -o $*.o $<


# To do compile
all: CHECK_START $(SUBDIR) Image 

CHECK_START:

$(SUBDIR): ECHO 
	$(Q)make -s -C $@ 

ECHO:

Image: 
	$(Q)make -s -C $(srctree)/tools/build 

start:
	$(Q)make -s -C $(srctree)/tools/build start 

debug:
	$(Q)make -s -C $(srctree)/tools/build debug 

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
	
