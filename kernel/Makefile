obj-y += sched.o
obj-y += vsprintf.o
obj-y += printk.o
obj-y += panic.o
obj-y += asm.o
obj-y += traps.o
obj-y += sys_call.o
obj-y += signal.o
obj-y += mktime.o
obj-y += fork.o
obj-y += exit.o
obj-y += sys.o
obj-y += math/
obj-y += ptrace.o

$(obj)/%.o: $(obj)/%.s FORCE
	$(Q)$(AS) $(KBUILD_AFLAGS_KERNEL) -o $@ $<

