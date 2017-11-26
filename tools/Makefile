#
# Kernel hacking to debug or run kernel
#

-include $(srctree)/tools/debug/Makefile

quiet_cmd_running_linux = RUN     vmlinux
      cmd_running_linux = $(QEMU) $(QEMU_FLAGS) > /dev/null 2>&1

quiet_cmd_debug_linux = DEBUG      vmlinux
      cmd_debug_linux = $(QEMU) $(QEMU_FLAGS) -s -S > /dev/null 2>&1

PHONY += start
start: vmlinux
	$(call if_changed,running_linux)

PHONY += debug
debug: vmlinux
	$(call if_changed,debug_linux)
