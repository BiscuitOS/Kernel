/*
 * Data segment on Userland virtual space
 *
 * (C) 2018.11.12 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>


#define ARRAY_LEN      4

#ifdef CONFIG_DEBUG_VA_USER_DATA_GNINIT
/* Global un-initialize data */
char  GUnInit_char;
short GUnInit_short;
int   GUnInit_int;
long  GUnInit_long;
char  GUnInitA_char[ARRAY_LEN];
short GUnInitA_short[ARRAY_LEN];
int   GUnInitA_int[ARRAY_LEN];
long  GUnInitA_long[ARRAY_LEN];
char  *GUnInitP_char;
short *GUnInitP_short;
int   *GUnInitP_int;
long  *GUnInitP_long;
#endif

#ifdef CONFIG_DEBUG_VA_USER_DATA_GINITZERO
/* Global inited zero data */
char  GInitZero_char  = 0;
short GInitZero_short = 0;
int   GInitZero_int   = 0;
long  GInitZero_long  = 0;
char  GInitZeroA_char[ARRAY_LEN]  = { 0, 0, 0, 0};
short GInitZeroA_short[ARRAY_LEN] = { 0, 0, 0, 0};
int   GInitZeroA_int[ARRAY_LEN]   = { 0, 0, 0, 0};
long  GInitZeroA_long[ARRAY_LEN]  = { 0, 0, 0, 0};
char  *GInitZeroP_char  = NULL;
short *GInitZeroP_short = NULL;
int   *GInitZeroP_int   = NULL;
long  *GInitZeroP_long  = NULL;
#endif

#ifdef CONFIG_DEBUG_VA_USER_DATA_GINITNZERO
/* Global inited non-zero data */
char  GInitNZero_char  = 'A';
short GInitNZero_short = 0x10;
int   GInitNZero_int   = 0x20;
long  GInitNZero_long  = 0x30;
char  GInitNZeroA_char[ARRAY_LEN]  = { 'A', 'B', 'C', 'D'};
short GInitNZeroA_short[ARRAY_LEN] = { 0x1, 0x2, 0x3, 0x5};
int   GInitNZeroA_int[ARRAY_LEN]   = { 0x2, 0x3, 0x4, 0x5};
long  GInitNZeroA_long[ARRAY_LEN]  = { 0x3, 0x4, 0x5, 0x6};
char  *GInitNZeroP_char  = &GInitNZero_char;
short *GInitNZeroP_short = &GInitNZero_short;
int   *GInitNZeroP_int   = &GInitNZero_int;
long  *GInitNZeroP_long  = &GInitNZero_long;
#endif

#ifdef CONFIG_DEBUG_VA_USER_DATA_SNINIT
/* Static un-initialize data */
static char  SUnInit_char;
static short SUnInit_short;
static int   SUnInit_int;
static long  SUnInit_long;
static char  SUnInitA_char[ARRAY_LEN];
static short SUnInitA_short[ARRAY_LEN];
static int   SUnInitA_int[ARRAY_LEN];
static long  SUnInitA_long[ARRAY_LEN];
static char  *SUnInitP_char;
static short *SUnInitP_short;
static int   *SUnInitP_int;
static long  *SUnInitP_long;
#endif

#ifdef CONFIG_DEBUG_VA_USER_DATA_SINITZERO
/* Static inited zero data */
static char  SInitZero_char  = 0;
static short SInitZero_short = 0;
static int   SInitZero_int   = 0;
static long  SInitZero_long  = 0;
static char  SInitZeroA_char[ARRAY_LEN]  = { 0, 0, 0, 0};
static short SInitZeroA_short[ARRAY_LEN] = { 0, 0, 0, 0};
static int   SInitZeroA_int[ARRAY_LEN]   = { 0, 0, 0, 0};
static long  SInitZeroA_long[ARRAY_LEN]  = { 0, 0, 0, 0};
static char  *SInitZeroP_char  = NULL;
static short *SInitZeroP_short = NULL;
static int   *SInitZeroP_int   = NULL;
static long  *SInitZeroP_long  = NULL;
#endif

#ifdef CONFIG_DEBUG_VA_USER_DATA_SINITNZERO
/* Static inited non-zero data */
static char  SInitNZero_char  = 'A';
static short SInitNZero_short = 0x10;
static int   SInitNZero_int   = 0x20;
static long  SInitNZero_long  = 0x30;
static char  SInitNZeroA_char[ARRAY_LEN]  = { 'A', 'B', 'C', 'D'};
static short SInitNZeroA_short[ARRAY_LEN] = { 0x1, 0x2, 0x3, 0x5};
static int   SInitNZeroA_int[ARRAY_LEN]   = { 0x2, 0x3, 0x4, 0x5};
static long  SInitNZeroA_long[ARRAY_LEN]  = { 0x3, 0x4, 0x5, 0x6};
static char  *SInitNZeroP_char  = &SInitNZero_char;
static short *SInitNZeroP_short = &SInitNZero_short;
static int   *SInitNZeroP_int   = &SInitNZero_int;
static long  *SInitNZeroP_long  = &SInitNZero_long;
#endif

int main()
{
    /* Defined on ld-scripts */
    char stack_bsp;
    extern char __executable_start[];
    extern char edata[];
    extern char etext[];
    extern char end[];
    char *heap_base;
    unsigned long *mmap_base = NULL;
    int mmap_fd;

    heap_base = (char *)malloc(sizeof(char));
    mmap_fd = open("/dev/mem", O_RDONLY);
    if (mmap_fd > 0)
        mmap_base = (unsigned long *)mmap(0, 4096, PROT_READ, MAP_SHARED,
                                                  mmap_fd, 50000000);

#ifdef CONFIG_DEBUG_VA_USER_DATA_LSNINIT
    /* Local static un-initialize data */
    static char  LSUnInit_char;
    static short LSUnInit_short;
    static int   LSUnInit_int;
    static long  LSUnInit_long;
    static char  LSUnInitA_char[ARRAY_LEN];
    static short LSUnInitA_short[ARRAY_LEN];
    static int   LSUnInitA_int[ARRAY_LEN];
    static long  LSUnInitA_long[ARRAY_LEN];
    static char  *LSUnInitP_char;
    static short *LSUnInitP_short;
    static int   *LSUnInitP_int;
    static long  *LSUnInitP_long;
#endif

#ifdef CONFIG_DEBUG_VA_USER_DATA_LSINITZERO
    /* Local static inited zero data */
    static char  LSInitZero_char  = 0;
    static short LSInitZero_short = 0;
    static int   LSInitZero_int   = 0;
    static long  LSInitZero_long  = 0;
    static char  LSInitZeroA_char[ARRAY_LEN]  = { 0, 0, 0, 0};
    static short LSInitZeroA_short[ARRAY_LEN] = { 0, 0, 0, 0};
    static int   LSInitZeroA_int[ARRAY_LEN]   = { 0, 0, 0, 0};
    static long  LSInitZeroA_long[ARRAY_LEN]  = { 0, 0, 0, 0};
    static char  *LSInitZeroP_char  = NULL;
    static short *LSInitZeroP_short = NULL;
    static int   *LSInitZeroP_int   = NULL;
    static long  *LSInitZeroP_long  = NULL;
#endif

#ifdef CONFIG_DEBUG_VA_USER_DATA_LSINITNZERO
    /* Local static inited non-zero data */
    static char  LSInitNZero_char  = 'A';
    static short LSInitNZero_short = 0x10;
    static int   LSInitNZero_int   = 0x20;
    static long  LSInitNZero_long  = 0x30;
    static char  LSInitNZeroA_char[ARRAY_LEN]  = { 'A', 'B', 'C', 'D'};
    static short LSInitNZeroA_short[ARRAY_LEN] = { 0x1, 0x2, 0x3, 0x5};
    static int   LSInitNZeroA_int[ARRAY_LEN]   = { 0x2, 0x3, 0x4, 0x5};
    static long  LSInitNZeroA_long[ARRAY_LEN]  = { 0x3, 0x4, 0x5, 0x6};
    static char  *LSInitNZeroP_char  = &LSInitNZero_char;
    static short *LSInitNZeroP_short = &LSInitNZero_short;
    static int   *LSInitNZeroP_int   = &LSInitNZero_int;
    static long  *LSInitNZeroP_long  = &LSInitNZero_long;
#endif

#ifdef CONFIG_DEBUG_VA_USER_DATA_LNINIT
    /* Local un-initialize data */
    char  LUnInit_char;
    short LUnInit_short;
    int   LUnInit_int;
    long  LUnInit_long;
    char  LUnInitA_char[ARRAY_LEN];
    short LUnInitA_short[ARRAY_LEN];
    int   LUnInitA_int[ARRAY_LEN];
    long  LUnInitA_long[ARRAY_LEN];
    char  LUnInitP_char;
    short LUnInitP_short;
    int   LUnInitP_int;
    long  LUnInitP_long;
#endif

#ifdef CONFIG_DEBUG_VA_USER_DATA_LINITZERO
    /* Local inited zero data */
    char  LInitZero_char  = 0;
    short LInitZero_short = 0;
    int   LInitZero_int   = 0;
    long  LInitZero_long  = 0;
    char  LInitZeroA_char[ARRAY_LEN]  = { 0, 0, 0, 0};
    short LInitZeroA_short[ARRAY_LEN] = { 0, 0, 0, 0};
    int   LInitZeroA_int[ARRAY_LEN]   = { 0, 0, 0, 0};
    long  LInitZeroA_long[ARRAY_LEN]  = { 0, 0, 0, 0};
    char  *LInitZeroP_char  = NULL;
    short *LInitZeroP_short = NULL;
    int   *LInitZeroP_int   = NULL;
    long  *LInitZeroP_long  = NULL;
#endif

#ifdef CONFIG_DEBUG_VA_USER_DATA_LINITNZERO
    /* Local inited non-zero data */
    char  LInitNZero_char  = 'A';
    short LInitNZero_short = 0x10;
    int   LInitNZero_int   = 0x20;
    long  LInitNZero_long  = 0x30;
    char  LInitNZeroA_char[ARRAY_LEN]  = { 'A', 'B', 'C', 'D'};
    short LInitNZeroA_short[ARRAY_LEN] = { 0x1, 0x2, 0x3, 0x5};
    int   LInitNZeroA_int[ARRAY_LEN]   = { 0x2, 0x3, 0x4, 0x5};
    long  LInitNZeroA_long[ARRAY_LEN]  = { 0x3, 0x4, 0x5, 0x6};
    char  *LInitNZeroP_char  = &LInitNZero_char;
    short *LInitNZeroP_short = &LInitNZero_short;
    int   *LInitNZeroP_int   = &LInitNZero_int;
    long  *LInitNZeroP_long  = &LInitNZero_long;
#endif

#ifdef CONFIG_DEBUG_VA_USER_DATA_HEAP
    /* Data on Heap */
    char  *HP_char  = NULL;
    short *HP_short = NULL;
    int   *HP_int   = NULL;
    long  *HP_long  = NULL;

    HP_char  = (char *)malloc(sizeof(char));
    HP_short = (short *)malloc(sizeof(short));
    HP_int   = (int *)malloc(sizeof(int));
    HP_long  = (long *)malloc(sizeof(int));
#endif

#ifdef CONFIG_DEBUG_VA_USER_DATA_MMAP
    /* Data on MMAP */
    char *MP_char   = NULL;
    short *MP_short = NULL;
    int   *MP_int   = NULL;
    long  *MP_long  = NULL;

    MP_char  = (char *)((unsigned long)mmap_base + 0);
    MP_short = (short *)((unsigned long)mmap_base + 2);
    MP_int   = (int *)((unsigned long)mmap_base + 4);
    MP_long  = (long *)((unsigned long)mmap_base + 8);
#endif

    printf("***************************************************************\n");
    printf("Data Segment: .data Describe\n\n");
    printf("+---+-------+-------+------+--+------+-+------+--------------+\n");
    printf("|   |       |       |      |  |      | |      |              |\n");
    printf("|   | .text | .data | .bss |  | Heap | | Mmap |        Stack |\n");
    printf("|   |       |       |      |  |      | |      |              |\n");
    printf("+---+-------+-------+------+--+------+-+------+--------------+\n");
    printf("0                                                           4G\n");
    printf("Executable start: %#08lx\n", (unsigned long)__executable_start);
    printf("Data Range:       %#08lx -- %#08lx\n", (unsigned long)etext, 
                                                   (unsigned long)edata);
    printf("BSS  Range:       %#08lx -- %#08lx\n", (unsigned long)edata, 
                                                   (unsigned long)end);
    printf("Heap Base:        %#08lx\n", (unsigned long)heap_base);
    printf("Mmap Base:        %#08lx\n", (unsigned long)mmap_base);
    printf("Stack:            %#08lx\n", (unsigned long)&stack_bsp);
    printf("***************************************************************\n");

#ifdef CONFIG_DEBUG_VA_USER_DATA_GNINIT
    printf("\nGlobal Un-Initialize Data:\n"
           "[*]char   GUnInit_char              address: %#lx o\n" 
           "[*]short  GUnInit_short             address: %#lx |\n" 
           "[*]int    GUnInit_int               address: %#lx |\n" 
           "[*]long   GUnInit_long              address: %#lx |\n" 
           "[*]char   GUnInitA_char[ARRAY_LEN]  address: %#lx |\n" 
           "[*]short  GUnInitA_short[ARRAY_LEN] address: %#lx |\n" 
           "[*]int    GUnInitA_int[ARRAY_LEN]   address: %#lx |\n" 
           "[*]long   GUnInitA_long[ARRAY_LEN]  address: %#lx |\n" 
           "[*]char  *GUnInitP_char             address: %#lx |\n" 
           "[*]short *GUnInitP_short            address: %#lx |\n" 
           "[*]int   *GUnInitP_int              address: %#lx |\n" 
           "[*]long  *GUnInitP_long             address: %#lx V\n", 
           (unsigned long)&GUnInit_char,
           (unsigned long)&GUnInit_short,
           (unsigned long)&GUnInit_int,
           (unsigned long)&GUnInit_long,
           (unsigned long)GUnInitA_char,
           (unsigned long)GUnInitA_short,
           (unsigned long)GUnInitA_int,
           (unsigned long)GUnInitA_long,
           (unsigned long)&GUnInitP_char,
           (unsigned long)&GUnInitP_short,
           (unsigned long)&GUnInitP_int,
           (unsigned long)&GUnInitP_long);
#endif

#ifdef CONFIG_DEBUG_VA_USER_DATA_GINITZERO
    printf("\nGlobal Inited Zero Data:\n"
           "[*]char   GInitZero_char              address: %#lx o\n"
           "[*]short  GInitZero_short             address: %#lx |\n"
           "[*]int    GInitZero_int               address: %#lx |\n"
           "[*]long   GInitZero_long              address: %#lx |\n"
           "[*]char   GInitZeroA_char[ARRAY_LEN]  address: %#lx |\n"
           "[*]short  GInitZeroA_short[ARRAY_LEN] address: %#lx |\n"
           "[*]int    GInitZeroA_int[ARRAY_LEN]   address: %#lx |\n"
           "[*]long   GInitZeroA_long[ARRAY_LEN]  address: %#lx |\n"
           "[*]char  *GInitZeroP_char             address: %#lx |\n"
           "[*]short *GInitZeroP_short            address: %#lx |\n"
           "[*]int   *GInitZeroP_int              address: %#lx |\n"
           "[*]long  *GInitZeroP_long             address: %#lx V\n",
           (unsigned long)&GInitZero_char,
           (unsigned long)&GInitZero_short,
           (unsigned long)&GInitZero_int,
           (unsigned long)&GInitZero_long,
           (unsigned long)GInitZeroA_char,
           (unsigned long)GInitZeroA_short,
           (unsigned long)GInitZeroA_int,
           (unsigned long)GInitZeroA_long,
           (unsigned long)&GInitZeroP_char,
           (unsigned long)&GInitZeroP_short,
           (unsigned long)&GInitZeroP_int,
           (unsigned long)&GInitZeroP_long);
#endif

#ifdef CONFIG_DEBUG_VA_USER_DATA_GINITNZERO
    printf("\nGlobal Inited Non-Zero Data:\n"
           "[*]char   GInitNZero_char              address: %#lx o\n"
           "[*]short  GInitNZero_short             address: %#lx |\n"
           "[*]int    GInitNZero_int               address: %#lx |\n"
           "[*]long   GInitNZero_long              address: %#lx |\n"
           "[*]char   GInitNZeroA_char[ARRAY_LEN]  address: %#lx |\n"
           "[*]short  GInitNZeroA_short[ARRAY_LEN] address: %#lx |\n"
           "[*]int    GInitNZeroA_int[ARRAY_LEN]   address: %#lx |\n"
           "[*]long   GInitNZeroA_long[ARRAY_LEN]  address: %#lx |\n"
           "[*]char  *GInitNZeroP_char             address: %#lx |\n"
           "[*]short *GInitNZeroP_short            address: %#lx |\n"
           "[*]int   *GInitNZeroP_int              address: %#lx |\n"
           "[*]long  *GInitNZeroP_long             address: %#lx V\n",
           (unsigned long)&GInitNZero_char,
           (unsigned long)&GInitNZero_short,
           (unsigned long)&GInitNZero_int,
           (unsigned long)&GInitNZero_long,
           (unsigned long)GInitNZeroA_char,
           (unsigned long)GInitNZeroA_short,
           (unsigned long)GInitNZeroA_int,
           (unsigned long)GInitNZeroA_long,
           (unsigned long)&GInitNZeroP_char,
           (unsigned long)&GInitNZeroP_short,
           (unsigned long)&GInitNZeroP_int,
           (unsigned long)&GInitNZeroP_long);
#endif

#ifdef CONFIG_DEBUG_VA_USER_DATA_SNINIT
    printf("\nStatic Un-Initialize Data:\n"
           "[*]char   SUnInit_char              address: %#lx o\n"
           "[*]short  SUnInit_short             address: %#lx |\n"
           "[*]int    SUnInit_int               address: %#lx |\n"
           "[*]long   SUnInit_long              address: %#lx |\n"
           "[*]char   SUnInitA_char[ARRAY_LEN]  address: %#lx |\n"
           "[*]short  SUnInitA_short[ARRAY_LEN] address: %#lx |\n"
           "[*]int    SUnInitA_int[ARRAY_LEN]   address: %#lx |\n"
           "[*]long   SUnInitA_long[ARRAY_LEN]  address: %#lx |\n"
           "[*]char  *SUnInitP_char             address: %#lx |\n"
           "[*]short *SUnInitP_short            address: %#lx |\n"
           "[*]int   *SUnInitP_int              address: %#lx |\n"
           "[*]long  *SUnInitP_long             address: %#lx V\n",
           (unsigned long)&SUnInit_char,
           (unsigned long)&SUnInit_short,
           (unsigned long)&SUnInit_int,
           (unsigned long)&SUnInit_long,
           (unsigned long)SUnInitA_char,
           (unsigned long)SUnInitA_short,
           (unsigned long)SUnInitA_int,
           (unsigned long)SUnInitA_long,
           (unsigned long)&SUnInitP_char,
           (unsigned long)&SUnInitP_short,
           (unsigned long)&SUnInitP_int,
           (unsigned long)&SUnInitP_long);
#endif

#ifdef CONFIG_DEBUG_VA_USER_DATA_SINITZERO
    printf("\nStatic Inited Zero Data:\n"
           "[*]char   SInitZero_char              address: %#lx o\n"
           "[*]short  SInitZero_short             address: %#lx |\n"
           "[*]int    SInitZero_int               address: %#lx |\n"
           "[*]long   SInitZero_long              address: %#lx |\n"
           "[*]char   SInitZeroA_char[ARRAY_LEN]  address: %#lx |\n"
           "[*]short  SInitZeroA_short[ARRAY_LEN] address: %#lx |\n"
           "[*]int    SInitZeroA_int[ARRAY_LEN]   address: %#lx |\n"
           "[*]long   SInitZeroA_long[ARRAY_LEN]  address: %#lx |\n"
           "[*]char  *SInitZeroP_char             address: %#lx |\n"
           "[*]short *SInitZeroP_short            address: %#lx |\n"
           "[*]int   *SInitZeroP_int              address: %#lx |\n"
           "[*]long  *SInitZeroP_long             address: %#lx V\n",
           (unsigned long)&SInitZero_char,
           (unsigned long)&SInitZero_short,
           (unsigned long)&SInitZero_int,
           (unsigned long)&SInitZero_long,
           (unsigned long)SInitZeroA_char,
           (unsigned long)SInitZeroA_short,
           (unsigned long)SInitZeroA_int,
           (unsigned long)SInitZeroA_long,
           (unsigned long)&SInitZeroP_char,
           (unsigned long)&SInitZeroP_short,
           (unsigned long)&SInitZeroP_int,
           (unsigned long)&SInitZeroP_long);
#endif

#ifdef CONFIG_DEBUG_VA_USER_DATA_SINITNZERO
    printf("\nStatic Inited Non-Zero Data:\n"
           "[*]char   SInitNZero_char              address: %#lx o\n"
           "[*]short  SInitNZero_short             address: %#lx |\n"
           "[*]int    SInitNZero_int               address: %#lx |\n"
           "[*]long   SInitNZero_long              address: %#lx |\n"
           "[*]char   SInitNZeroA_char[ARRAY_LEN]  address: %#lx |\n"
           "[*]short  SInitNZeroA_short[ARRAY_LEN] address: %#lx |\n"
           "[*]int    SInitNZeroA_int[ARRAY_LEN]   address: %#lx |\n"
           "[*]long   SInitNZeroA_long[ARRAY_LEN]  address: %#lx |\n"
           "[*]char  *SInitNZeroP_char             address: %#lx |\n"
           "[*]short *SInitNZeroP_short            address: %#lx |\n"
           "[*]int   *SInitNZeroP_int              address: %#lx |\n"
           "[*]long  *SInitNZeroP_long             address: %#lx V\n",
           (unsigned long)&SInitNZero_char,
           (unsigned long)&SInitNZero_short,
           (unsigned long)&SInitNZero_int,
           (unsigned long)&SInitNZero_long,
           (unsigned long)SInitNZeroA_char,
           (unsigned long)SInitNZeroA_short,
           (unsigned long)SInitNZeroA_int,
           (unsigned long)SInitNZeroA_long,
           (unsigned long)&SInitNZeroP_char,
           (unsigned long)&SInitNZeroP_short,
           (unsigned long)&SInitNZeroP_int,
           (unsigned long)&SInitNZeroP_long);
#endif

#ifdef CONFIG_DEBUG_VA_USER_DATA_LSNINIT
    printf("\nLocal Static Un-Initialize Data:\n"
           "[*]char   LSUnInit_char              address: %#lx ^\n"
           "[*]short  LSUnInit_short             address: %#lx |\n"
           "[*]int    LSUnInit_int               address: %#lx |\n"
           "[*]long   LSUnInit_long              address: %#lx |\n"
           "[*]char   LSUnInitA_char[ARRAY_LEN]  address: %#lx |\n"
           "[*]short  LSUnInitA_short[ARRAY_LEN] address: %#lx |\n"
           "[*]int    LSUnInitA_int[ARRAY_LEN]   address: %#lx |\n"
           "[*]long   LSUnInitA_long[ARRAY_LEN]  address: %#lx |\n"
           "[*]char  *LSUnInitP_char             address: %#lx |\n"
           "[*]short *LSUnInitP_short            address: %#lx |\n"
           "[*]int   *LSUnInitP_int              address: %#lx |\n"
           "[*]long  *LSUnInitP_long             address: %#lx o\n",
           (unsigned long)&LSUnInit_char,
           (unsigned long)&LSUnInit_short,
           (unsigned long)&LSUnInit_int,
           (unsigned long)&LSUnInit_long,
           (unsigned long)LSUnInitA_char,
           (unsigned long)LSUnInitA_short,
           (unsigned long)LSUnInitA_int,
           (unsigned long)LSUnInitA_long,
           (unsigned long)&LSUnInitP_char,
           (unsigned long)&LSUnInitP_short,
           (unsigned long)&LSUnInitP_int,
           (unsigned long)&LSUnInitP_long);
#endif

#ifdef CONFIG_DEBUG_VA_USER_DATA_LSINITZERO
    printf("\nLocal Static Inited Zero Data:\n"
           "[*]char   LSInitZero_char              address: %#lx ^\n"
           "[*]short  LSInitZero_short             address: %#lx |\n"
           "[*]int    LSInitZero_int               address: %#lx |\n"
           "[*]long   LSInitZero_long              address: %#lx |\n"
           "[*]char   LSInitZeroA_char[ARRAY_LEN]  address: %#lx |\n"
           "[*]short  LSInitZeroA_short[ARRAY_LEN] address: %#lx |\n"
           "[*]int    LSInitZeroA_int[ARRAY_LEN]   address: %#lx |\n"
           "[*]long   LSInitZeroA_long[ARRAY_LEN]  address: %#lx |\n"
           "[*]char  *LSInitZeroP_char             address: %#lx |\n"
           "[*]short *LSInitZeroP_short            address: %#lx |\n"
           "[*]int   *LSInitZeroP_int              address: %#lx |\n"
           "[*]long  *LSInitZeroP_long             address: %#lx o\n",
           (unsigned long)&LSInitZero_char,
           (unsigned long)&LSInitZero_short,
           (unsigned long)&LSInitZero_int,
           (unsigned long)&LSInitZero_long,
           (unsigned long)LSInitZeroA_char,
           (unsigned long)LSInitZeroA_short,
           (unsigned long)LSInitZeroA_int,
           (unsigned long)LSInitZeroA_long,
           (unsigned long)&LSInitZeroP_char,
           (unsigned long)&LSInitZeroP_short,
           (unsigned long)&LSInitZeroP_int,
           (unsigned long)&LSInitZeroP_long);
#endif

#ifdef CONFIG_DEBUG_VA_USER_DATA_LSINITNZERO
    printf("\nLocal Static Inited Non-Zero Data:\n"
           "[*]char   LSInitNZero_char              address: %#lx ^\n"
           "[*]short  LSInitNZero_short             address: %#lx |\n"
           "[*]int    LSInitNZero_int               address: %#lx |\n"
           "[*]long   LSInitNZero_long              address: %#lx |\n"
           "[*]char   LSInitNZeroA_char[ARRAY_LEN]  address: %#lx |\n"
           "[*]short  LSInitNZeroA_short[ARRAY_LEN] address: %#lx |\n"
           "[*]int    LSInitNZeroA_int[ARRAY_LEN]   address: %#lx |\n"
           "[*]long   LSInitNZeroA_long[ARRAY_LEN]  address: %#lx |\n"
           "[*]char  *LSInitNZeroP_char             address: %#lx |\n"
           "[*]short *LSInitNZeroP_short            address: %#lx |\n"
           "[*]int   *LSInitNZeroP_int              address: %#lx |\n"
           "[*]long  *LSInitNZeroP_long             address: %#lx |\n",
           (unsigned long)&LSInitNZero_char,
           (unsigned long)&LSInitNZero_short,
           (unsigned long)&LSInitNZero_int,
           (unsigned long)&LSInitNZero_long,
           (unsigned long)LSInitNZeroA_char,
           (unsigned long)LSInitNZeroA_short,
           (unsigned long)LSInitNZeroA_int,
           (unsigned long)LSInitNZeroA_long,
           (unsigned long)&LSInitNZeroP_char,
           (unsigned long)&LSInitNZeroP_short,
           (unsigned long)&LSInitNZeroP_int,
           (unsigned long)&LSInitNZeroP_long);
#endif

#ifdef CONFIG_DEBUG_VA_USER_DATA_LNINIT
    printf("\nLocal Un-Initialize Data:\n"
           "[*]char   LUnInit_char              address: %#lx ^\n"
           "[*]short  LUnInit_short             address: %#lx |\n"
           "[*]int    LUnInit_int               address: %#lx |\n"
           "[*]long   LUnInit_long              address: %#lx |\n"
           "[*]char   LUnInitA_char[ARRAY_LEN]  address: %#lx |\n"
           "[*]short  LUnInitA_short[ARRAY_LEN] address: %#lx |\n"
           "[*]int    LUnInitA_int[ARRAY_LEN]   address: %#lx |\n"
           "[*]long   LUnInitA_long[ARRAY_LEN]  address: %#lx |\n"
           "[*]char  *LUnInitP_char             address: %#lx |\n"
           "[*]short *LUnInitP_short            address: %#lx |\n"
           "[*]int   *LUnInitP_int              address: %#lx |\n"
           "[*]long  *LUnInitP_long             address: %#lx o\n",
           (unsigned long)&LUnInit_char,
           (unsigned long)&LUnInit_short,
           (unsigned long)&LUnInit_int,
           (unsigned long)&LUnInit_long,
           (unsigned long)LUnInitA_char,
           (unsigned long)LUnInitA_short,
           (unsigned long)LUnInitA_int,
           (unsigned long)LUnInitA_long,
           (unsigned long)&LUnInitP_char,
           (unsigned long)&LUnInitP_short,
           (unsigned long)&LUnInitP_int,
           (unsigned long)&LUnInitP_long);
#endif

#ifdef CONFIG_DEBUG_VA_USER_DATA_LINITZERO
    printf("\nLocal Inited Zero Data:\n"
           "[*]char   LInitZero_char              address: %#lx ^\n"
           "[*]short  LInitZero_short             address: %#lx |\n"
           "[*]int    LInitZero_int               address: %#lx |\n"
           "[*]long   LInitZero_long              address: %#lx |\n"
           "[*]char   LInitZeroA_char[ARRAY_LEN]  address: %#lx |\n"
           "[*]short  LInitZeroA_short[ARRAY_LEN] address: %#lx |\n"
           "[*]int    LInitZeroA_int[ARRAY_LEN]   address: %#lx |\n"
           "[*]long   LInitZeroA_long[ARRAY_LEN]  address: %#lx |\n"
           "[*]char  *LInitZeroP_char             address: %#lx |\n"
           "[*]short *LInitZeroP_short            address: %#lx |\n"
           "[*]int   *LInitZeroP_int              address: %#lx |\n"
           "[*]long  *LInitZeroP_long             address: %#lx o\n",
           (unsigned long)&LInitZero_char,
           (unsigned long)&LInitZero_short,
           (unsigned long)&LInitZero_int,
           (unsigned long)&LInitZero_long,
           (unsigned long)LInitZeroA_char,
           (unsigned long)LInitZeroA_short,
           (unsigned long)LInitZeroA_int,
           (unsigned long)LInitZeroA_long,
           (unsigned long)&LInitZeroP_char,
           (unsigned long)&LInitZeroP_short,
           (unsigned long)&LInitZeroP_int,
           (unsigned long)&LInitZeroP_long);
#endif

#ifdef CONFIG_DEBUG_VA_USER_DATA_LINITNZERO
    printf("\nLocal Inited Non-Zero Data:\n"
           "[*]char   LInitNZero_char              address: %#lx ^\n"
           "[*]short  LInitNZero_short             address: %#lx |\n"
           "[*]int    LInitNZero_int               address: %#lx |\n"
           "[*]long   LInitNZero_long              address: %#lx |\n"
           "[*]char   LInitNZeroA_char[ARRAY_LEN]  address: %#lx |\n"
           "[*]short  LInitNZeroA_short[ARRAY_LEN] address: %#lx |\n"
           "[*]int    LInitNZeroA_int[ARRAY_LEN]   address: %#lx |\n"
           "[*]long   LInitNZeroA_long[ARRAY_LEN]  address: %#lx |\n"
           "[*]char  *LInitNZeroP_char             address: %#lx |\n"
           "[*]short *LInitNZeroP_short            address: %#lx |\n"
           "[*]int   *LInitNZeroP_int              address: %#lx |\n"
           "[*]long  *LInitNZeroP_long             address: %#lx o\n",
           (unsigned long)&LInitNZero_char,
           (unsigned long)&LInitNZero_short,
           (unsigned long)&LInitNZero_int,
           (unsigned long)&LInitNZero_long,
           (unsigned long)LInitNZeroA_char,
           (unsigned long)LInitNZeroA_short,
           (unsigned long)LInitNZeroA_int,
           (unsigned long)LInitNZeroA_long,
           (unsigned long)&LInitNZeroP_char,
           (unsigned long)&LInitNZeroP_short,
           (unsigned long)&LInitNZeroP_int,
           (unsigned long)&LInitNZeroP_long);
#endif

#ifdef CONFIG_DEBUG_VA_USER_DATA_HEAP
    printf("\nHeap Data:\n"
           "[*]char  *HP_char             address: %#lx o\n"
           "[*]short *HP_short            address: %#lx |\n"
           "[*]int   *HP_int              address: %#lx |\n"
           "[*]long  *HP_long             address: %#lx V\n",
           (unsigned long)HP_char,
           (unsigned long)HP_short,
           (unsigned long)HP_int,
           (unsigned long)HP_long);

    free(HP_char);
    free(HP_short);
    free(HP_int);
    free(HP_long);
#endif

#ifdef CONFIG_DEBUG_VA_USER_DATA_MMAP
    printf("\nMmap Data:\n"
           "[*]char  *MP_char             address: %#lx o\n"
           "[*]short *MP_short            address: %#lx |\n"
           "[*]int   *MP_int              address: %#lx |\n"
           "[*]long  *MP_long             address: %#lx V\n",
           (unsigned long)MP_char,
           (unsigned long)MP_short,
           (unsigned long)MP_int,
           (unsigned long)MP_long);
#endif

    if (mmap_base)
        munmap(mmap_base, 4096);
    if (mmap_fd > 0)
        close(mmap_fd);
    free(heap_base);

    return 0;
}

