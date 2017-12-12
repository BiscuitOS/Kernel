Interrupt Descriptor Table (IDT)
---------------------------------------------------------

  The interrupt descriptor table (IDT) associates each exception or interrupt
  vector with a gate descriptor for the procedure or task used to service
  the associated exception or interrupt. Like the GDT and LDTs, the IDT
  is an array of 8-byte descriptors (in protected mode). Unlink the GDT, the
  first entry of the IDT may contain a descriptor. To from an index into
  the IDT, the processor scales the exception or interrupt vector by eigth
  (the number of bytes in a gate descriptor). Because there are only 256
  interrupt or exception or interrupt vectors, the IDT need not contain
  more than 256 descriptors. It can contain fewer than 256 descriptors,
  because descriptors are required only for interrupt and exception vectors
  that may occur. All empty descriptor slots in the IDT should have the
  present flag for the descriptor set to 0.

  The base addresses of the IDT should be aligned on an 8-byte boundary
  to maximize performance of cache line fills. The limit value is expressed
  in bytes and is added to the base address to get the address of the last
  valid byte. A limit value of 0 result in exactly 1 valid byte. Because
  IDT entries are always eight bytes long, the limit should always by one
  less than an integral multiple of eight (that is, 8N -1)

  The IDT may reside anywhere in the linear address space. As Figure, the
  processor locates the IDT using the IDTR register. This register holds
  both a 32-bit base address and 16-bit limit for the IDT.

  The LIDT (load IDT register) and SIDT (store IDT register) instructions 
  load and store the contents of the IDTR register, respectively. The 
  LIDT instruction loads the IDTR register with the base address and limit
  held in a memory operand. This instruction can be executed only when the
  CPL is 0. It normally is used by the initialization code of an operating
  system when creating an IDT. An operating system also may use it to change
  from one IDT to another. The SIDT instruction copies the base and limit
  value stored in IDTR to memory. This instruction can be executed at any
  privilege level.

  If a vector references a descriptor beyond the limit of the IDT, a general-
  protection exception (#GP) is generated.

  ***NOTE***

    Because interrupts are delivered to the processor core only once, an 
    incorrectly configured IDT could result in incomplete interrupt handling
    and/or the blocking of the blocking of interrupt delivery.

