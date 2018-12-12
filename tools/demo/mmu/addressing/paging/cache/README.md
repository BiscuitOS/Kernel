Paging-Structure Caches
------------------------------------------------------

In addition to the TLBs, a processor may cache other information about the
paging structure in memory.

# Cache for Paging structures

A processor may support following paging-structure caches:

* **PDE cache** The use of the PDE cache depends on the paging mode:

```
  -- For 32-bit paging, each PDE-cache entry is referenced by a 10-bit value
     and is used for linear address for which bits 31:22 have that value.
 
  A PDE-cache entry contains information from the PDE use to translation the 
  relevant linear addresses.

  -- The physical address from the PDE (the address of the page table). (No
     PDE-cache entry is created for a PDE that maps a page.)

  -- The logical-AND of the R/W flags in the PDE.

  -- The logical-AND of the U/S flags in the PDE.

  -- The logical-OR of the XD flag in the PDE.

  -- The value of the PCD and PWT flags of the PDE.

  The following items detail how a processor may use the PDE cache:

  -- If the processor has a PDE-cache entry for a linear addres, it may use
     that entry when translation the linear address.

  -- The processor does not create a PDE-cache entry unless the P flag is 1,
     the PS flag is 0, and the reserved bits are 0 in the PDE in memory.

  -- The processor does not create a PDE-cache entry unless the accessed flag
     is 1 in the PDE in memory; before caching a translation, the processor
     sets any accessed flags that are not already 1.

  -- The processor may create a PDE-cache entry even if there are no 
     translations for any linear address that might use that entry.

  -- If the processor creates a PDE-cache entry, the processor may retain it 
     unmodified even if software subsequently modifies the corresponding PDE
     in memory.

```
