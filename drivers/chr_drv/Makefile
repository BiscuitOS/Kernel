LDFLAGS += -r

OBJS := tty_io.o
OBJS += serial.o
OBJS += console.o

all: $(OBJS) chr_drv.o

chr_drv.o: $(OBJS)
	$(Q)$(LD) $(LDFLAGS) -o chr_drv.o $^
	$(Q)sync

clean:
	$(Q)rm *.o 
