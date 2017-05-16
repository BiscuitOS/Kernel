LDFLAGS += -r

OBJS := super.o
OBJS += buffer.o

all: $(OBJS) fs.o ECHO

fs.o: $(OBJS)
	$(Q)$(LD) $(LDFLAGS) -o fs.o $^
	$(Q)sync

ECHO:

clean:
	$(Q)rm *.o 
