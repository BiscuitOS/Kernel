LDFLAGS += -r

OBJS := math_emulate.o

all: $(OBJS) math.o ECHO

math.o: $(OBJS)
	$(Q)$(LD) $(LDFLAGS) -o math.o $^
	$(Q)sync

ECHO:

clean:
	$(Q)rm *.o 
