/*
 * ELF format
 *
 * (C) 2018.03 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/fs.h>

#include <test/debug.h>
#include <linux/elf.h>
#include <linux/ptrace.h>
#include <linux/binfmts.h>
#include <linux/string.h>
#include <linux/stat.h>
#include <linux/ctype.h>

#include <asm/segment.h>

#define __LIBRARY__
#include <linux/unistd.h>

static int elf_read(struct inode *inode, unsigned long offset,
                    char *addr, unsigned long count);
#ifdef CONFIG_ELF_SECTION_TABLE
static void parse_section_flags(unsigned int flags);
static void parse_section_type(unsigned int type);
static int elf_section_table_info(struct elf_shdr *shdrp, char *shstrtab);
static int elf_symbol_info(struct Elf32_Sym *sym, char *strtab, int nr);
static int elf_symbol_info_core(struct Elf32_Sym *sym, char *strtab, int nr);
static void elf_symbol_info_header(void);
#endif

/*
 * ELF Section '.bis.data'
 *   More section see "arch/x86/kernel/vmlinux.lds.S"
 *   * Add a global initialized data into '.bis.data'
 *   * Add a global uninitialized data into '.bid.data'
 *   * Add a local static initialized data into '.bis.data'
 *   * Add a local static uninitialized data into '.bis.data'
 */
#ifdef CONFIG_ELF_PRIVATE_SECTION_DATA
__attribute__ ((section(".bis.data"))) int bis_init = 0x687;
__attribute__ ((section(".bis.data"))) int bis_uninit0;
__attribute__ ((section(".bis.data"))) int bis_uninit1;
__attribute__ ((section(".bis.data"))) int bis_uninit2;
__attribute__ ((section(".bis.data"))) int bis_uninit3;
__attribute__ ((section(".bis.data"))) int bis_uninit4;

static void bis_data_section(void)
{
    __attribute__ ((section(".bis.data"))) static int bis_init0 = 0x678;	
    __attribute__ ((section(".bis.data"))) static int bis_uninit5;	
    __attribute__ ((section(".bis.data"))) static int bis_uninit6;	
    __attribute__ ((section(".bis.data"))) static int bis_uninit7;	
    __attribute__ ((section(".bis.data"))) static int bis_uninit8;	
    __attribute__ ((section(".bis.data"))) static int bis_uninit9;	


    if (0) {
        /* remove warning on 'compile stage' */
        printk("bis_init %d\n", bis_init);
        printk("bis_init0 %d\n", bis_init0);
        printk("bis_uninit0 %d\n", bis_uninit0);
        printk("bis_uninit1 %d\n", bis_uninit1);
        printk("bis_uninit2 %d\n", bis_uninit2);
        printk("bis_uninit3 %d\n", bis_uninit3);
        printk("bis_uninit4 %d\n", bis_uninit4);
        printk("bis_uninit5 %d\n", bis_uninit5);
        printk("bis_uninit6 %d\n", bis_uninit6);
        printk("bis_uninit7 %d\n", bis_uninit7);
        printk("bis_uninit8 %d\n", bis_uninit8);
        printk("bis_uninit9 %d\n", bis_uninit9);
    }
}
#endif
/*
 * ELF Section '.bis.text'
 *   More section see 'arch/x86/kernel/vmlinux.lds.S'
 *   * Add a static function into '.bis.text'
 *   * Add a extern function into '.bis.text'
 */
#ifdef CONFIG_ELF_PRIVATE_SECTION_FUNC
__attribute__ ((section(".bis.text"))) static int bis_static_func(void)
{
    extern char __bis_text[], __bis_etext[];

    printk("SECTION: .bis.text [%#x - %#x]\n", 
          (unsigned int)__bis_text, (unsigned int)__bis_etext);
    return 0;
}
__attribute__ ((section(".bis.text"))) int bis_extern_func(void)
{
    return 0;
}
#endif

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
#ifdef CONFIG_PARSE_ELF_HEADER
static int parse_elf_header(struct elfhdr *eh)
{
    int i;

    /* 
     * Magic number and other information 
     *   Start with: 0x7f 0x45 0x4c 0x46 
     */
    printk("Magic: ");
    for (i = 0; i < 16; i++)
        printk(" %#x", (unsigned int)eh->e_ident[i]);
    printk("\n");
    
    /* 
     * File format: e_ident[EI_CLASS]
     *   ELFCLASS32:          1    Elf32
     *   ELFCLASS64:          2    Elf64 
     */
    printk("Object:       %s\n", (eh->e_ident[4] == 1) ?
                         "Elf32" : "Elf64");

    /* 
     * Object file: e_ident[EI_DATA]
     *   ELFDATANONE:         0    Invalid data encoding
     *   ELFDATA2LSB:         1    2's complement, little endian
     *   ELFDATA2MSB:         2    2's complement, big endian
     */
    if (eh->e_ident[5])
        printk("Data coding:  %s\n", (eh->e_ident[5] == 1) ?
                "Little endian" : "Big endian");
    else
        printk("Data coding: Invalid data encoding\n");

    /* 
     * OS ABI identification: e_ident[EI_OSABI]
     *   ELFOSABI_SYSV:       0    Unix system V ABI 
     *   ELFOSABI_HPUX:       1    HP-UX
     *   ELFOSABI_NETBSD:     2    NetBSD
     *   ELFOSABI_GNU:        3    Object uses GNU ELF extensions.
     *   ELFOSABI_LINUX:      3    Linux
     */
    switch (eh->e_ident[7]) {
    case 0:
        printk("ABI:          Unix SystemV\n");
        break;
    case 1:
        printk("ABI:          HP-Unix\n");
        break;
    case 2:
        printk("ABI:          NetBSD\n");
        break;
    case 3:
        printk("ABI:          Linux/GNU ELF extension\n");
        break;
    default:
        printk("ABI:          Unknow ABI\n");
        break;
    }

    /* ABI version: e_ident[EI_ABIVERSION] */
    printk("ABI Version:  %d\n", eh->e_ident[8]);

    /*
     * Object file type: e_type 
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

    /* 
     * Target machine: e_machine
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

    /*
     * Entry point virtual address
     *  This is the memory address of the entry point from where the 
     *  process starts executing. This field is either 32 or 64 bits 
     *  long depending on the format defined earlier.
     */
    printk("Entry-Addr:   %#8x\n", (unsigned int)eh->e_entry);

    /* 
     * Program header table file offset
     *  Points to the start of the program header table. It usually 
     *  follows the file header immediately, making the offset 0x34 
     *  or 0x40 for 32- and 64-bit ELF executables, respectively. 
     */
    printk("Prog-Table:   %#x\n", eh->e_phoff);

    /*
     * Section header table file offset 
     *  Points to the start of the section header table.
     */
    printk("Sect-Table:   %#x\n", eh->e_shoff);

    /* 
     * Processor-specific flags
     *  Interpretation of this field depends on the target architecture.
     */
    printk("Prog-flags:   %#x\n", eh->e_flags);

    /* 
     * ELF header size in bytes 
     *  Contains the size of this header, normally 64 Bytes for 64-bit 
     *  and 52 Bytes for 32-bit format
     */
    printk("ELF size:     %d\n", eh->e_ehsize);

    /* 
     * Program header table entry size 
     *  Contains the size of a program header table entry.
     */
    printk("Phtable size: %#x\n", eh->e_phentsize);

    /* 
     * Program header table entry count 
     *  Contains the size of a section header table entry.
     */
    printk("Phtable num:  %#x\n", eh->e_phnum);

    /* 
     * Section header table entry size 
     *  Contains the number of entries in the section header table
     */
    printk("SectTab size: %#x\n", eh->e_shentsize);

    /* 
     * Section header table entry size 
     *  Contains the number of entries in the section header table
     */
    printk("SectTab num:  %#x\n", eh->e_shnum);

    /* 
     * Section header string table index 
     *  Contains index of the section header table entry that 
     *  contains the section names
     */
    printk("StringTab:    %#x\n", eh->e_shstrndx);

    return 0;
}
#endif // CONFIG_PARSE_ELF_HEADER

#ifdef CONFIG_ELF_SECTION_SHSTRTAB
/*
 * Parse ELF .shstrtab section
 *   Section String Table that contain string for section table, e.g.
 *   .data, .code and .strtab and so on.
 *
 *   e_shstrndx point to index of .shstrtab in Section Table.
 */
#define SHSTRTAB_MAXLEN      128
static int parse_elf_shstrtab_section(struct inode *inode, 
                            struct elf_shdr *shdrp)
{
    char shstrtab[SHSTRTAB_MAXLEN];
    unsigned long old_fs;
    int retval;
    int i;

    /*
     * Obtain '.strtab' from ELF file
     */
    old_fs = get_fs();
    set_fs(get_ds());
    retval = elf_read(inode, shdrp->sh_offset, shstrtab, shdrp->sh_size);
    set_fs(old_fs);
    if (retval < 0) {
        printk("Unable to obtain '.strtab' from ELF file.\n");
        goto err;
    }

    for (i = 0; i < shdrp->sh_size; i++) 
        printk("%c", shstrtab[i]);
    return 0;

err:
    return retval;
}
#endif

#ifdef CONFIG_ELF_SECTION_STRTAB
#define STRTAB_MAXLEN    256
/*
 * Parse ELF .strtab section
 *   String table that contaim string from symbol.
 */
static int parse_elf_strtab_section(struct inode *inode,
             struct elf_shdr *shdrp, const char *shstrtab)
{
    unsigned long old_fs;
    char strtab[STRTAB_MAXLEN];
    int retval, i;

    printk("Loading %s section.\n", &shstrtab[shdrp->sh_name]);
    old_fs = get_fs();
    set_fs(get_ds());
    retval = elf_read(inode, shdrp->sh_offset, strtab, shdrp->sh_size);
    set_fs(old_fs);
    if (retval < 0) {
        printk("Unable to obtain '.strtab' from ELF file.\n");
        goto err;
    }

    for (i = 0; i < shdrp->sh_size; i++)
        printk("%c", strtab[i]);
    return 0;

err:
    Ereturn retval;
}
#endif

#ifdef CONFIG_ELF_SECTION_CODE
#define CODE_MAXLEN       256
/*
 * Parse ELF .code section
 *   Code section contain all executable code.
 */
static int parse_elf_code_section(struct inode *inode,
             struct elf_shdr *shdrp, const char *shstrtab)
{
    unsigned long old_fs;
    unsigned long context[CODE_MAXLEN];
    char *chtab;
    int retval, i, j, k, size;

    /*
     * Read .text or .data section from ELF file
     */
    old_fs = get_fs();
    set_fs(get_ds());
    retval = elf_read(inode, shdrp->sh_offset, (char *)context, 
             shdrp->sh_size - CODE_MAXLEN * sizeof(unsigned long) ? 
             CODE_MAXLEN * sizeof(unsigned long) : shdrp->sh_size);
    set_fs(old_fs);
    if (retval < 0) {
       printk("Unable loading code section from ELF file.\n");
       goto err;
    }

    /* Display elf-section table information */
    elf_section_table_info(shdrp, (char *)shstrtab);

    /* Dump text context */
    printk("Context:\n");
    chtab = (char *)context;
    size = ((CODE_MAXLEN - shdrp->sh_size / 4 ? shdrp->sh_size / 4 : 
             CODE_MAXLEN) / 4) ? ((CODE_MAXLEN - shdrp->sh_size / 4 ?
             shdrp->sh_size / 4 : CODE_MAXLEN) / 4) : 1;
    for (i = 0; i < size; i++) {
        printk("%04x  ", i * 16);
        for (j = 0; j < (size == 1 ? 1 : 4); j++)
            printk("%08x ", (unsigned int)context[i * 4 + j]);
        printk("  ");
        for (k = 0; k < (size == 1 ? 4 : 16); k++)
            if (isalnum((unsigned char)chtab[i * 16 + k]) ||
                isalpha((unsigned char)chtab[i * 16 + k]))
                printk("%c", chtab[i * 16 + k]);
            else
                printk(".");
        printk("\n");
    }
    printk("\n");
    return 0;

err:
    return retval;
}
#endif

#ifdef CONFIG_ELF_SECTION_DATA
#define DATA_MAXLEN      128
/*
 * Parse ELF .data section
 *   Data section contain all initialize data.
 */
static int parse_elf_data_section(struct inode *inode,
             struct elf_shdr *shdrp, const char *shstrtab)
{
    unsigned long old_fs;
    unsigned long content[DATA_MAXLEN];
    char *chtab;
    int retval, i, j, k, size;

    /*
     * Load .data section from ELF file.
     */
    old_fs = get_fs();
    set_fs(get_ds());
    retval = elf_read(inode, shdrp->sh_offset, (char *)content,
                 (DATA_MAXLEN * sizeof(unsigned long) - shdrp->sh_size) ? 
                 shdrp->sh_size : DATA_MAXLEN * sizeof(unsigned long));
    set_fs(old_fs);
    if (retval < 0) {
        printk("Unable to load .data from ELF by elf_read!\n");
        goto err;
    }

    /* Display elf-section table information */
    elf_section_table_info(shdrp, (char *)shstrtab);

    /* Content */
    printk("Content:\n");
    chtab = (char *)content;
    size = ((DATA_MAXLEN - shdrp->sh_size / 4 ? shdrp->sh_size / 4 : 
             DATA_MAXLEN) / 4) ? ((DATA_MAXLEN - shdrp->sh_size / 4 ? 
             shdrp->sh_size / 4 : DATA_MAXLEN) / 4) : 1;
    for (i = 0; i < size; i++) {
        printk("%04x  ", i * 16);
        for (j = 0; j < (size == 1 ? 1 : 4); j++)
            printk("%08x ", (unsigned int)content[i * 4 + j]);
        for (k = 0; k < (size == 1 ? 4 : 16); k++)
            if (isalnum((unsigned char)chtab[i * 16 + k]) ||
                isalpha((unsigned char)chtab[i * 16 + k]))
                printk("%c", chtab[i * 16 + k]);
            else
                printk(".");
        printk("\n");
    }
    printk("\n");

    return 0;

err:
    return retval;
}
#endif

#ifdef CONFIG_ELF_SECTION_BSS
#define BSS_MAXLEN      128
/*
 * Parse ELF .bss section
 *   BSS section contain all uninitialize data.
 */
static int parse_elf_bss_section(struct inode *inode,
             struct elf_shdr *shdrp, const char *shstrtab)
{
    unsigned long old_fs;
    unsigned long content[BSS_MAXLEN];
    char *chtab;
    int retval, i, j, k, size;

    /*
     * Load .bss section from ELF file.
     */
    old_fs = get_fs();
    set_fs(get_ds());
    retval = elf_read(inode, shdrp->sh_offset, (char *)content,
                 (BSS_MAXLEN * sizeof(unsigned long) - shdrp->sh_size) ?
                 shdrp->sh_size : BSS_MAXLEN * sizeof(unsigned long));
    set_fs(old_fs);
    if (retval < 0) {
        printk("Unable to load .bss from ELF by elf_read()!\n");
        goto err;
    }

    /* Display elf-section table information */
    elf_section_table_info(shdrp, (char *)shstrtab);

    /* Content */
    printk("Content:\n");
    chtab = (char *)content;
    size = ((BSS_MAXLEN - shdrp->sh_size / 4 ? shdrp->sh_size / 4 :
             BSS_MAXLEN) / 4) ? ((BSS_MAXLEN - shdrp->sh_size / 4 ?
             shdrp->sh_size / 4 : BSS_MAXLEN) / 4) : 1;
    for (i = 0; i < size; i++) {
        printk("%04x  ", i * 16);
        for (j = 0; j < (size == 1 ? 1 : 4); j++)
            printk("%08x ", (unsigned int)content[i * 4 + j]);
        for (k = 0; k < (size == 1 ? 4 : 16); k++)
            if (isalnum((unsigned char)chtab[i * 16 + k]) ||
                isalpha((unsigned char)chtab[i * 16 + k]))
                printk("%c", chtab[i * 16 + k]);
            else
                printk(".");
        printk("\n");
    }
    printk("\n");

    return 0;

err:
    return retval;
}
#endif

#ifdef CONFIG_ELF_SECTION_RODATA
#define RODATA_MAXLEN      128
/*
 * Parse ELF .rodata section
 *   Read-Only Data section contain all read-only data.
 */
static int parse_elf_rodata_section(struct inode *inode,
             struct elf_shdr *shdrp, const char *shstrtab)
{
    unsigned long old_fs;
    unsigned long content[RODATA_MAXLEN];
    char *chtab;
    int retval, i, j, k, size;

    /*
     * Load .bss section from ELF file.
     */
    old_fs = get_fs();
    set_fs(get_ds());
    retval = elf_read(inode, shdrp->sh_offset, (char *)content,
                 (RODATA_MAXLEN * sizeof(unsigned long) - shdrp->sh_size) ?
                 shdrp->sh_size : RODATA_MAXLEN * sizeof(unsigned long));
    set_fs(old_fs);
    if (retval < 0) {
        printk("Unable to load .rodata from ELF by elf_read()!\n");
        goto err;
    }

    /* Display elf-section table information */
    elf_section_table_info(shdrp, (char *)shstrtab);

    /* Content */
    printk("Content:\n");
    chtab = (char *)content;
    size = ((RODATA_MAXLEN - shdrp->sh_size / 4 ? shdrp->sh_size / 4 :
             RODATA_MAXLEN) / 4) ? ((RODATA_MAXLEN - shdrp->sh_size / 4 ?
             shdrp->sh_size / 4 : RODATA_MAXLEN) / 4) : 1;
    for (i = 0; i < size; i++) {
        printk("%04x  ", i * 16);
        for (j = 0; j < (size == 1 ? 1 : 4); j++)
            printk("%08x ", (unsigned int)content[i * 4 + j]);
        for (k = 0; k < (size == 1 ? 4 : 16); k++)
            if (isalnum((unsigned char)chtab[i * 16 + k]) ||
                isalpha((unsigned char)chtab[i * 16 + k]))
                printk("%c", chtab[i * 16 + k]);
            else
                printk(".");
        printk("\n");
    }
    printk("\n");

    return 0;

err:
    return retval;
}
#endif

#ifdef CONFIG_ELF_SECTION_COMMENT
#define COMMENT_MAXLEN      128
/*
 * Parse ELF .comment section
 *   Comment section contain all comment information.
 */
static int parse_elf_comment_section(struct inode *inode,
             struct elf_shdr *shdrp, const char *shstrtab)
{
    unsigned long old_fs;
    unsigned long content[COMMENT_MAXLEN];
    char *chtab;
    int retval, i, j, k, size;

    /*
     * Load .comment section from ELF file.
     */
    old_fs = get_fs();
    set_fs(get_ds());
    retval = elf_read(inode, shdrp->sh_offset, (char *)content,
                 (COMMENT_MAXLEN * sizeof(unsigned long) - shdrp->sh_size) ?
                 shdrp->sh_size : COMMENT_MAXLEN * sizeof(unsigned long));
    set_fs(old_fs);
    if (retval < 0) {
        printk("Unable to load .comment from ELF by elf_read()!\n");
        goto err;
    }

    /* Display elf-section table information */
    elf_section_table_info(shdrp, (char *)shstrtab);

    /* Content */
    printk("Content:\n");
    chtab = (char *)content;
    size = ((COMMENT_MAXLEN - shdrp->sh_size / 4 ? shdrp->sh_size / 4 :
             COMMENT_MAXLEN) / 4) ? ((COMMENT_MAXLEN - shdrp->sh_size / 4 ?
             shdrp->sh_size / 4 : COMMENT_MAXLEN) / 4) : 1;
    for (i = 0; i < size; i++) {
        printk("%04x  ", i * 16);
        for (j = 0; j < (size == 1 ? 1 : 4); j++)
            printk("%08x ", (unsigned int)content[i * 4 + j]);
        for (k = 0; k < (size == 1 ? 4 : 16); k++)
            if (isalnum((unsigned char)chtab[i * 16 + k]) ||
                isalpha((unsigned char)chtab[i * 16 + k]))
                printk("%c", chtab[i * 16 + k]);
            else
                printk(".");
        printk("\n");
    }
    printk("\n");

    return 0;

err:
    return retval;
}
#endif

#ifdef CONFIG_ELF_SECTION_SYMTAB
#define SYMTAB_MAXLEN      256
#define STRTAB_MAXLEN      256
/*
 * Parse ELF .symtab section
 *   Symbol table section contain all symbol.
 */
static int parse_elf_symtab_section(struct inode *inode,
             struct elf_shdr *shdrp, const char *shstrtab,
             struct elf_shdr *strtabp)
{
    unsigned long old_fs;
    unsigned long content[SYMTAB_MAXLEN];
    char strtab[STRTAB_MAXLEN];
    struct Elf32_Sym *symtab;
    int retval;
    int i;

    /*
     * Load .comment section from ELF file.
     */
    old_fs = get_fs();
    set_fs(get_ds());
    retval = elf_read(inode, shdrp->sh_offset, (char *)content,
                 (SYMTAB_MAXLEN * sizeof(unsigned long) - shdrp->sh_size) ?
                 shdrp->sh_size : SYMTAB_MAXLEN * sizeof(unsigned long));
    set_fs(old_fs);
    if (retval < 0) {
        printk("Unable to load .symtab from ELF by elf_read()!\n");
        goto err;
    }

    /* Load .strtab section from ELF file. */
    set_fs(get_ds());
    retval = elf_read(inode, strtabp->sh_offset, strtab,
              (STRTAB_MAXLEN - strtabp->sh_size ?
               strtabp->sh_size : STRTAB_MAXLEN));
    set_fs(old_fs);

    if (retval < 0) {
        printk("Unable to load .strtab section from ELF.\n");
        goto err;
    }
    /* Point to .strtab section */
    symtab = (struct Elf32_Sym *)content;
    
    elf_symbol_info_header();
    for (i = 0; i < (shdrp->sh_size - SYMTAB_MAXLEN ? 
                SYMTAB_MAXLEN / sizeof(struct Elf32_Sym) : 
                shdrp->sh_size / sizeof(struct Elf32_Sym)); i++)
        elf_symbol_info_core(&symtab[i], strtab, i);
    /* Display elf-section table information */
    elf_section_table_info(shdrp, (char *)shstrtab);
    
    /* Cut warning for elf_symbol_info */
    if (0)
        elf_symbol_info(&symtab[i], strtab, i);

    return 0;

err:
    return retval;
}
#endif

#ifdef CONFIG_ELF_SECTION_RELTEXT
#define RELTEXT_MAXLEN      128
/*
 * Parse ELF .rel.text section
 *   Relocation table section contain all relocation information.
 */
static int parse_elf_rel_text_section(struct inode *inode,
             struct elf_shdr *shdrp, const char *shstrtab)
{
    unsigned long old_fs;
    unsigned long content[RELTEXT_MAXLEN];
    char *chtab;
    int retval, i, j, k, size;

    /*
     * Load .rel.text section from ELF file.
     */
    old_fs = get_fs();
    set_fs(get_ds());
    retval = elf_read(inode, shdrp->sh_offset, (char *)content,
                 (RELTEXT_MAXLEN * sizeof(unsigned long) - shdrp->sh_size) ?
                 shdrp->sh_size : RELTEXT_MAXLEN * sizeof(unsigned long));
    set_fs(old_fs);
    if (retval < 0) {
        printk("Unable to load .rel.text from ELF by elf_read()!\n");
        goto err;
    }

    /* Display elf-section table information */
    elf_section_table_info(shdrp, (char *)shstrtab);

    /* Content */
    printk("Content:\n");
    chtab = (char *)content;
    size = ((RELTEXT_MAXLEN - shdrp->sh_size / 4 ? shdrp->sh_size / 4 :
             RELTEXT_MAXLEN) / 4) ? ((RELTEXT_MAXLEN - shdrp->sh_size / 4 ?
             shdrp->sh_size / 4 : RELTEXT_MAXLEN) / 4) : 1;
    for (i = 0; i < size; i++) {
        printk("%04x  ", i * 16);
        for (j = 0; j < (size == 1 ? 1 : 4); j++)
            printk("%08x ", (unsigned int)content[i * 4 + j]);
        for (k = 0; k < (size == 1 ? 4 : 16); k++)
            if (isalnum((unsigned char)chtab[i * 16 + k]) ||
                isalpha((unsigned char)chtab[i * 16 + k]))
                printk("%c", chtab[i * 16 + k]);
            else
                printk(".");
        printk("\n");
    }
    printk("\n");

    return 0;

err:
    return retval;
}
#endif

#ifdef CONFIG_ELF_SECTION_TABLE
/*
 * find special section by name
 */
static struct elf_shdr *elf_find_section_by_name(struct elfhdr *eh,
               struct elf_shdr *shtab, const char *name, char *shstrtab)
{
    int i;

    for (i = 1; i < eh->e_shnum; i++)
        if (strcmp(&shstrtab[shtab[i].sh_name], name) == 0)
            return &shtab[i];
    return NULL;
}

/*
 * Parse section flags
 */
static void parse_section_flags(unsigned int flags)
{
    /* SHF_WRITE: writeable */
    if ((flags & 0x6) == 0x6)
        printk(" AX ");
    else if ((flags & 0x3) == 0x3)
        printk(" WA ");
    else if ((flags & 0x2) == 0x2)
        printk("  A ");
    else if ((flags & 0x1) == 0x1)
        printk("  W ");
    else if ((flags & 0x4) == 0x4)
        printk("  X ");
    else
        printk("    ");
}

/*
 * Parse section type
 */
static void parse_section_type(unsigned int type)
{
    switch (type) {
    case 0:
        printk("NULL            ");
        break;
    case 1:
        printk("PROGBITS        ");
        break;
    case 2:
        printk("SYMTAB          ");
        break;
    case 3:
        printk("STRTAB          ");
        break;
    case 4:
        printk("RELA            ");
        break;
    case 5:
        printk("HASH            ");
        break;
    case 6:
        printk("DYNAMIC         ");
        break;
    case 7:
        printk("NOTE            ");
        break;
    case 8:
        printk("NOBITS          ");
        break;
    case 9:
        printk("REL             ");
        break;
    case 10:
        printk("SHLIB           ");
        break;
    case 11:
        printk("DNYSYM          ");
        break;
    default:
        printk("XXXXXX          ");
        break;
    }
}

static void elf_section_table_info_header(void)
{
    printk("Name              Type            Addr     Off    "
           "Size   ES Flg Lk Inf Al\n");
}

static int elf_section_table_info_core(struct elf_shdr *shdrp, char *shstrtab)
{
    printk("%-18s", &shstrtab[shdrp->sh_name]);
    parse_section_type(shdrp->sh_type);
    printk("%08x ", shdrp->sh_addr);
    printk("%06x ", shdrp->sh_offset);
    printk("%06x ", shdrp->sh_size);
    printk("00 ");
    parse_section_flags(shdrp->sh_flags);
    printk("%2d ", shdrp->sh_link);
    printk("%3d ", shdrp->sh_info);
    printk("%2d\n", shdrp->sh_addralign);
    return 0;
}

/*
 * Display ELF section table information
 */
static int elf_section_table_info(struct elf_shdr *shdrp, char *shstrtab)
{
    elf_section_table_info_header();
    elf_section_table_info_core(shdrp, shstrtab);
    return 0;
}

/*
 * Symbol 
 */

/* symbol type */
static void elf_symbol_type(struct Elf32_Sym *sym)
{
    switch (sym->st_info & 0xf) {
    case 0:
        printk("NOTYPE  ");
        break;
    case 1:
        printk("OBJECT  ");
        break;
    case 2:
        printk("FUNC    ");
        break;
    case 3:
        printk("SECTION ");
        break;
    case 4:
        printk("FILE    ");
        break;
    default:
        printk("XXXXX   ");
        break;
    }
}

/* Symbol bind information */
static void elf_symbol_bind(struct Elf32_Sym *sym)
{
    switch ((sym->st_info >> 4) & 0xF) {
    case 0:
        printk("LOCAL  ");
        break;
    case 1:
        printk("GLOBAL ");
        break;
    case 2:
        printk("WEAK   ");
        break;
    default:
        printk("       ");
        break;
    }
}

/*
 * The section number that symbol locate in.
 */
static void elf_symbol_section_nr(struct Elf32_Sym *sym)
{
    if (sym->st_shndx == 0xfffffff1)
        printk("ABS ");
    else if (sym->st_shndx == 0xfffffff2)
        printk("COM ");
    else
        printk("%3d ", sym->st_shndx);
}

/* Symbol name */
static void elf_symbol_name(struct Elf32_Sym *sym, char *strtab)
{
    if (sym->st_name == 0)
        printk("\n");
    else
        printk("%s\n", &strtab[sym->st_name]);
}

static void elf_symbol_info_header(void)
{
    printk("   Num:    Value  Size Type    Bind   Vis      Ndx Name\n");
}

static int elf_symbol_info_core(struct Elf32_Sym *sym, char *strtab, int nr)
{
    printk("%6d: ", nr);
    printk("%08x  ", sym->st_value);
    printk("%4d ", sym->st_size);
    elf_symbol_type(sym);
    elf_symbol_bind(sym);
    printk("DEFAULT  ");
    elf_symbol_section_nr(sym);
    elf_symbol_name(sym, strtab);
    return 0;
}

static int elf_symbol_info(struct Elf32_Sym *sym, char *strtab, int nr)
{
    elf_symbol_info_header();
    elf_symbol_info_core(sym, strtab, nr);
    return 0;
}

#define SHSTRTAB_LEN    256
/*
 * Parse ELF Section Table.
 */
static int parse_elf_section_table(struct linux_binprm *bprm)
{
    unsigned long old_fs;
    struct elf_shdr shdr[20];
    struct elf_shdr *shdrp, *strtabp;
    struct elfhdr *eh = (struct elfhdr *)bprm->buf;
    int retval;
    int index;
    char shstrtab[SHSTRTAB_LEN];

    /* 
     * Read ELF section table 
     *   e_shoff: Section table offset in ELF file.
     *   e_shnum: Section table number.
     */
    old_fs = get_fs();
    set_fs(get_ds());
    retval = elf_read(bprm->inode, eh->e_shoff, (char *)shdr, 
                          sizeof(struct elf_shdr) * eh->e_shnum);
    set_fs(old_fs < 0);
    if (retval < 0) {
        printk("Unable to read Section Table from ELF file\n");
        goto err;
    }

    /* Obtain .strtab table entry */
    index = eh->e_shstrndx;
    shdrp = &shdr[index];
    set_fs(get_ds());
    retval = elf_read(bprm->inode, shdrp->sh_offset, shstrtab,
                      shdrp->sh_size);
    set_fs(old_fs);
    if (retval < 0) {
        printk("Unable to read .shstrtab from ELF file\n");
        goto err;
    }

    /* Loading .strtab section header */
    strtabp = elf_find_section_by_name(eh, shdr, ".strtab", shstrtab);
    if (!strtabp) {
        printk("Unable to read .strtab section head from ELF file.\n");
        goto err;
    }

#ifdef CONFIG_ELF_SECTION_CODE
    shdrp = elf_find_section_by_name(eh, shdr, ".code", shstrtab);
    if (!shdrp) {
        /* Unable obtain '.code' section and try '.text' section */
        shdrp = elf_find_section_by_name(eh, shdr, ".text", shstrtab);
        if (!shdrp) {
            printk("Unable obtain .code section from ELF file.\n");
            goto err;
        }
    }
    parse_elf_code_section(bprm->inode, shdrp, shstrtab);
#endif // Code Section

#ifdef CONFIG_ELF_SECTION_DATA
    shdrp = elf_find_section_by_name(eh, shdr, ".data", shstrtab);
    if (!shdrp) {
        printk("Unable obtain .data section from ELF file.\n");
        goto err;
    }
    parse_elf_data_section(bprm->inode, shdrp, shstrtab);
#endif // Data Section

#ifdef CONFIG_ELF_SECTION_BSS
    shdrp = elf_find_section_by_name(eh, shdr, ".bss", shstrtab);
    if (!shdrp) {
        printk("Unable obtain .bss section from ELF file.\n");
        goto err;
    } else
        parse_elf_bss_section(bprm->inode, shdrp, shstrtab);
#endif // BSS Section

#ifdef CONFIG_ELF_SECTION_RODATA
    shdrp = elf_find_section_by_name(eh, shdr, ".rodata", shstrtab);
    if (!shdrp) {
        printk("Unable obtain .rodata section from ELF file.\n");
        goto err;
    } else
        parse_elf_rodata_section(bprm->inode, shdrp, shstrtab);
#endif // BSS Section

#ifdef CONFIG_ELF_SECTION_COMMENT
    shdrp = elf_find_section_by_name(eh, shdr, ".comment", shstrtab);
    if (!shdrp) {
        printk("Unable obtain .rodata section from ELF file.\n");
        goto err;
    } else
        parse_elf_comment_section(bprm->inode, shdrp, shstrtab);
#endif // Comment Section

#ifdef CONFIG_ELF_SECTION_SYMTAB
    shdrp = elf_find_section_by_name(eh, shdr, ".symtab", shstrtab);
    if (!shdrp) {
        printk("Unable obtain .symtab section from ELF file.\n");
        goto err;
    } else
        parse_elf_symtab_section(bprm->inode, shdrp, shstrtab, strtabp);
#endif // Symtab Section

#ifdef CONFIG_ELF_SECTION_STRTAB
    shdrp = elf_find_section_by_name(eh, shdr, ".strtab", shstrtab);
    if (!shdrp) {
        printk("Unable to .strtab section table\n");
        goto err;
    }
    parse_elf_strtab_section(bprm->inode, shdrp, shstrtab);
#endif // Strtab section

#ifdef CONFIG_ELF_SECTION_RELTEXT
    shdrp = elf_find_section_by_name(eh, shdr, ".rel.text", shstrtab);
    if (!shdrp) {
        printk("Unable to .rel.text section table\n");
        goto err;
    }
    parse_elf_rel_text_section(bprm->inode, shdrp, shstrtab);
#endif // relocation table section

#ifdef CONFIG_ELF_SECTION_SHSTRTAB
    parse_elf_shstrtab_section(bprm->inode, shdrp);
#endif

    return 0;
err:
    return retval;
}
#endif

/* Read elf file from VFS  */
static int elf_read(struct inode *inode, unsigned long offset,
                    char *addr, unsigned long count)
{
    struct file file;
    int result = -ENOEXEC;

    if (!inode->i_op || !inode->i_op->default_file_ops)
        goto end_readexec;

    file.f_mode  = 1;
    file.f_flags = 0;
    file.f_count = 1;
    file.f_inode = inode;
    file.f_pos   = 0;
    file.f_reada = 0;
    file.f_op    = inode->i_op->default_file_ops;
    if (file.f_op->open)
        if (file.f_op->open(inode, &file))
            goto end_readexec;
    if (!file.f_op || !file.f_op->read)
        goto close_readexec;
    if (file.f_op->lseek) {
        if (file.f_op->lseek(inode, &file, offset, 0) != offset)
            goto close_readexec;
    } else
        file.f_pos = offset;
    if (get_fs() == USER_DS) {
        result = verify_area(VERIFY_WRITE, addr, count);
        if (result)
            goto close_readexec;
    }
    result = file.f_op->read(inode, &file, addr, count);
    
close_readexec:
    if (file.f_op->release)
        file.f_op->release(inode, &file);
end_readexec:
    return result;
}

/*
 * Load specical EXEC file.
 */
static int do_elf(char *filename, char **argv, char **envp, 
                  struct pt_regs *regs)
{
    struct linux_binprm bprm;
    unsigned long old_fs;
    int retval = 0;

    retval = open_namei(filename, 0, 0, &bprm.inode, NULL);
    if (retval)
        return retval;
    if (!S_ISREG(bprm.inode->i_mode)) {
        retval = -EACCES;
        goto elf_error2; 
    }
    if (!bprm.inode->i_sb) {
        retval = -EACCES;
        goto elf_error2;
    }
#ifdef CONFIG_PARSE_ELF_HEADER
    /* Read Execute-file */
    memset(bprm.buf, 0, sizeof(bprm.buf));
    old_fs = get_fs();
    set_fs(get_ds());
    retval = elf_read(bprm.inode, 0, bprm.buf, 128);
    set_fs(old_fs);
    if (retval < 0)
        goto elf_error2;
    parse_elf_header((struct elfhdr *)bprm.buf);
#endif
#ifdef CONFIG_ELF_PRIVATE_SECTION_DATA
    bis_data_section();
#endif
#ifdef CONFIG_ELF_PRIVATE_SECTION_FUNC
    bis_static_func();
    bis_extern_func();
#endif
#ifdef CONFIG_ELF_SECTION_TABLE
    /* Read Execute-file */
    memset(bprm.buf, 0, sizeof(bprm.buf));
    old_fs = get_fs();
    set_fs(get_ds());
    retval = elf_read(bprm.inode, 0, bprm.buf, 128);
    set_fs(old_fs);
    if (retval < 0)
        goto elf_error2;
    parse_elf_section_table(&bprm);
#endif

    /* Cut warning */
    if (0) {
        printk("Hello World %d\n", (int)old_fs);
    }

elf_error2:
    iput(bprm.inode);

    return retval;
}

#ifdef CONFIG_ELF_SPECIAL_SYMBOL
/*
 * Extern symbol defined from ld.
 *   see arch/x86/kernel/vmlinux.lds.S
 */
static int elf_special_link_symbol(void)
{
    /* Start address of program */
    extern char __executable_start[];
    /* End address of code. */
    extern char __etext[], _etext[], etext[];
    /* End address of data. */
    extern char __edata[], _edata[], edata[];
    /* End address of program */
    extern char __end[], _end[], end[];

    printk("Program start Address: %#x\n", (unsigned int)__executable_start);
    printk("Code End Address:      %#x:%#x:%#x\n", 
            (unsigned int)__etext, (unsigned int)_etext, (unsigned int)etext);
    printk("Data End Address:      %#x:%#x:%#x\n", (unsigned int)__edata, 
                       (unsigned int)_edata, (unsigned int)edata);
    printk("Program end Address:   %#x:%#x:%#x\n", (unsigned int)__end,
                       (unsigned int)_end, (unsigned int)end);
    return 0;
}
#endif

/* Common system call entry */
int sys_d_parse_elf(struct pt_regs regs)
{
    int error;
    char *filename;

    error = getname((char *)regs.ebx, &filename);
    if (error)
        return error;
    error = do_elf(filename, (char **)regs.ecx, (char **)regs.edx, &regs);
    putname(filename);

#ifdef CONFIG_ELF_SPECIAL_SYMBOL
    elf_special_link_symbol();
#endif

    /* Cut warning */
    if (0) {
        elf_read(NULL, 0, NULL, 0);
    }
    return error;
}

/* Invoke by system call: int $0x80 */
int debug_binary_elf_format(void)
{
#ifdef CONFIG_ELF_RELOCATABLE_FILE
    d_parse_elf("/bin/demo.o", NULL, NULL);
#endif
    return 0;
}
