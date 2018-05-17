obj-y += sched.o
obj-y += vsprintf.o
obj-y += printk.o
obj-y += panic.o
obj-y += traps.o
obj-y += system.o
obj-y += signal.o
obj-y += mktime.o
obj-y += fork.o
obj-y += exit.o
obj-y += sys.o
obj-y += ptrace.o
obj-y += ioport.o
obj-y += irq.o
obj-y += itimer.o
obj-y += version.o
obj-y += info.o
obj-y += ldt.o
obj-y += module.o
obj-y += time.o

$(obj)/%.o: $(obj)/%.s FORCE
	$(Q)$(AS) $(KBUILD_AFLAGS_KERNEL) -o $@ $<

$(obj)/system.s: $(obj)/sys_call.S FORCE
	$(Q)$(CPP) -traditional -I$(srctree)/include $< -o $@

clean:
	$(Q)rm -rf system.*
