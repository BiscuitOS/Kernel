obj-y += head.o


local_LDFLAGS += -Ttext 0

-include include/config/auto.conf

ifdef CONFIG_DEBUG_BOOTLOADER
  KBUILD_AFLAGS_KERNEL += $(call arg-check,-ggdb -am)
endif

####
# Build bootloader
bootloader: bootsect setup ECHO
	$(Q)dd if=/dev/zero bs=512 count=5 of=$@ \
	       conv=notrunc > /dev/null 2>&1
	$(Q)dd if=bootsect bs=512 count=1 of=$@ \
	       conv=notrunc > /dev/null 2>&1
	$(Q)dd if=setup bs=512 seek=1 count=4 \
	       of=$@ conv=notrunc > /dev/null 2>&1

bootsect: bootsect.o
	$(Q)$(LD) $(LDFLAGS) $(local_LDFLAGS) -o $@ $<
ifdef CONFIG_DEBUG_BOOTLOADER
	$(Q)cp $@ .debug_$@
endif
	$(Q)$(OBJCOPY) -R .pdr -R .comment -R.note -S -O binary $@

setup: setup.o
	$(Q)$(LD) $(LDFLAGS) $(local_LDFLAGS) -o $@ $<
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
	$(Q)rm -rf .debug_bootsect .debug_setup
endif
