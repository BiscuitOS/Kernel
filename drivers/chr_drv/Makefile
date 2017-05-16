LDFLAGS += -r

OBJS := tty_io.o
OBJS += serial.o
OBJS += console.o
OBJS += rs_io.o
OBJS += keyboard.o

all: $(OBJS) chr_drv.o 

chr_drv.o: $(OBJS) 
	$(Q)$(LD) $(LDFLAGS) -o chr_drv.o $^
	$(Q)sync

keyboard.s:
	$(Q)$(CPP) $(CPPFLAGS) kb.S -o keyboard.s

clean:
	$(Q)rm *.o 
	$(Q)rm keyboard.s 
