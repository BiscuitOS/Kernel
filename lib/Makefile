LDFLAGS += -r

OBJS := string.o
OBJS += ctype.o
OBJS += errno.o
OBJS += malloc.o
OBJS += open.o
OBJS += dup.o
OBJS += close.o
OBJS += _exit.o
OBJS += execve.o
OBJS += wait.o
OBJS += setsid.o
OBJS += write.o

all: $(OBJS) lib.o ECHO

lib.o: $(OBJS)
	$(Q)$(LD) $(LDFLAGS) -o lib.o $^
	$(Q)sync

ECHO:

clean:
	$(Q)rm *.o
