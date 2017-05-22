LDFLAGS += -r

OBJS := memory.o
OBJS += page.o

all: $(OBJS) mm.o ECHO

mm.o: $(OBJS)
	$(Q)$(LD) $(LDFLAGS) -o mm.o $^
	$(Q)sync

ECHO:

clean:
	$(Q)rm *.o
