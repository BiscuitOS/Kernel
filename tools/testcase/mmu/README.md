Protected-Mode Memory Management on X86 Architecture
--------------------------------------------------------

### Memory Manmagement Overview

  The memory management facilities of the IA-32 architecture are divided
  into two parts: segmentation and paging. 

  * Segmentation provides a mechanism of isolating indvidual code, data, 
    and stack modules so that multiple programs (or tasks) can run on the
    same processor without interfering with one another. 

  * Paging provides a mechanism for implementing a conventional demand-page,
    virutal-memory system where sections of a program's execution 
    environment are mapped into physical memory as needed. Paging can also
    be used to provide isolation between multiple tasks. 

  When operating in protected mode, some form of segmentation must be used.
  There is no mode bit to disable segmentation. The use of paging, however,
  is optional.

  These two mechanisms (segmentation and paging) can be configured to 
  support simple single-program (or sigle-task) systems, multitasking system,
  or multiple-processor systems that used shared memory.

  As shown in Figure 3-1, segmentation provides a mechanism for dividing
  the processor's addressable memory space (called the linear address space)
  into smaller protected address space called segments. Segments can be 
  used to hold the code, data, and stack for a program or to hold system 
  data structures (such as a TSS or LDT). If more than one program (or task)
  is running on a processor, each program can be assigned its own set of
  segments. The processor then enforces the boundaries between these segment
  and insures that one program does not interfere with the execution of
  another program by writing into the other program's segments. The 
  segmentation mechanism also allows typing of segments so that the operations
  that may be performed on a particular type of segment can be restricted.

  All the segments in a system are contained in the processor's linear address
  space. To locate a byte in a particular segment, a logical address (also
  called a far pointer) must be provided. A logical address consists of a
  segment selector and an offset. The segment selector is unique identifier
  for a segment. Among other things it provides an offset into a descriptor
  table (such as the global descriptor table, GDT) to a data structure called
  a segment descriptor. Each segment has a segment descriptor, which specifies
  the size of the segment, the access rights and privilege level for the 
  segment, the segment type, and the location of the first byte of the segment
  in the linear address space (called the base address of the segment). The
  offset part of the logical address is added to the base address for the
  segment to locate a byte within the segment. The base address plus the
  offset thus form a linear address in the processor's linear address space.

  ![Alt text](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel_hacking/testcase/mmu/SEG_PAGING.png)

  If paging is not used, the linear address space of the processor is mapped
  directly into the physical address space of processor. The physical address
  space is defined as the range of addresses that the processor can generate
  on its address bus.

  Because multitasking computing systems commonly define a linear address
  space much larger than it is economically feasible to contain all at once
  in physical memory, some method of `virtualizing` the linear address space
  is needed. This virtualization of the linear address space is handled 
  through the processor's paging mechanism.

  Paging supports a `virtual memory` environment where a large linear address
  space is simulated with a small amount of physical memory (RAM and ROM) and
  some disk storage. When using paging, each segment is divided into pages (
  typically 4 KBytes each in size), which are stored either in physical memory
  or on the disk. The operating system or executive maintains a page directory
  and a set of page tables to keep track of the pages. When a program (or task)
  attempts to access an address location in the linear address space, the 
  processor uses the page directory and page tables to translate the linear
  address into a physical address and then performs the requested operation (
  read or write) on the memory location.

  If the page being accessed is not currently in physical memory, the 
  processor interrupts execution of the program(by generating a page-fault 
  exception). The operating system or executive then reads the page into
  physical memory from the disk and continues executing the program.

  When paging is implemented properly in the operating-system or executive,
  the swapping of pages between physical memory and the disk is transparent
  to the correct execution of a program. Even programs written for 16-bit
  IA-32 processors can be paged (transparently) when they are run in
  virtual-8086 mode.

### File list

  * Segmentation

    The segmentation mechanism on IA-32 architecture. More Segmentation refer
    `segmentation/README.md` 
  
  * logic

    The logical address mechanism on IA-32 architecture. More information refer
    `logic/READMD.md`

  * linear

    The linear address mechanism on IA-32 architecture. More information refer
    `linear/READMD.md`
