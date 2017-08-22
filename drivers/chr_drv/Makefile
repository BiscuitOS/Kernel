obj-y += tty_io.o
obj-y += serial.o
obj-y += console.o
obj-y += rs_io.o
obj-y += keyboard.o
obj-y += tty_ioctl.o

$(obj)/%.o: $(obj)/%.s FORCE
	$(Q)$(AS) $(KBUILD_AFLAGS_KERNEL) -o $@ $<

$(obj)/keyboard.s: $(obj)/kb.S $(srctree)/include/linux/config.h FORCE
	$(Q)$(CPP) $(CPPFLAGS) $(obj)/kb.S -o $(obj)/keyboard.s
