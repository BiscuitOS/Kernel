LDFLAGS += -r

OBJS := ll_rw_blk.o
OBJS += hd.o
OBJS += floppy.o

all: $(OBJS) blk_drv.o

blk_drv.o: $(OBJS)
	$(Q)$(LD) $(LDFLAGS) -o blk_drv.o $^
	$(Q)sync

clean:
	$(Q)rm *.o
