#ifndef _A_OUT_H
#define _A_OUT_H

#define __GNU_EXEC_MACROS__

struct exec {
    unsigned long a_magic;  /* Use macros N_MAGIC, etc for access */
    unsigned a_text;        /* length of text, in bytes */
    unsigned a_data;        /* length of data, in bytes */
    unsigned a_bss; /* length of uninitialized data area for file, in bytes*/
    unsigned a_syms;        /* length of symbol table data in file, in bytes */
    unsigned a_entry;       /* start address */
    unsigned a_trsize;      /* length of relocation info for text, in bytes */
    unsigned a_drsize;      /* length of relocation info for data, in bytes */
};

/* Address of data segment in memory after it is loaded.
   Note that it is up to you to define SEGMENT_SIZE
   on machines not listed here. */
#if defined(vax) || defined(hp300) || defined(pyr)
#define SEGMENT_SIZE PAGE_SIZE
#endif
#ifdef hp300
#define PAGE_SIZE    4096
#endif
#ifdef sony
#define SEGMENT_SIZE 0x2000
#endif /* Sony. */
#ifdef is68k
#define SEGMENT_SIZE 0x20000
#endif
#if defined(m68k) && defined(PORTAR)
#define PAGE_SIZE    0x4000
#define SEGMENT_SIZE PAGE_SIZE
#endif

#define PAGE_SIZE    4096
#define SEGMENT_SIZE 1024

#ifndef N_MAGIC
#define N_MAGIC(exec) ((exec).a_magic)
#endif

#ifndef OMAGIC
/* Code indicating object file or impure executable. */
#define OMAGIC    0407
/* Code indicating pure executable. */
#define NMAGIC    0410
/* Code indicating demand-paged executable. */
#define ZMAGIC    0413
#endif /* not OMAGIC */

#define _N_HDROFF(x) (SEGMENT_SIZE - sizeof(struct exec))

#ifndef N_TXTOFF
#define N_TXTOFF(x)  \
 (N_MAGIC(x) == ZMAGIC ? _N_HDROFF((x)) + \
 sizeof(struct exec) : sizeof(struct exec))
#endif

#ifndef N_DATOFF
#define N_DATOFF(x) (N_TXTOFF(x) + (x).a_text)
#endif

#endif
