/*
 * ELF format
 *
 * (C) 2018.03 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#define __LIBRARY__
#include <unistd.h>

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/fs.h>

#include <test/debug.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <elf.h>

/*
 * Parse ELF header
 *
 * The ELF header defines whether to use 32-bit addresses. The header 
 * contains three fields that are affected by this setting and offset other 
 * fields that follow them. The ELF header is 52 bytes long for 32-bit 
 * binaries respectively.
 *
 * 32 Bit ELF header
 * -----------------------------------------------------------------------
 * | Offset | Size |       Field       |            Purpose              |
 * -----------------------------------------------------------------------
 * | 0x00   |  4   | e_ident[EI_MAG0]  | 0x7F followed by ELF(45 4c 46)  |
 * |        |      | e_ident[EI_MAG3]  | in ASCII; these four bytes      |
 * |        |      |                   | constitute the magic number.    |
 * -----------------------------------------------------------------------
 * | 0x04   |  1   | e_ident[EI_CLASS] | This byte is set to 1 or 2 to   |
 * |        |      |                   |signif 32-bit format             |
 * -----------------------------------------------------------------------
 * | 0x05   |  1   | e_ident[EI_DATA]  | This byte is set to either 1 or |
 * |        |      |                   | 2 to signify little or big      |
 * |        |      |                   | endianness, respectively. This  |
 * |        |      |                   | affects interpretation of       |
 * |        |      |                   | multi-byte fields starting with |
 * |        |      |                   | offset 0x10.                    |
 * -----------------------------------------------------------------------
 * | 0x06   |  1   | e_ident[EI_VE..N] | Set to 1 for the original       |
 * |        |      | EI_VERSION        | version of ELF.                 |
 * -----------------------------------------------------------------------
 * | 0x07   |  1   | e_ident[EI_OSABI] | Identifies the target operating |
 * |        |      |                   | system ABI. It is often set to  |
 * |        |      |                   | 0 regardless of the target      |
 * |        |      |                   | platform.                       |
 * -----------------------------------------------------------------------
 * | 0x08   |  1   | e_ident[EI_AB..N] | Further specifies the ABI       |
 * |        |      | EI_ABIVERSION     | version. Its interpretation     |
 * |        |      |                   | depends on the target ABI.      |
 * |        |      |                   | Linux kernel (after at least    |
 * |        |      |                   | 2.6) has no definition of it.   |
 * |        |      |                   | In that case, offset and size   |
 * |        |      |                   | of EI_PAD are 8.                |
 * -----------------------------------------------------------------------
 * | 0x09   |  7   | e_ident[EI_PAD]   | currently unused                |
 * -----------------------------------------------------------------------
 * | 0x10   |  2   | e_type            | 1, 2, 3, 4 specify whether the  |
 * |        |      |                   | object is relocatable,          |
 * |        |      |                   | executable, shared, or core,    |
 * |        |      |                   | respectively.                   |
 * -----------------------------------------------------------------------
 * | 0x12   |  2   | e_machine         | Specifies target instruction    |
 * |        |      |                   | set architecture.               |
 * ----------------------------------------------------------------------- 
 * | 0x14   |  4   | e_version         | Set to 1 for the original       |
 * |        |      |                   | version of ELF.                 |
 * -----------------------------------------------------------------------
 * | 0x18   |  4   | e_entry           | This is the memory address of   | 
 * |        |      |                   | the entry point from where the  |
 * |        |      |                   | process starts executing. This  |
 * |        |      |                   | field is either 32 or 64 bits   |
 * |        |      |                   | long depending on the format    |
 * |        |      |                   | defined earlier.                |
 * -----------------------------------------------------------------------
 * | 0x1C   |  4   | e_phoff           | Points to the start of the      |
 * |        |      |                   | program header table. It        |
 * |        |      |                   | usually follows the file header |
 * |        |      |                   | immediately, making the offset  |
 * |        |      |                   | 0x34 for 32-bit ELF executables |
 * -----------------------------------------------------------------------
 * | 0x20   |  4   | e_shoff           | Points to the start of the      |
 * |        |      |                   | section header table.           |
 * -----------------------------------------------------------------------
 * | 0x24   |  4   | e_flags           | Interpretation of this field    |
 * |        |      |                   | depends on the target           |
 * |        |      |                   | architecture.                   |
 * -----------------------------------------------------------------------
 * | 0x28   |  2   | e_ehsize          | Contains the size of this       |
 * |        |      |                   | header normally 52 Bytes        |
 * ----------------------------------------------------------------------- 
 * | 0x2A   |  2   | e_phentsize       | Contains the size of a program  |
 * |        |      |                   | header table entry.             |
 * -----------------------------------------------------------------------
 * | 0x2C   |  2   | e_phnum           | Contains the number of entries  |
 * |        |      |                   | in the program header table.    |
 * -----------------------------------------------------------------------
 * | 0x2E   |  2   | e_shentsize       | Contains the size of a section  |
 * |        |      |                   | header table entry.             |
 * -----------------------------------------------------------------------
 * | 0x30   |  2   | e_shnum           | Contains the number of entries  |
 * |        |      |                   | in the section header table     |
 * -----------------------------------------------------------------------
 * | 0x32   |  2   | e_shstrndx        | Contains index of the section   |
 * |        |      |                   | header table entry that         |
 * |        |      |                   | contains the section names.     |
 * -----------------------------------------------------------------------
 *
 */
static int parse_elf_header(Elf32_Ehdr *eh)
{
    /* Magic number and other information */
    printk("E_IDENT:      %#x %#x %#x %#x\n", eh->e_ident[EI_MAG0], 
            eh->e_ident[EI_MAG1], eh->e_ident[EI_MAG2], 
            eh->e_ident[EI_MAG3]);

    /* File format: e_ident[EI_CLASS]
     *   ELFCLASS32:          1    Elf32
     *   ELFCLASS64:          2    Elf64 
     */
    printk("Object:       %s\n", (eh->e_ident[EI_CLASS] == ELFCLASS32) ? 
                         "Elf32" : "Elf64");

    /* Object file: e_ident[EI_DATA]
     *   ELFDATANONE:         0    Invalid data encoding
     *   ELFDATA2LSB:         1    2's complement, little endian
     *   ELFDATA2MSB:         2    2's complement, big endian
     */
    if (eh->e_ident[EI_DATA])
        printk("Data coding:  %s\n", (eh->e_ident[EI_DATA] == ELFDATA2LSB) ?
                "LSB" : "MSB");
    else
        printk("Data coding: Invalid data encoding\n");

    /* File version: e_ident[EI_VERSION] */
    printk("File Version: %d\n", eh->e_ident[EI_VERSION]);

    /* OS ABI identification: e_ident[EI_OSABI]
     *   ELFOSABI_SYSV:       0    Unix system V ABI 
     *   ELFOSABI_HPUX:       1    HP-UX
     *   ELFOSABI_NETBSD:     2    NetBSD
     *   ELFOSABI_GNU:        3    Object uses GNU ELF extensions.
     *   ELFOSABI_LINUX:      3    Linux
     */
    switch (eh->e_ident[EI_OSABI]) {
    case ELFOSABI_SYSV:
        printk("ABI:          Unix SystemV\n");
        break;
    case ELFOSABI_HPUX:
        printk("ABI:          HP-Unix\n");
        break;
    case ELFOSABI_NETBSD:
        printk("ABI:          NetBSD\n");
        break;
    case ELFOSABI_GNU:
        printk("ABI:          Linux/GNU ELF extension\n");
        break;
    default:
        printk("ABI:          Unknow ABI\n");
        break;
    }

    /* ABI version: e_ident[EI_ABIVERSION] */
    printk("ABI Version:  %d\n", eh->e_ident[EI_ABIVERSION]);

    /* Object file type: e_type 
     *   ET_NONE:      0     No file type
     *   ET_REL:       1     Relocatable file
     *   ET_EXEC:      2     Executable file
     *   ET_DYN:       3     Shared object file
     *   ET_CORE:      4      Core file
     */
    switch (eh->e_type) {
    case ET_NONE:
        printk("File type:    No file type\n");
        break;
    case ET_REL:
        printk("File type:    Relocatable file\n");
        break;
    case ET_EXEC:
        printk("File type:    Executable file\n");
        break;
    case ET_DYN:
        printk("File type:    Shared object file\n");
        break;
    case ET_CORE:
        printk("File type:    Core file\n");
        break;
    default:
        printk("File type:    Unknow type\n");
        break;
    }

    /* Target machine: e_machine
     *   EM_NONE:      0   No machine
     *   EM_M32:       1   AT&T WE 32100
     *   EM_SPARC:     2   SUN SPARC
     *   EM_386:       3   Intel 80386
     */
    switch (eh->e_machine) {
    case EM_NONE:
        printk("Target Mach:  No machine\n");
        break;
    case EM_M32:
        printk("Target Mach:  AT&T WE 32100\n");
        break;
    case EM_SPARC:
        printk("Target Mach:  SUN SPARC\n");
        break;
    case EM_386:
        printk("Target Mach:  Intel 80386\n");
        break;
    default:
        printk("Target Mach:  Unknow machine\n");
        break;
    }

    /* Version: e_version */
    printk("Version:      %d\n", eh->e_version);

    /* Entry point virtual address
     *  This is the memory address of the entry point from where the 
     *  process starts executing. This field is either 32 or 64 bits 
     *  long depending on the format defined earlier.
     */
    printk("Entry-Addr:   %#8x\n", eh->e_entry);

    /* Program header table file offset
     *  Points to the start of the program header table. It usually 
     *  follows the file header immediately, making the offset 0x34 
     *  or 0x40 for 32- and 64-bit ELF executables, respectively. 
     */
    printk("Prog-Table:   %#x\n", eh->e_phoff);

    /* Section header table file offset 
     *  Points to the start of the section header table.
     */
    printk("Sect-Table:   %#x\n", eh->e_shoff);

    /* Processor-specific flags
     *  Interpretation of this field depends on the target architecture.
     */
    printk("Prog-flags:   %#x\n", eh->e_flags);

    /* ELF header size in bytes 
     *  Contains the size of this header, normally 64 Bytes for 64-bit 
     *  and 52 Bytes for 32-bit format
     */
    printk("ELF size:     %d\n", eh->e_ehsize);

    /* Program header table entry size 
     *  Contains the size of a program header table entry.
     */
    printk("Phtable size: %#x\n", eh->e_phentsize);

    /* Program header table entry count 
     *  Contains the size of a section header table entry.
     */
    printk("Phtable num:  %#x\n", eh->e_phnum);

    /* Section header table entry size 
     *  Contains the number of entries in the section header table
     */
    printk("SectTab size: %#x\n", eh->e_shentsize);

    /* Section header table entry size 
     *  Contains the number of entries in the section header table
     */
    printk("SectTab num:  %#x\n", eh->e_shnum);

    /* Section header string table index 
     *  Contains index of the section header table entry that 
     *  contains the section names
     */
    printk("StringTab:    %#x\n", eh->e_shstrndx);
    return 0;
}

int sys_d_parse_elf(const char *file, char **argv, char **envp)
{
    struct buffer_head *bh;
    struct m_inode *inode;
    Elf32_Ehdr *eh;

    if (!(inode = namei(file)))
        return -ENOENT;
    if (!(bh = bread(inode->i_dev, inode->i_zone[0]))) {
        panic("Unable read data zone");
        return -ENOENT;
    }
    
    eh = (Elf32_Ehdr *)bh->b_data;
    parse_elf_header(eh);
    return 0;
}

/* Invoke by system call: int $0x80 */
int debug_binary_elf_format(void)
{
    d_parse_elf("/usr/bin/demo", NULL, NULL);
    return 0;
}
