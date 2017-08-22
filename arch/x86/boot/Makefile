all: bootloader

obj-y += head.o

$(obj)/%.o: $(obj)/%.s FORCE
	$(Q)$(AS) $(KBUILD_AFLAGS_KERNEL) -o $@ $<

# build bootloader for i386
bootloader:
	$(Q)$(AS) $(KBUILD_AFLAGS_KERNEL) -o bootsect.o bootsect.s
	$(Q)$(AS) $(KBUILD_AFLAGS_KERNEL) -o setup.o setup.s
