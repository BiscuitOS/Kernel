#/bin/bash

PWD=`pwd`/tools/demo/mmu/addressing/virtual_address/data/kernel
SRC=${PWD}/kern_data.c
DET=${PWD}/.tmp/kern_data.ko
MF=${PWD}/.tmp/Makefile

if [ -d ${PWD}/.tmp ]; then
  rm -rf ${PWD}/.tmp > /dev/null 2>&1
fi
mkdir -p ${PWD}/.tmp

## Auto create Makefie to build module
echo '# Module on higher linux version' >> ${MF}
echo 'obj-m += kern_data.o' >> ${MF}
echo '' >> ${MF}
echo 'kern_dataDIR ?= /lib/modules/$(shell uname -r)/build' >> ${MF}
echo '' >> ${MF}
echo 'PWD       := $(shell pwd)' >> ${MF}
echo '' >> ${MF}
echo 'ROOT := $(dir $(M))' >> ${MF}
echo 'DEMOINCLUDE := -I$(ROOT)../include -I$(ROOT)/include' >> ${MF}
echo '' >> ${MF}
echo 'all:' >> ${MF}
echo '\t$(MAKE) -C $(kern_dataDIR) M=$(PWD) modules' >> ${MF}
echo '' >> ${MF}
echo 'install:' >> ${MF}
echo '\t@sudo insmod kern_data.ko' >> ${MF}
echo '\t@dmesg | tail -n 18' >> ${MF}
echo '\t@sudo rmmod kern_data' >> ${MF}
echo '' >> ${MF}
echo 'clean:' >> ${MF}
echo '\t@rm -rf *.o *.o.d *~ core .depend .*.cmd *.ko *.ko.unsigned *.mod.c .tmp_ \' >> ${MF}
echo '    .cache.mk *.save *.bak Modules.* modules.order Module.* *.b' >> ${MF}
echo '' >> ${MF}
echo 'CFLAGS_kern_data.o := -Wall $(DEMOINCLUDE)' >> ${MF}
echo "CFLAGS_kern_data.o += $*" >> ${MF}
echo '' >> ${MF}

# Copy SRC file
cp ${SRC} ${PWD}/.tmp
# Compile Kernel module
#make -s -C ${PWD}/.tmp > /dev/null 2>&1
make -s -C ${PWD}/.tmp 
# install ko
cp ${DET} ${PWD}
# Rmove tmpdir
#rm -rf ${PWD}/.tmp
