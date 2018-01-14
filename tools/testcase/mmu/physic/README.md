Physical Address Space on X86 Architecture
--------------------------------------------------

  In protected mode, the IA-32 architecture provides a normal physical
  address space of 4 GByte (2^32 bytes). This is the address space that
  the processor can address on its address bus. This address space is 
  flat (unsegmented), with addresses ranging continuously from 0 to 
  0xFFFFFFFFh. This physical address space can be mapped to read-write
  memory, read-only memory, and memory mapped I/O. The memory mapping
  facilities described in this chapter can be used to divide this physical
  memory up into segment and/or page.

  Starting with the Pentium Pro processor, the IA-32 architecture also
  supports an extension of the physical address space to 2^36 bytes (64
  GBytes). With a maximum physical address of 0xFFFFFFFFFH. This is 
  extension is invoked in either of two ways:

  * Using the physical address extension (PAE) flag, located in bit 5 of 
    control register CR4.

  * Using the 36-bit page size extension (PSE-36) feture (introduced in
    the Pentium III processors).

  Physical address support has since been extended beyond 36 bits.
