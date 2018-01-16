Paging Mechanism on X86 Architecture
--------------------------------------------------------

  `Segmentation/README.md` explains how to segmentation converts logical
  addresses to linear addresses. **Paging** (or linear-address translation)
  is the process of translating linear addresses so that they can be used
  to access memory or I/O devices. Paging translate each linear address to
  a physical address and determines, for each translation, what access
  to the linear address are allowed (the address's access rights) and the
  type of caching used for such accesses (the address's memory type).

### Paging Mode and Control Bits

  Paging behavior is controlled by following control bits:

  * The WP and PG flags in control register CR0 (bit 16 and bit 31,
    respectively).

  * The PSE, PAE, PGE, PCIDE, SMEP, SMAP, and PKE flags in control register
    CR4 (bit 4, bit 5, bit 7, bit 17, bit 20, bit 21, and bit 22).

  * The LME and NXE flags in the IA32_EFER MSR (bit 8 and bit 11).

  * The AC flag in the EFLAGS register (bit 18).

  Software enable paging by using MOV to CR0 instruction to set CR0.PG.
  Before doing so, software should ensure that control register CR3 contains
  the physical address of the first paging structure that the processor
  will use the linear-address translation and that structure is initialize
  as desire.

### Three Paging Modes

  If CR0.PG = 0, paging is not used. The logical processor treats all
  linear addresses as if they were physical address. CR4.PAE and 
  IA32_EFER are ignored by the processor, as are CR0.WP, CR4.PGE,
  CR4.SMEP, CR4.SMAP, and IA32_EFER.NXE.

  Paging is enabled if CR0.GP = 1. Paging can be enabled only if protection
  is enabled (CR0.PE = 1). If paging is enabled, one of three paging mode
  is used:   

  * If CR0.PG = 1 and CR4.PAE = 0, 32-bit paging is used. 32-bit paging
    is detailed in Section `32-bit paging`.

  * If CR0.PG = 1, CR4.PAE = 1, and IA32_EFER.LME = 0, PAE paging is used.

  * If CR0.PG = 1, CR4.PAE = 1, and IA32_EREF.LME = 1, 4-level paging is 
    used.

  The three paging mode differ with regard to the following details:

  * Linear-address width. The size of the linear addresses that can 
    translated.

  * Physical-address width. The size of the physical addresses produced
    by paging.

  * Page size. The granularity at which linear addresses are translated.
    Linear addresses on the same page are translated to corresponding
    physical addresses on the same page.

  * Support for execute-disable access rights. In some paging modes, software
    can be prevented from fetching instructions from pages that are 
    otherwise readable.

  * Support for PCIDs. With 4-level paging, software can enable a facility
    by which a logical processor caches information for multiple linear-
    address spaces. The processor may retain cached information when software
    switches between different linear-address space.

  * Support for protection keys. With 4-level paging,  software can enable a
    facility by which each linear address is associated with a protection key.
    Software can use a new control register to determine, for each protection
    keys, how software can access linear address associated with that
    protection key.

  Table 4-1 illustrates the principal differences between the three paging
  modes.

  ![Alt text](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel_hacking/testcase/mmu/TABLE_PGING_MODE.png)

### Paging-Mode Enabling

  If CR0.PG = 1, a logical processor is in one of three paging mode, depending
  on the values of CR4.PAE and IA32_EFER.LME. Figure 4-1 illustrates how 
  software can enable these modes and make transitions between them. The
  following items identify certain limitations and other details:

  ![Alt text](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel_hacking/testcase/mmu/CHG_PAGING_MODE.png)
  
  * IA32_EFER.LME cannot be modified while paging is enable (CR0.PG = 1). 
    Attempts to do so using WRMSR cause a general-protection exception (#GP).

  * Paging cannot be enabled (by setting CR0.PG to 1) while CR4.PAE = 0 and 
    IA32_EFER.LME = 1. Attempts to do so using MOV to CR0 cause a general-
    protection exception (#GP).

  * CR4.PAE cannot be cleared while 4-level paging is active (CR0.PG = 1 and
    IA32_EFER.LME = 1). Attempts to do so using MOV to CR4 cause a general-
    protection exception (#GP(0)).

  * Regardless of the current paging mode, software can disable paging by 
    clearing CR0.PG with MOV to CR0.

  * Software can make transitions between 32-bit paging and PAE paging
    by changing the value of CR4.PAE with MOV to CR4.

  * Software cannot make transitions directly between 4-level and either
    of the other two paging modes. It must first disable paging (by clearing
    CR0.PG with MOV to CR0), then set CR4.PAE and IA32_EFER.LME to the 
    desired values (with MOV to CR4 and WRMSR), and then re-enable paging
    (by setting CR0.PG with MOV to CR0). As noted earlier, an attempt to
    clear either CR4.PAE or IA32_EFER.LME cause a general-protection
    exception (#GP(0)).

### Hierarchical Paging Structures: An Overview

  All three paging modes translate linear addresses using hierarchical paging
  structures. This section provides an overview of their operation.

  Every paging structure is 4096 Bytes in size and comprises a number of
  individual entries. With 32-bit paging, each entry is 32 bits (4 Bytes).
  There are thus 1024 entires in each structure. With PAE paging and 4-level
  paging, each entry is 64 bits (8 bytes). There are thus 512 entries in each
  structure. (PAE paging includes one exception, a paging structure that is
  32 bytes in size, containing 4 64-bit entries.)

  The processor uses the upper portion of a linear address to identify a 
  series of paging-structure entries. The last of these entries identifies
  the physical address of the region to which the linear address translates (
  called the page frame). The lower portion of the linear address (called
  the page offset) identifies the specific address within that region to 
  which the linear address translates.

  Each paging-struct entry contains a physical address, which is either the
  address of another paging structure or the address of a page frame. In the
  first case, the entry is entry is said to reference the other paging 
  structure. In the latter, the entry is said to map a page.

  The first paging structure used for any translation is located at the
  physical address in CR3. A linear address is translated using the following
  iterative procedure. A portion of the linear address (initially the 
  uppermost bits) selects an entry in a paging structure (initially the one 
  location using CR3). If that entry references another paging structure,
  the process continues with that paging structure and with the portion of 
  the linear address immediately below that just used. If instead the entry
  maps a page, the process completes: the physical address in the entry is
  that of the page frame and the remanining lower portion of the linear 
  address is the page offset.

  The following items give an example for each of the three paging mode (each
  example locates a 4-KByte page frame):

  * With 32-bit paging, each paging struct comprises 1024 = 2^10 entries.
    For this reson, the translation processor uses 10 bit at a time from a
    32-bit linear address. Bits 31:22 identify the first paging-structure
    entry and bits 21:12 identify a second. The latter identifies the page
    frame. Bits 11:0 of the linear address are the page offset within the
    4-Kbyte page frame. (See Figure 4-2 for an illustration.) 

  * With PAE paging, the first paging structure comprises only 4 = 2^2 entires.
    Translation thus begins by using bit 31:30 from a 32-bit linear address
    to identify the first paging-structure entry. Other paging structures
    comprise 512 = 2^9 entries, so the process continues by using 9 bit at a
    time. Bits 29:21 identify a second paging-struct entry and bits 20:12
    identify a third. This last identifies the page frame. (See Figure 4-5
    for all illustration.)

  * With 4-level paging, each paging struct comprises 512 = 2^9 entires and 
    translation uses 9 bits at a time from a 48-bit linear address. Bits 47:39
    identify the first paging-structure entry, bits 38:30 identify a second,
    bits 29:21 a third, and bits 20:12 identify a fourth. Again, the last
    identifies the page frame. (See Figure 4-8 for a illustration.)

  The translation process in each of the examples above completes by 
  identifying a page frame. The page frame is part of the translation of the 
  original linear address. In some cases, however, the paging structures may
  by configured so that the translation process terminates before identifying
  a page frame. This occurs if the process encounters a paging-structure
  entry that is marked `not present` (because its P flag -- bit 0 -- is clear)
  or in which a reserved bit is set. In this case, there is no translation 
  for the linear address. an access to that address causes a page-fault
  exception.

  In the example above, a paging-structure entry maps a page with a 4-KByte
  page frame when only 12 bits remain in the linear address. Entries
  identified earlier always reference other paging structures. That may not
  apply in other cases. The following items identify when an entry maps a
  page and when it references another paging structure:

  * If more than 12 bits remain in the linear address, bit 7 (PS - page size)
    of the current paging-structure entry is consulted. If the bit is 0, the 
    entry references another paging structure. If the bit is 1, the entry 
    maps a page.

  * If only 12 bit remain in the linear address, the current paging-structure
    entry always maps a page (bit 7 is used for other purposes).

  If a paging-structure entry maps a page when more than 12 bits remain in the
  linear address, the entry identifies a page frame larger than 4 KBytes. For
  example, 32-bit paging uses the upper 10 bits of a linear address to locate
  the first paging-structure entry. 22 bits remain. If that entry maps a page,
  the page frame is 2^22 Bytes = 4 MBytes. 32-bit paging supports 4-MByte
  pages if CR4.PSE = 1. PAE paging and 4-level paging support 2-MByte pages
  (regardless of the value of CR4.PSE). 4-level paging may support 1-GByte
  pages.

  Paging structures are given different names based on their uses in the 
  translation process. Table 4-2 gives the names of the different paging
  structures. It also provides, for each structure, the source of the
  physical address used to locate it (CR3 or different paging-structure
  entry). The bits in the linear address used to select an entry from the 
  structure, and details of whether and how such an entry can map a page.

  ![Alt text](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel_hacking/testcase/mmu/PAGING_STURE_DIFF_MODE.png)

### 32-BIT PAGING

  A logical proceesor uses 32-bit paging if CR0.PG = 1 and CR4.PAE = 0. 32-bit
  paging translates 32-bit linear addresses to 40-bit physical addresses.
  Although 40 bits corresponds to 1 TByte, linear addresses are limited to 
  32 bits. At most 4 GBytes of linear-address space may be accessed at any
  given time.

  32-bit paging uses a hierarchy of paging structures to produce a translation
  for a linear address. CR3 is used to locate the first paging-struct, the
  page direntory. Table 4-3 illustrates how CR3 is used with 32-bit paging.

  32-bit paging may map linear addresses to either 4-KByte pages or 4-MByte
  pages. Figure 4-2 illustrates the translation process when it uses a 4-Kbyte
  page. Figure 4-3 covers the case of a 4-MByte page. The following items
  describe the 32-bit paging process in more detail as well has how the page
  size is determined.

  * A 4-KByte naturally aligned page directory is located at the physical 
    address specified in bits 31:12 of CR3 (See Table 4-3). A page directory
    comprises 1024 32-bit entries (PDEs). A PDE is selected using the physical
    address defined as follows:

    -- Bits 39:32 are all 0.

    -- Bits 31:12 are from CR3.
