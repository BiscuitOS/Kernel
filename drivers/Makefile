LDFLAGS += -r

chr_OBJS = $(srctree)/drivers/chr_drv/chr_drv.o
blk_OBJS = $(srctree)/drivers/blk_drv/blk_drv.o
OBJS :=

all: $(OBJS) $(chr_OBJS) $(blk_OBJS)
	$(Q)$(LD) $(LDFLAGS) -o drivers.o $^

$(chr_OBJS):
	$(Q)make -C chr_drv all

$(blk_OBJS):
	$(Q)make -C blk_drv all

clean:
	$(Q)make -C chr_drv clean
	$(Q)make -C blk_drv clean
	$(Q)rm *.o
