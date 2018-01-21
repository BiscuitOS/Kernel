Linear Address mechanism on X86 architecture
--------------------------------------------------

  At the system-architecture level in protected mode, the processor uses two
  stages of address translation to arrive at a physical address: 
  logical-address translation and linear address space paging.

  Even with the minimum use of segments, every byte in the processor's
  address space is accessed with a logical address. A logical address 
  consists of a 16-bit segment selector and a 32-bit offset (See Figure 3-5).
  The segment selector identifies the segment the byte is located in and
  the offset specifies the location of the byte in the segment relative
  to the base address of the segment.

  The processor translates every logical address into a linear address.
  A linear address is a 32-bit address address in the processor's linear
  address space. Like the physical address space, the linear address space
  is a flat (unsegmented), 2^32-byte address space, with addresses ranging
  from 0 to 0xFFFFFFFFH. The linear address space contains all the segments
  and system tables defined for a system.

### Translate logical address to linear address

  To translate a logical address into a linear address, the processor does
  the following:

  1. Uses the offset in the segment selector to locate the segment descriptor
     for the segment int the GDT or LDT and reads it into the processor. (
     This step is needed only when a new segment selector os loaded into a
     segment register.)

  2. Examines the segment descriptor to check the access rights and range
     of the segment to insure that the segment is accessible and that the
     offset is within the limits of the segment.

  3. Adds the base address of the segment from the segment descriptor to 
     the offset to from a linear address.

  ![Alt text](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel_hacking/testcase/mmu/LA_TO_LINE.png)  

  If paging is not used, the processor maps the linear address directly to
  a physical address (that is, the linear address goes out on the processor's
  address bus). If the linear address space is paged, a second level of 
  address translation is used to translate the linear address into a physical
  address.
