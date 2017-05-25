LDFLAGS += -r

OBJS := string.o
OBJS += ctype.o
OBJS += errno.o

all: $(OBJS) lib.o ECHO

lib.o: $(OBJS)
	$(Q)$(LD) $(LDFLAGS) -o lib.o $^
	$(Q)sync

ECHO:

clean:
	$(Q)rm *.o
