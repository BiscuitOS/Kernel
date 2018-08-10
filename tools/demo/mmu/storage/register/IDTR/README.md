Interrupt Descriptor Table Register (IDTR)
---------------------------------------------------------------------

```
 IDTR

 47                             16 15                      0
 +--------------------------------+------------------------+
 |                                |                        |
 |   32-Bit Linear Base Address   |   16-Bit Table Limit   |
 |                                |                        |
 +--------------------------------+------------------------+

```

The `IDTR` register holds the base address (32 bits in protected mode) and
16-bit table limit for the `IDT`. The base address specifies the linear
address of byte 0 of the `IDT`. The table limit specifies the number of bytes
in the table. The `LIDT` and `SIDT` instructions load and store the `IDTR`
register, respectively. On power up or reset of the processor, the base
address is set to the default value of 0 and the limit is set to 0xFFFFH. 
The base address and limit in the register can then be changed as part of
the processor initialization process.

## link

  [Intel Architectures Software Developer's Manual: Combined Volumes: 3 -- Chapter 2 System architecture overview: 2.4 Memory-Management register](https://software.intel.com/en-us/articles/intel-sdm)
