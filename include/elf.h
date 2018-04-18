/* This file defines standard ELF types, structures, and macros.
   Copyright (C) 1995-2014 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _ELF_H
#define _ELF_H 1

#include <stdint.h>

/* Type for a 16-bit quantity */
typedef uint16_t Elf32_Half;

/* Types for signed and unsigned 32-bit quantities. */
typedef uint32_t Elf32_Word;
typedef int32_t  Elf32_Sword;

/* TRype of addresses. */
typedef uint32_t Elf32_Addr;

/* Type of file offsets */
typedef uint32_t Elf32_Off;

/* Type of section indices, which are 16-bit quantities */
typedef uint16_t Elf32_Section;

/* Type for version symbol information. */
typedef Elf32_Half Elf32_Versym;

/* The ELF file header. This appears at the start of every ELF file. */

#define EI_NIDENT    (16)

typedef struct
{
    unsigned char e_ident[EI_NIDENT];  /* Magic number and other info */
    Elf32_Half    e_type;              /* Object file type */
    Elf32_Half    e_machine;           /* Architecture */
    Elf32_Word    e_version;           /* Object file version */
    Elf32_Addr    e_entry;             /* Entry point virtual address */
    Elf32_Off     e_phoff;             /* Program header table file offset */
    Elf32_Off     e_shoff;             /* Section header table file offset */
    Elf32_Word    e_flags;             /* Processor-specific flags */
    Elf32_Half    e_ehsize;            /* ELF header size in bytes */
    Elf32_Half    e_phentsize;         /* Program header table entry size */
    Elf32_Half    e_phnum;             /* Program header table entry count */
    Elf32_Half    e_shentsize;         /* Section header table entry size */
    Elf32_Half    e_shnum;             /* Section header table entry count */
    Elf32_Half    e_shstrndx;          /* Section header string table index */
} Elf32_Ehdr;

/* Field in the e_ident array. The EI_ macros are indices into the array.
 * The macros under each EI_* macro are the values the byte may have. */

#define EI_MAG0           0            /* File identification byte 0 index */
#define ELFMAG0           0x7F         /* Magic number byte 0 */

#define EI_MAG1           1            /* File identification byte 1 index */
#define ELFMAG1           'E'          /* Magic number byte 1 */

#define EI_MAG2           2            /* File identification byte 2 index */
#define ELFMAG2           'L'          /* Magic number byte 2 */

#define EI_MAG3           3            /* File identification byte 3 index */
#define ELFMAG3           'F'          /* Magic number byte 3 */

/* Conglomeration of the identification bytes, for easy testing as a word */
#define ELFMAG            "\177ELF"
#define SELFMAG           4

#define EI_CLASS          4            /* File class byte index */
#define ELFCLASSNONE      0            /* Invalid class */
#define ELFCLASS32        1            /* 32-bit objects */
#define ELFCLASS64        2            /* 64-bit objects */
#define ELFCLASSNUM       3

#define EI_DATA           5            /* Data encoding byte idex */
#define ELFDATANONE       0            /* Invalid data encoding */
#define ELFDATA2LSB       1            /* 2's complement, little endian */
#define ELFDATA2MSB       2            /* 2's complement, big endian */
#define ELFDATANUM        3

#define EI_VERSION        6            /* File version byte index */
                                       /* Value must by EV_CURRENT */

#define EI_OSABI          7            /* OS ABI identification */
#define ELFOSABI_NONE     0            /* UNIX System V ABI */
#define ELFOSABI_SYSV     0            /* Alias. */
#define ELFOSABI_HPUX     1            /* HP-UX */
#define ELFOSABI_NETBSD   2            /* NetBSD */
#define ELFOSABI_GNU      3            /* Object uses GNU ELF extensions. */
#define ELFOSABI_LINUX    ELFOSABI_GNU /* Compatibility alisa */

#define EI_ABIVERSION     8            /* ABI version */

/* Legal values for e_type (object file type) */

#define ET_NONE           0            /* No file type */
#define ET_REL            1            /* Relocatable file */
#define ET_EXEC           2            /* Executable file */
#define ET_DYN            3            /* Shared object file */
#define ET_CORE           4            /* Core file */
#define ET_NUM            5            /* Number of defined types */

/* Legal values for e_machines (architecture). */

#define EM_NONE           0            /* No machine */
#define EM_M32            1            /* AT&T WE 32100 */
#define EM_SPARC          2            /* SUN SPARC */
#define EM_386            3            /* Intel 80386 */

/* Legal values for e_version (version) */
#define EV_NONE           0            /* Invalid ELF version */
#define EV_CURRENT        1            /* Current version */
#define EV_NUM            2

/* Section header. */

typedef struct
{
    Elf32_Word  sh_name;               /* Section name (String tbl index) */
    Elf32_Word  sh_type;               /* Section type */
    Elf32_Word  sh_flags;              /* Section flags */
    Elf32_Addr  sh_addr;               /* Section virtual addr at execution */
    Elf32_Off   sh_offset;             /* Section file offset */
    Elf32_Word  sh_size;               /* Section size in bytes */
    Elf32_Word  sh_link;               /* Link to another section */
    Elf32_Word  sh_info;               /* Additional section information */
    Elf32_Word  sh_addralign;          /* Section alignment */
    Elf32_Word  sh_entsize;            /* Entry size if section holds table */
} Elf32_Shdr;

#endif 
