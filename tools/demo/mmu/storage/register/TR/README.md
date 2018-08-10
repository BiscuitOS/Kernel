Local Descriptor Table Register (LDTR)
---------------------------------------------------------------------

```
Task Register

 System Segment    Segment Descriptor Registe4r (Automatically Loaded)
   Register

 0            15                                                 
 +-------------+ +----------------------------+---------------+-----------+
 |             | |                            |               |           |
 |   Seg.Sel   | | 32-bit Linear Base Address | Segment Limit | Attribute |
 |             | |                            |               |           |
 +-------------+ +----------------------------+---------------+-----------+

```
The task register holds the 16-bit segment selector, base address (32 bits
in protected mode), segment limit, and descriptor attributes for the `TSS`
of the current task. The selector references the `TSS` descriptor in the
`GDT`. The base address specifies the linear address of byte 0 of the `TSS`.
The segment limit specifies the number of bytes in the `TSS`.

The `LTR` and `STR` instructions load and store the segment selector part
of the task register, respectively. When the `LTR` instruction loads a
segment selector in the task register, the base address, limit, and 
descriptor attributes from the TSS descriptor are automatically loaded
into the task register. On power up or reset of the processor, the base
address is set to the default of the 0 and the limit is set to 0xFFFFH.

When a task switch occurs, the task register is automatically loaded with
the segment selector and descriptor for the `TSS` for the new task. The
contents of the task regsiter are not automatically saved prior to writing
the new `TSS` information into the register.

## link

  [Intel Architectures Software Developer's Manual: Combined Volumes: 3 -- Chapter 2 System architecture overview: 2.4 Memory-Management register](https://software.intel.com/en-us/articles/intel-sdm)
