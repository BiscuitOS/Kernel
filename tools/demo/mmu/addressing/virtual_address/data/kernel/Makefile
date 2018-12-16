obj-$(CONFIG_DEBUG_VA_KERN)   += data.o

## Newest Kernel version
INDIC_FLAGS += -fno-common 
INDIC_FLAGS += -DDEBUG_A

DO_HERE     :=$(shell $(obj)/do.sh $(INDIC_FLAGS))
$(info " SH      tools/demo/mmu/addressing/virtual_address/data/kernel$(DO_HERE)")
