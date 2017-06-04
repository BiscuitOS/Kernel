LDFLAGS += -r

OBJS := common.o
OBJS += stringlib.o
OBJS += inline_assembly.o
OBJS += interrupt.o
OBJS += common_assembly.o
OBJS += system_call.o
OBJS += MMU.o

all: $(OBJS) testcode.o ECHO

testcode.o: $(OBJS)
	$(Q)$(LD) $(LDFLAGS) -o testcode.o $^
	$(Q)sync

ECHO:

clean:
	$(Q)rm *.o
