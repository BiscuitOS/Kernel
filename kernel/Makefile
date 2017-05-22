LDFLAGS += -r

OBJS := sched.o
OBJS += vsprintf.o
OBJS += printk.o
OBJS += panic.o
OBJS += asm.o
OBJS += traps.o
OBJS += system_call.o
OBJS += signal.o
OBJS += mktime.o

all: $(OBJS) KERNEL

KERNEL: $(OBJS)
	$(Q)$(LD) $(LDFLAGS) -o kernel.o $^
	$(Q)sync

clean:
	$(Q)rm *.o
