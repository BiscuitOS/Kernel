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

Each paging-structure entry contains a physical address, which is either the
address of another paging structure or the address of a page frame. In the 
first case, the entry is said to reference the other paging structure; in the
latter, the entry is said to map a page. 

The first paging structure used for any translation is located at the physical
address in CR3. A linear address is translated using the following iterative
procedure. A portion of the linear address (initially the uppermost bits) 
selects an entry in a paging structure (initially the one located using CR3).
If that entry references another paging structure, the processor continues with
that paing structure and with portion of the linear address immediately below
that just used. If instead the entry maps a page, the process completes: the 
physical address in the entry is that of the page frame and the remaining lower
portion of the linear address is the page offset.

The following items give an example for each of the three paging modes (each
example locates a 4-KByte page frame):

* With 32-bit paging, each paging structure comprises 1024 = 2^10 entries. For
  this reason, the translation process uses 10 bits at a time from a 32-bit
  linear address. Bits 31:22 identify the first paging-structure entry and
  21:12 identify a second. The latter identifies the page frame. Bits 11:0 of 
  the linear address are the page offset within the 4-Kbyte page frame. As
  Figure.

  ![Linear-Address 4-KBytes](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel/MMU000421.png)

* With PAE paging, the first paging structure comprises only 4 = 2^2 entries.
  Translation thus begins by using bits 31:30 from a 32-bit linear address to
  identify the first paging-structure entry. Other paging structures comprise
  512 = 2^9 entries, so the process continues by using 9 bits at a time. Bits
  29:21 identify a second paging-structure entry and bits 20:12 identify a 
  third. This last identifies the page frame. See Figure.

  ![Linear-Address 4-KBytes PAE](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel/MMU000422.png)

* With 4-level paging, each paging structure comprises 512 = 2^9 entries and
  translation uses 9 bits at a time from a 48-bit linear address. Bits 47:39
  identify the first paging-structure entry, bits 38:30 identify a second, bits
  29:21 a third, and bits 20:12 identify a fourth. Again, the last identifies
  the page frame. See Figure.

  ![Linear-Address 4-level](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel/MMU000423.png)

The translation process in each of the examples above completes by identifying
a page frame; the page frame is part of the translation of the original linear
address. In some cases, however, the paging structures may be configured so
that the translation process terminates before identifying a page frame. This
occurs if the process encounters a paging-structure entry that is marked "not
present" (because its P flag -- bit 0 -- is clear) or in which a reserved bit
is set. In this case, there is no translation for the linear address; an access
to that address causes a page-fault exception..

In the example above, a paging-structure entry maps a page with a 4-KByte page
frame when only 12 bits remain in the linear address; entries identified 
earlier always refernce other paging structures. That may not apply in other
cases. The following items identify when an entry maps a page and when it 
references another paging structure:

* If more than 12 bits remain in the linear address, bit 7 (PS -- page size) of
  the current paging-structure entry is consulted. If the bit is 0, the entry
  references another paging structure; if the bit is 1, the entry maps a page.

* If only 12 bits remain in the linear address, the current paging-structure
  entry always maps a page (bit 7 is used for other purposes).

# 32-Bit Paging

A logical processor uses 32-bit paging if CR0.PG = 1 and CR4.PAE = 0. 32-bit
paging translates 32-bit linear addresses to 40-bit physical addresses.
Although 40 bits corresponds to 1 TByte, linear addresses are limited to 32
bits; at most 4 GBytes of linear-address space may be accessed at any given
time.

32-bit paging uses a hierarchy of paging structures to produce a translation
for a linear address. CR3 is used to locate the first paging-structure, the 
page directory. Table illustrates how CR3 is used with 32-bit paging.

![Use of CR3 with 32-bit Paging](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel/MMU000424.png)

32-bit paging may map linear addresses to either 4-KByte pages or 4-MByte 
pages. Figure 4-2 illustrates the translate process when it uses a 4-Kbyte
page; Figure 4-3 covers the case of a 4-MByte page. The following items
describe the 32-bit paging process in more detail as well as how the page size
is tetermined:

![Figure 4-2](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel/MMU000425.png)

![Tigure 4-3](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel/MMU000426.png)

* A 4-KByte naturally aligned page directory is located at the physical address
  specified in bits 31:12 of CR3. A page directory comprise 1024 32-bit entries
  (PDEs). A PDE is selected using the physical address defined as follow:

  -- Bits 39:32 are all 0.

  -- Bits 31:12 are from CR3.

  -- Bits 11:2 are bits 31:22 of the linear address.

  -- Bits 1:0 are 0.

```
CR3
31                    12
+-----------------------+----+
| Base Physical Address |    |     
+-----------------------+----+
           |
           |
           |             Linear address
           |             31         22  
           |             +------------+---------------+
           |             | PDE offset |               |
           |             +------------+---------------+
           |                   |
           |                   |
 PDE       |                   |
 31        V            12     V     2     0
 +-----------------------+------------+----+
 |                       |            | 00 |
 +-----------------------+------------+----+

```

Because a PDE is identified using bits 31:22 of the linear address, it controls
access to a 4-Mbyte region of the linear-address space. Use of the PDE depends
on CR4.PSE and the PDE's PS flag (bit 7):

* If CR4.PSE = 1 and the PDE's PS flag is 1, the PDE maps a 4-MByte page. The
  final physical address is computed as follows:

  -- Bits 39:32 are bits 20:13 of the PDE.

  -- Bits 31:22 are bits 31:22 of the PDE.

  -- Bits 21:0 are from the original linear address.

* If CR4.PSE = 0 or the PDE's PS flag is 0, a 4-KByte naturally aligned page
  table is located at the physical address specified in bits 31:12 of the PDE.
  A page table comprises 1024 entries (PTEs). A PTE is selected using the 
  physical address defined as follows:

  -- Bits 39:32 are all 0.

  -- Bits 31:12 are from the PDE.

  -- Bits 11:2 are bits 21:12 of the linear address.

  -- Bits 1:0 are 0.

```
 PDE
 31                    12
 +-----------------------+----+
 | Base Physical Address |    |     
 +-----------------------+----+
           |
           |
           |   Linear address
           |   31            21        12        0
           |   +------------+------------+-------+
           |   |            | PTE offset |       |
           |   +------------+------------+-------+
           |                   |
           |                   |
 PTE       |                   |
 31        V            12     V     2     0
 +-----------------------+------------+----+
 |                       |            | 00 |
 +-----------------------+------------+----+
```

* Because a PTE is identified using bits 31:12 of the linear address, every PTE
  maps a 4-KByte page. The final physical address is computed as follow:

  -- Bits 39:32 are all 0.

  -- Bits 31:12 are from the PTE.

  -- Bits 11:0 are from the original linear address.

```
 PTE
 31                    12
 +-----------------------+----+
 | Base Physical Address |    |     
 +-----------------------+----+
           |
           |
           |  Linear address
           |  31         12  
           |  +------------+---------------+
           |  |            | Original addr |
           |  +------------+---------------+
           |                   |
           |                   |
 Physical  |                   |
 31        V            12     V           0
 +-----------------------+-----------------+
 |                       |                 |
 +-----------------------+-----------------+

```

A reference using a linear address that is successfully translated to a 
physical address is performed only if allowed by the access rights of the 
translation.

![Tigure 4-4](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel/MMU000427.png)

![Table 4-3](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel/MMU000428.png)

![Table 4-4](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel/MMU000429.png)

![Table 4-5](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel/MMU000430.png)

![Table 4-6](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel/MMU000431.png)
