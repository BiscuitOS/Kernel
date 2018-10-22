Logcial-Address
------------------------------------------

Even with the minimum use of segments, every byte in the processor's address
space is accessed with a logical address. A **logical address** consists of
16-bit segment selector and a 32-bit offset. The segment selector identifies
the segment the byte is locate in and the offset specifies the location of
byte in the segment relative to the base address of the segment. As Figure.

```
Logical-Address:

 15                            31                                  0
 +--------------+----+-----+   +-----------------------------------+
 |    Index     | TI | RPL | : |             Offset                |
 +--------------+----+-----+   +-----------------------------------+
```

## Segment Selector

A segment selector is a 16-bit identifier for a segment. It doesn't point
directly to the segment, but instead points to the segment descriptor that
defines the segment. A segment selector contains the following items:

```
       15                                        3    2     0
       +-----------------------------------------+----+-----+
       |               Index                     | TI | RPL |
       +-----------------------------------------+----+-----+
                                                    A    A
                                                    |    |
       Table Indicator -----------------------------o    |
         0 = GDT                                         |
         1 = LDT                                         |
       Request Privilege Level (RPL) --------------------o
```


* **Index**   (Bit3 through 15) -- Selects one of 8192 descriptors in the GDT
              or LDT. The processor multiplies the index value by 8 (the 
              number of bytes in a segment descriptor) and adds the result
              to the base address of the GDT or LDT (from the GDTR or LDTR
              register, respectively).

* **TI (table indicator) flag**
              (Bit 2) -- Specifies the descriptor table to use: Clearing this
              flag selects the GDT; Setting this flag selects the current
              LDT.

* **Requested Privilege Level (RPL)**
              (Bits 0 and 1) -- Specifies the privilege level of the selector.
              The privilege level can range from 0 to 3, with 0 being the most
              privileged level. 

The first entry of the GDT is not used by the processor. A segment selector
that points to this entry of the GDT (that is, a segment selector with an index
of 0 and the TI flag set to 0) is used as **null segment selector**. The 
processor does not generate an exception when a segment register (other than
the CS or SS register) is loaded with a null selector. It does, however, 
generate an exception when a segment register holding a null selector is used
to access memory. A null selector can be used to initialize unused segment
registers. Loading the CS or SS register with a null segment selector causes
a general-protection exception (#GP) to be generated.

Segment selectors are visible to application programs as part of a pointer 
variable, but the values of selectors are usually assigned or modified by link
editors or linking loaders, not appliaction programs.

## Translation

#### Virtual address translate to logical address

The application or kernel program access each byte in program's address space
with a **virtual address**. A **virtual address** contains a bit-width with
machine offset from 0 to maximum address. Linux 32-bit allows splitting the 
user and kernel address ranges in different ways: 3G/1G user/kernel.

To translate a virtual address into a logical address, the processor does 
the following:

1. Find a segment selector. If virtual address points to kernel data segment, 
   processor will select DS as segment selector; If virtual address points to 
   code segment, processor will select CS as segment selector; If virtual 
   address points to stack, processor will select SS as segment selector; If 
   virtual address points to user application data, the processor will select
   FS as segment selector.

2. Use the virtual address as offset for logcial address.

Translation as Figure:

```

                              Virtual address

                              31                           0
                              +----------------------------+
                              |                            |
                              +----------------------------+
                              |
                              |
                              |
                              |
  Logical address             |
                              V
  15                      0   31                           0
  +-----------------------+   +----------------------------+
  |    Segment Selector   | : |           Offset           |
  +-----------------------+   +----------------------------+

```

#### Logical address translate to virtual address

A logical address consists of a 16-bit segment selector and a 32-bit offset.
For every application and kernel program, each byte is accessed on virtual 
space. It is easy to obtain virtual address from logical address, processor
only cover offset into virtual address, as Figure.

```

  Logical address

  15                      0   31                           0
  +-----------------------+   +----------------------------+
  |    Segment Selector   | : |           Offset           |
  +-----------------------+   +----------------------------+
                              | 
                              |
                              |
                              |
             Virtual address  V
                              31                           0
                              +----------------------------+
                              |                            |
                              +----------------------------+
```

#### Logical address translate to linear address

The processor translates every logical address into a linear address. A
**linear address** is a 32-bit address in the processor's linear address 
space. Like the physical address space, the linear address space is a flat(
unsegmented), 2^32-byte address space, with address ranging from 0 to 
0xFFFFFFFFH. The linear address space contains all the segments and system
tables defined for a system.

To translate a logical address into a linear address, the processor does the
following:

1. Uses the offset in the segment selector to locate the segment descriptor
   for the segment in the GDT or LDT and reads it into the processor. (This
   step is needed only when a new segment selector is loaded into a segment
   register.)

2. Examines the segment descriptor to check the access right and range of the
   segment to insure that the segment is accessible and theat the offset is
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

## Appendix
