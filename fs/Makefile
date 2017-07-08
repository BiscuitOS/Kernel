LDFLAGS += -r

OBJS := super.o
OBJS += buffer.o
OBJS += open.o
OBJS += inode.o
OBJS += truncate.o
OBJS += bitmap.o

all: $(OBJS) fs.o ECHO

fs.o: $(OBJS)
	$(Q)$(LD) $(LDFLAGS) -o fs.o $^
	$(Q)sync

ECHO:

clean:
	$(Q)rm *.o
