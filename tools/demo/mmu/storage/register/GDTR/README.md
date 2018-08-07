Global Descriptor Table Register (GDTR)
---------------------------------------------------------------------

```
 GDTR

 47                             16 15                      0
 +--------------------------------+------------------------+
 |                                |                        |
 |   32-Bit Linear Base Address   |   16-Bit Table Limit   |
 |                                |                        |
 +--------------------------------+------------------------+

```

The `GDTR` register holds the base address and the 16-bit table limit for
the `GDT`. The base address specifies the linear address of byte 0 of the
GDT. The table limit specifies the number of bytes in the table.

The `LGDT` and `SGDT` instructions load and store the GDTR regsiter,
respectively. On power up or reset of the processor, the base address is
set to the default value of 0 and the limit is set to 0xFFFFH. A new 
base address must be loaded into the `GDTR` as part of the processor
initlization process for protected-mode operation.

## link

  [Intel Architectures Software Developer's Manual: Combined Volumes: 3 -- Chapter 2 System architecture overview: 2.4 Memory-Management register](https://software.intel.com/en-us/articles/intel-sdm)
