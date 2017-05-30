LDFLAGS += -r

OBJS := common.o
OBJS += stringlib.o
OBJS += inline_assembly.o

all: $(OBJS) testcode.o ECHO

testcode.o: $(OBJS)
	$(Q)$(LD) $(LDFLAGS) -o testcode.o $^
	$(Q)sync

ECHO:

clean:
	$(Q)rm *.o
