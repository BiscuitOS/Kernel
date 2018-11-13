Paging Mechanism
----------------------------------------------

Paging can be used with any of the segmentation models described in Figure. The
processor's paging mechanism divides the linear address space (into which 
segment are mapped) into pages (as shown in Figure). These linear-address-space
pages are then mapped to pages in the physical address space. The paging 
mechanism offers several page-level protection facilities that can be used or
instead of the segment-protection facilities. For example, it lets read-write
protection be enforced on a page-by-page basis. The paging mechanism also 
provides two-level user-supervisor protection that can also be specified on
a page-by-page basis.

![Segmentation and Paging](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel/MMU000417.png)

## Physical Address Space

In protected mode, the IA-32 architecture provides a normal physical address 
space of 4 GBytes (2^32 bytes). This is the address space that the processor
can address on its address bus. This address space is flat (unsegmented), with
addresses ranging continuously from 0 to 0xFFFFFFFF. This physical address 
space can be mapped to read-write memory, read-only memory, and memory mapped
I/O. The memory mapping facilities described in this chapter can be used to
divide this physical memory up into segments and/or pages.

# Paging Modes And Control Bits

Paging behavior is controlled by the following control bits:

* The WP an PG flags in control register CR0 (bit 16 and bit 31, respectively).

* The PSE, PAE, PGE, PCIDE, SMEP, and PKE flags in control register CR4 (bit 4,
  bit 5, bit 7, bit 17, bit 20, bit 21, and bit 22, respectively).

* The AC flag in the EFLAGS register (bit 18).

Software enables paging by using the MOV to CR0 instruction to set CR0.PG.
Before doing so, software should ensure that control register CR3 contains the
physical address of the first paging struction that the processor will use for
linear-address translation and 

### CR0

More information about CR0 Register see **tools/demo/mmu/storage/register/CR0**

### CR4

More information about CR4 Register see **tools/demo/mmu/storage/register/CR4**

### EFLAGS

More information about EFLAGS Register see **tools/demo/mmu/storage/register/EFLAGS**

### Three Paing Modes

If CR0.PG = 0, paging is not used. The logical processor treats all linear 
address as if they were physical address. CR4.PAE is ignored by the processor,
as are CR0.WP, CR4.PSE, CR4.PGE, CR4.SMEP, and CR4.SMA.

Paging is enabled if CR0.PG = 1. Paging can be enabled only if protection is
enabled (CR0.PE = 1). If paging is enabled, one of three paging modes is used.
The values of CR4.PAE determine which paging mode is used.

* If CR0.PG = 1 and CR4.PAE = 0, 32-bit paging is used.

* If CR0.PG = 1, CR4.PAE = 1, and IA32_EFER.LME = 0, PAE paging is used.

* If CR0.PG = 1, CR4.PAE = 1, and IA32_EFER.LME = 1, 4-level paging is used.

The three paging modes differ with regard to the following details:

* Linear-address width. The size of the linear addresses that can be
  translated.

* Physcial-address width. The size fo the linear addresses that can be
  translated.

* Page size. The granularity at which linear addresses are translated. Linear
  addresses on the same page are translated to corresponding physcial addresses
  on the same page.

* Support for execute-disable access rights. In some paging modes, software can
  be prevented from fetching instructions from pages that are otherwise
  readable.

* Support for PCIDs. With 4-level paging, software can enable modes, software
  can be prevented from fetching instructions from pages that are otherwise 
  readable.

* Support for protection keys. With 4-level paging, software can enable a 
  facility by which each linear address is associated with a protection key.
  software can use a new control register to determine, for each protection 
  keys, how software can access linear addresses associated with that 
  protection key.

### Paging-Mode Enabling 

If CR0.PG = 1, a logical processor is in one of thress paging mode, depending
on the values of CR4.PAE and IA32_EFER.LME. Figure illustrates how software
can enable these modes and make translations between them. The following items
identify certain limitations and other details:

![Enabling and Changing Paging Modes](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel/MMU000419.png)

* IA32_EFER.LME cannot be modify while paging is enabled (CR0.PG = 1). Attempts
  to do so using WRMSP cause a general-protection exception (#GP(0)).

* Paing cannot be enabled (by setting CR0.PG to 1) while CR4.PAE = 0 and 
  IA32_EFER.LME = 1. Attempts to do so using MOV to CR0 cause a
  general-protection exception (#GP(0)).

* CR4.PAE cannot cleared while 4-level paging is active (CR0.PG = 1 and 
  IA32_EFER.LME = 1). Attempts to do so using MOV to CR4 cause a
  general-protection exception (#GP(0)).

* Regardless of the current paging mode, software can disable paging by
  clearing CR0.PG with MOV to CR0.

* Software can make transitions between 32-bit paging and PAE paging by 
  changing the value of CR4.PAE with MOV to CR4.

* Software cannot make transitions directly between 4-level paging and either
  of the other two paing modes. It must first disable paging (by clearing 
  CR4.PG with MOV to CR0), then set CR4.PAE and IA32_EFER.LME to the desired
  values (with MOV to CR4 and WRMSR), and then re-enable paging (by setting
  CR0.PG with MOV to CR0). As noted earlier, an attempt to clear either CR4.PAE
  or IA32_EFER.LME cause a general-protection exception (#GP).

* VXM transitions allow transitions between paging modes that are not possible
  using MOV to CR or WRMSR. This is because VMX transitions can load CR0, CR4,
  and IA32_EFER in one operation.

# Hierarchical Paging Structures: An Overview

All three paging modes tranlate linear address using hierarchical paging 
structures. Every paging structures is 4096 Byte in size and comprises a number
of individual entries. With 32-bit paging, each entry is 32-bits (4 bytes); 
there are thus 1024 entries in each structure. With PAE paging and 4-level
paging, each entry is 64 bits (8 bytes); these are thus 512 entries in each
struction. (PAE paging includes one exception, a paging structure that is 32
bytes in size, containing 4 64-bit entries.)

The processor uses the upper portion of a linear address to indentify a series
of paging-structure entries. The last of these entries identifies the physical
address of the region to which the linear address translates (called the page
frame). The lower portion of the linear address (called the page offset) 
identifies the specific address within that region to which the linear address
translates.

Each paging-structure used for any translation is located at the physical
address in CR3. A linear address is translated using the following iterative 
procedure.
