
## Package information
# Target name
TARGET_IMAGE := BiscuitOS
# valinux path
VMLINUX := $(srctree)/vmlinux
# bootloader
BOOTLOADER := $(srctree)/arch/$(SRCARCH)/boot/bootloader
# Size of vmlinux
IMAGE_SIZE := $(word 1,$(shell du -b $(VMLINUX)))
# Size of Image
IMAGE_SIZE := $(shell expr $(IMAGE_SIZE) + 5120)
# sector of IMAGE
TOTAL_SECT := $(shell expr $(IMAGE_SIZE) / 512)
# sector of vmlinux
VMLINUX_SECT := $(shell expr $(TOTAL_SECT) - 8)

vmlinux: ECHO
	$(Q)dd bs=512 if=/dev/zero count=$(TOTAL_SECT) \
	       of=$(TARGET_IMAGE) conv=notrunc > /dev/null 2>&1
	$(Q)dd bs=512 if=$(BOOTLOADER) count=5 \
	       of=$(TARGET_IMAGE) conv=notrunc > /dev/null 2>&1
	$(Q)dd bs=512 if=$(VMLINUX) seek=5 count=$(VMLINUX_SECT) \
	       of=$(TARGET_IMAGE) conv=notrunc > /dev/null 2>&1
	$(Q)$(warning "HelloWorld")
ECHO:
