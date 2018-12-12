#/bin/bash

CC=$*
PWD=`pwd`"/tools/demo/mmu/addressing/virtual_address/user"

##
# Compile data.c
${CC} -o ${PWD}/data.elf ${PWD}/data.c
${CC} ${PWD}/data.c -c -o ${PWD}/data.o

##
# Obtain ELF file
objdump -xdhs ${PWD}/data.o > ${PWD}/data.objdump.elf

##
# Obtain link scripts
ld -verbose > ${PWD}/ld_scripts.elf
