Local Descriptor Table Register (LDTR)
---------------------------------------------------------------------

```
LDTR

 System Segment    Segment Descriptor Registe4r (Automatically Loaded)
   Register

 0            15                                                 
 +-------------+ +----------------------------+---------------+-----------+
 |             | |                            |               |           |
 |   Seg.Sel   | | 32-bit Linear Base Address | Segment Limit | Attribute |
 |             | |                            |               |           |
 +-------------+ +----------------------------+---------------+-----------+

```
The `LDTR` register holds the 16-bit segment selector, base address (32 bit
in protected mode), segment limit, and descriptor attributes for the `LDT`.
The base address specifies the linear address of byte 0 of the `LDT` segment.
The segment limit specifies the number of bytes in segment.

The `LLDT` and `SLDT` instructions load and store the segment selector part
of the `LDTR` register, respectively. The segment that contains the `LDT` 
must have a segment descriptor in the `GDT`. When the `LLDT` instruction
loads a segment selector in the `LDTR`: the base address, limit, and
descriptor attributes from the `LDT` descriptor are automtically loaded
in the `LDTR`.

When a task switch occurs, the `LDTR` is automatically loaded with the 
segment selector and descriptor for the `LDT`for the new task. The contents
of the `LDTR` are not automatically saved prior to writing the new `LDT`
information into the register.

On power up or reset of the process, the segment selector and base address
are set to the default value of 0 and the limit is set to 0xFFFFH.

## link

  [Intel Architectures Software Developer's Manual: Combined Volumes: 3 -- Chapter 2 System architecture overview: 2.4 Memory-Management register](https://software.intel.com/en-us/articles/intel-sdm)
