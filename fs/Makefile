LDFLAGS += -r

OBJS := super.o
OBJS += buffer.o
OBJS += open.o
OBJS += inode.o
OBJS += truncate.o
OBJS += bitmap.o
OBJS += file_table.o
OBJS += read_write.o
OBJS += pipe.o
OBJS += char_dev.o
OBJS += file_dev.o
OBJS += block_dev.o
OBJS += namei.o
OBJS += exec.o
OBJS += stat.o
OBJS += fcntl.o
OBJS += ioctl.o

all: $(OBJS) fs.o ECHO

fs.o: $(OBJS)
	$(Q)$(LD) $(LDFLAGS) -o fs.o $^
	$(Q)sync

ECHO:

clean:
	$(Q)rm *.o
