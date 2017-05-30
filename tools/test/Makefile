LDFLAGS += -r
EXTRA_CFLAGS += -DCONFIG_TESTCODE

OBJS := stringlib.o
OBJS += test_main.o

all: $(OBJS) testcode.o ECHO

testcode.o: $(OBJS)
	$(Q)$(LD) $(LDFLAGS) -o testcode.o $^
	$(Q)sync

ECHO:

clean:
	$(Q)rm *.o
