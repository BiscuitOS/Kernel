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

* The LME and NXE flags in the IA32_EFER MSR (bit 8 and bit 11, respectively).

* The AC flag in the EFLAGS register (bit 18).

Software enables paging by using the MOV to CR0 instruction to set CR0.PG.
Before doing so, software should ensure that control register CR3 contains the
physical address of the first paging struction that the processor will use for
linear-address translation and 

### CR0

Control register CR0 determine operating system mode of the processor and the
characteristics of the currently executing task. The register are 32 bits in
all 32-bit mode.

**CR0** contains system control flags that control operating mode and state of
the processor.

![CR0_Register](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel/MMU000414.png)

* CR0.PG

  **Paging (bit 31 of CR0)** -- Enables paging when set; disables paging when
  clear. When paging is disabled, all linear address are treated as physical
  address. The PG flag has no effect if the PE flag (bit 0 of register CR0) is
  not also set; setting the PG flag when the PE flag is clear causes a general-
  protection exception (#GP).

More information about CR0 Register see **tools/demo/mmu/storage/register/CR0**


