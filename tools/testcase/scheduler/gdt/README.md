Global Descriptor Table (GDT)
--------------------------------------

  When operating in protected mode, all memory accesses pass through
  either gloabl descriptor table (GDT) or and optional local 
  descriptor table (LDT) as shown in Figure. There tables contain
  entries called segment descriptors. Segment descriptors provide
  the base address of segments well as access right, type, and usage
  information.

  ![Alt text](https://github.com/EmulateSpace/PictureSet/blob/master/gdt/IA32_system-level_Registers.png)

  Each segment descriptor has an associated segment selector. A segment
  selector provides the software that uses it with an index into the 
  GDT or LDT (the offset of its associated segment descriptor), a
  global/local flag (determines whether the selector points to the
  GDT or the LDT), and access rights information. and access rights 
  information.

  To access a byte in a segment, a segment selector and an offset must
  be supplied. The segment selector provides access to the segment
  descriptor for the segment (in the GDT or LDT). From the segment
  descriptor, the processor obtains the base address of the segment
  in the linear address space. The offset then provide the location
  of the byte relative to base address. This mechanism can be used
  to access any valid code, data, or stack segment, provided the segment
  is accessible from the current privilege (CPL) at which the processor
  is operating. The CPL is defined as the protection level of the 
  currently executing code segment.

  Above Figure. The solid arrows in the figure indicate a linar 
  address, dashed lines indicated a segment selector, and the dotted
  arrows indicate a physical address. For simplicity, many of the 
  segment selector are shown as direct pointers to a segment. However,
  the actual path from a segment selector to its associated segment
  is always through a GDT or LDT.

  The linear address of the base of the GDT is contained in the GDT
  register (GDTR). the linear address of the LDT is contained in the 
  LDT register(LDTR).

