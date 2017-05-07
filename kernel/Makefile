LDFLAGS += -r

OBJS := sched.o 
OBJS += vsprintf.o
OBJS += printk.o
OBJS += panic.o


all: $(OBJS) KERNEL 

KERNEL: $(OBJS) 
	$(Q)$(LD) $(LDFLAGS) -o kernel.o $^
	$(Q)sync

clean:
	$(Q)rm *.o
