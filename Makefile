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
OBJCOPY = $(CROSS_COMPILE)objcopy
NM      = $(CROSS_COMPILE)nm
STRIP   = $(CROSS_COMPILE)strip
AR      = $(CROSS_COMPILE)ar

# Compile flags
ASFLAGS  = 
CFLAGS   =
LDFLAGS  =

ifeq ($(ARCH), i386)
  ASFLAGS += --32
  LDFLAGS += -m elf_i386 --traditional-format
  CFLAGS  += -m32 -fno-stack-protector -fgnu89-inline -g  
endif

ifneq ($(DEBUG),)
  ASFLAGS += -ggdb -am 
endif

# Header
CFLAGS += -I$(srctree)/include
LDFLAGS += -Ttext 0

export VERSION NAME
export ARCH CROSS_COMPILE AS LD CC OBJCOPY NM AR
export STRIP
export ASFLAGS CFLAGS LDFLAGS

# Subdir 
SUBDIR += boot init lib drivers kernel fs mm kernel/math

ARCHIVES := $(srctree)/kernel/kernel.o
DRIVERS  := $(srctree)/drivers/chr_drv/chr_drv.o
LIBS     := $(srctree)/lib/lib.o
MATH     := $(srctree)/kernel/math/math.o 
FS       := $(srctree)/fs/fs.o
MM       := $(srctree)/mm/mm.o

IMAGE_PACKAGE := $(ARCHIVES) $(DRIVERS) $(LIBS) $(MATH) $(FS) $(MM)

export SUBDIR IMAGE_PACKAGE

# General Rule
.c.o:
	$(Q)$(CC) $(CFLAGS) -c -o $*.o $<
.s.o:
	$(Q)$(AS) $(ASFLAGS) -o $*.o $<


# To do compile
all: CHECK_START $(SUBDIR) Image 
	$(Q)figlet "BiscuitOS"

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

call:
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
	
