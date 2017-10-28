obj-y += head.o

LDFLAGS += -Ttext 0
-include $(srctree)/include/config/auto.conf

bootloader: bootsect setup ECHO
	$(Q)dd if=/dev/zero bs=512 count=5 of=$@ \
	       conv=notrunc > /dev/null 2>&1
	$(Q)dd if=bootsect bs=512 count=1 of=$@ \
	       conv=notrunc > /dev/null 2>&1
	$(Q)dd if=setup bs=512 seek=1 count=4 \
	       of=$@ conv=notrunc > /dev/null 2>&1
	$(warning "  DC    uboot")

bootsect: bootsect.o
	$(Q)$(LD) $(LDFLAGS) -o $@ $<
ifdef CONFIG_DEBUG_BOOTLOADER
	$(Q)cp $@ .debug_$@
endif
	$(Q)$(OBJCOPY) -R .pdr -R .comment -R.note -S -O binary $@

setup: setup.o
	$(Q)$(LD) $(LDFLAGS) -o $@ $<
ifdef CONFIG_DEBUG_BOOTLOADER
	$(Q)cp $@ .debug_$@
endif
	$(Q)$(OBJCOPY) -R .pdr -R .comment -R.note -S -O binary $@

bootsect.o: bootsect.s
	$(Q)$(AS) $(KBUILD_AFLAGS_KERNEL) -o $@ $<
	
setup.o: setup.s
	$(Q)$(AS) $(KBUILD_AFLAGS_KERNEL) -o $@ $<
	
ECHO:

clean:
	$(Q)rm -rf bootsect setup bootloader
ifdef CONFIG_DEBUG_BOOTLOADER
	$(Q)rm -rf .debug_bootsect boot_sect
endif
