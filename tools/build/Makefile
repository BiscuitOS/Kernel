# ----------------------------------------------------------------------
# Create Image
# Maintainer: Buddy <buddy.zhang@aliyun.com>
#
# Copyright (C) 2017 BiscuitOS
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.

ifneq ($(DEBUG),)
  include $(srctree)/tools/debug/Makefile
endif

all: Image System
# ---------------------------
# Start debug system
# ---------------------------
ifneq ($(DEBUG),)

# ---------------------------
# Create floppy or disk image
# ---------------------------
ifeq ($(DEBUG_TOOLS), qemu)
System: 
	$(Q)dd if=/dev/zero of=$(IMAGE_FILE) bs=512 count=2880 
ifeq ($(BOOTLOADER), Floppy)
		$(Q) - losetup -d $(VIRTUAL_DEVICE)
		$(Q)losetup $(VIRTUAL_DEVICE) $(IMAGE_FILE)
		$(Q)dd if=Image ibs=512 skip=4096 of=$(VIRTUAL_DEVICE) \
                    obs=512 seek=0 count=1
else
		$(Q)dd if=Image ibs=512 skip=4096 of=$(IMAGE_FILE) \
	                obs=512 seek=0 count=1
endif

# -------------------------
# Runing system on Qemu
# More Qemu configure see tools/debug/qemu/Makefile.header
# -------------------------
start:
ifeq ($(BOOTLOADER), Floppy)
		$(Q)$(QEMU) $(QEMU_FLAGS) -boot order=a -fda $(VIRTUAL_DEVICE)
else
		$(Q)$(QEMU) $(QEMU_FLAGS) -boot a -hda $(IMAGE_FILE)
endif

# -------------------------
# Debug system on Qemu
# -------------------------
debug:
ifeq ($(BOOTLOADER), Floppy)
		$(Q)$(QEMU) $(QEMU_FLAGS) -boot order=a -fda $(VIRTUAL_DEVICE)
else
		$(Q)$(QEMU) $(QEMU_FLAGS) -boot a -hda $(IMAGE_FILE)
endif

else # DEBUG_TOOLS

# -------------------------
# Use other debug tools
# -------------------------
System: Image
	# Empty

start:
	# Empty

debug:
	# Empty


endif # End of EBUG_TOOLS
# --------------------------
# Don't debug system
# --------------------------
else # DEBUG
System: Image
	# Empty

start:
	# Empty

debug:
	# Empty

endif # End DEBUG

# -------------------------
# Link all objects
# -------------------------
LD_SCRIPTS := $(srctree)/arch/$(ARCH)/boot.ld

OBJFILE := $(srctree)/boot/bootsect.o

Image: ECHO $(OBJFILE)
	$(Q)$(LD) $(LDFLAGS) -o $@ $(OBJFILE) -T$(LD_SCRIPTS)

ECHO:

.PHONY: clean
clean:
	$(Q)rm Image $(IMAGE_FILE)
ifeq ($(DEBUG_TOOLS), qemu)
ifeq ($(BOOTLOADER), Floppy)
		- losetup -d $(VIRTUAL_DEVICE)
endif
endif
