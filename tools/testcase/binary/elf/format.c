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

#include <asm/segment.h>

#define __LIBRARY__
#include <linux/unistd.h>

static int elf_read(struct inode *inode, unsigned long offset,
                    char *addr, unsigned long count);

/*
 * ELF Section '.bis.data'
 *   More section see "arch/x86/kernel/vmlinux.lds.S"
 *   * Add a global initialized data into '.bis.data'
 *   * Add a global uninitialized data into '.bid.data'
 *   * Add a local static initialized data into '.bis.data'
 *   * Add a local static uninitialized data into '.bis.data'
 */
#ifdef CONFIG_ELF_SECTION_DATA
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
#ifdef CONFIG_ELF_SECTION_FUNC
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

#ifdef CONFIG_ELF_SECTION_TABLE
/*
 * Parse ELF Section Table.
 */
static int parse_elf_section_table(struct linux_binprm *bprm)
{
    unsigned long old_fs;
    struct elf_shdr shdr[20];
    struct elfhdr *eh = (struct elfhdr *)bprm->buf;
    int retval;
    int index, offset;

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
        printk("Unable to read section table from ELF file\n");
        goto err;
    }

    /* Obtain .strtab index in Section Table */

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
#ifdef CONFIG_ELF_SECTION_DATA
    bis_data_section();
#endif
#ifdef CONFIG_ELF_SECTION_FUNC
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

elf_error2:
    iput(bprm.inode);

    return retval;
}

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
