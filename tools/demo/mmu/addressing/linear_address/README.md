Linear-Address
------------------------------------------

The processor translates every logical address into a linear address. A 
**linear address** is a 32-bit address in the processor's linear address 
space. Like the physical address space, the linear address space is a flat 
(unsegmented) 2^32-byte address space, with addresses ranging from 0 to 
0xFFFFFFFFH. The linear address space contains all the segments and system 
table defined for a system. As Figure:

```
Linear address space

  +--------------------+ 4G
  |                    |
  |                    |
  |                    |
  |                    |
  |                    |
  |                    |
  |                    |
  |                    |
  |                    |
  |                    |
  |                    |
  |     Flat Model     |
  |                    |
  |                    |
  |                    |
  |                    |
  |                    |
  |                    |
  |                    |
  |                    |
  |                    |
  |                    |
  |                    |
  |                    |
  |                    |
  +--------------------+ 0

linear adress

  31                                           0
  +--------------------------------------------+
  |                                            |
  +--------------------------------------------+

```

If paging is not used, the linear address space of the processor is mapped
directly into the physical address space of processor. The physical address
space is defined as the range of addresses that the processor can generate
on its address bus.

Because multitasking computing systems commonly define a linear address space
much larger than it is economically feasible to contain all at in physical
memory, some method of "virtualizing" the linear address space is needed.
This virtualization of the linear address space is handled through the 
processor's paging mechanism.

# Translation for linear-address

### Translate logical address to virtual address

At the system-architecture level in protected mode, the processor uses two 
stage of address translation to arrive at a physical address: logical-address
translation and linear address space paging.

To translate a logical address into a linear address, the processor does the
following:

1. Uses the offset in the segment selector to locate the segment descriptor
   for the segment in the GDT or LDT and reads it into the processor. (This
   step is needed only when a new segment selector is loaded into a segment
   register.)

2. Examines the segment descriptor to check the access right and range of the
   segment to insure that the segment is accessible and that the offset is
   within the limits of the segment.

3. Adds the base address of the segment from the segment descriptor to the
   offset to form a linear address.

```
  Logical address

  15                      0   31                           0
  +-----------------------+   +----------------------------+
  |    Segment Selector   | : |           Offset           |
  +-----------------------+   +----------------------------+
    |                                        |
    |                                        |
    |                                        |
    |     Descriptor Table                   |
    |    +----------------+                  |
    |    |                |                  |
    |    |                |                  |
    |    |                |                  |
    |    |                |                  |
    |    |                |                  |
    |    |                |                  |
    |    |                |                  |
    |    +----------------+                  V
    |    |    Segment     |                +---+
    o--->|   Descriptor  -|--------------->| + |
         +----------------+                +---+
         |                |                  |
         +----------------+                  |
                                             |
                                             |
                                             |
                              32             V             0
                              +----------------------------+
                              |       Linear Address       |
                              +----------------------------+
```

### Translate linear address to physical address



## Appendix
