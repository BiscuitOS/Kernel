CACHING TRANSLATION INFORMATION
--------------------------------------------------

  The IA-32 architecture may accelerate the address-translation process by
  caching data from the paging structure on the processor. Because the 
  processor does not ensure that the data that it caches are always 
  consistent with the structures in memory, it is important for software
  developments to understand how and when the processor may cache such data.
  They should also understand what actions software can take to remove cached
  data that may be inconsistent and when it should do so. This section
  provides software developers information about the relevant processor
  operation.

### Process-Context Identifiers (PCIDs)

  Processor-context identifiers (PCIDs) are a facility by which a logical
  processor may cache information for multiple linear-address space. The 
  processor may retain cached information when software switches to a 
  different linear-address space with a different PCID (e.g. by loading CR3).

  A PCID is a 12-bit identifier. Non-zero PCIDs are enabled by setting the
  PCIDE flag (bit 17) of CR4. If CR4.PCIDE = 0, the current PCID is always
  000H. Otherwise, the current PCID is the value of bits 11:0 of CR3. Not
  all processors allow CR4.PCIDE to be set to 1.

  The processor ensures that CR4.PCIDE can be 1 only in IA-32e mode (thus,
  32-bit paging and PAE paging use only PCID 000H). In addition, software
  can change CR4.PCIDE from 0 to 1 only if CR3[11:0] = 000H. These require-
  ments are enforced by the following limitations on the MOV CR instruction:

  More information see Intel `4.10.1 Process-Context Identifiers (PCIDs)` 

### Translation lookaside Buffers (TLBs)

  A processor may cache information about the translation of linear addresses
  in translation lookside buffer (TLBs). In general, TLBs contains entries
  that map page numbers to page frames. These terms are defined in next
  Section that describes how information may be cached in TLBs, and gives
  details of TLB usage. Next section explains the global-page feature,
  which allows software to indicate that certain translations should receive
  special treatment when cached in the TLBs.

#### Page Numbers, Page Frames and Page Offsets

  Next section gives details of how the different paging modes modes
  translation linear addresses to physical addresses. Specifically, the upper
  bits of a linear address (called the page number) determine the uppper bits
  of the physical address (called the page frame). The lower bits of the 
  linear address (called the page offset) determine the lower bits of the
  physical address. The boundary between the page number and the page offset
  is determined by the page size, Specifically:

  * 32-bit paging:

    ```
      -- If the translation does not use a PTE (Because CR4.PSE = 1 and the 
         PS flag is 1 in the PDE used), the page size is 4MByte and the page
         number comprises bits 31:22 of the linear address.

      -- If the translation does use a PTE, the page size is 4KBytes and the
         page number comprises bits 31:12 of the linear address.

      linear address:

      31-----------------------12--------------------
      |          Page Number     |                  |
      -----------------------------------------------
    ```

#### Caching Translations in TLB

  The processor may accelerate the paging process by caching individual
  translations in translation lookaside buffers (TLBs). Each entry in a TLB
  is an individual translation. Each translation is referenced by a page
  number. It contains the following infromation from the paging-structure
  entries used to translate linear addresses with the page number:

  * The physical address corresponding to the page number (the page frame).

  * The access rights from the paging-structure entries used to translate
    linear addresses with the page number.

    ```
      -- The logical-AND of the R/W flags.

      -- The logical-AND of the U/S flags.

    ```
  * Attributes from a paging-structure entry that identifies the final page
    frame for the page number (either a PTE or a paging-structure entry
    in which the PS flag is 1):

    ```
      -- The dirty flag.

      -- The memory type.
    ```

    (TLB entries may contain other information as well. A processor may
    implement multiple TLBs, and some of these may be for special purpose,
    e.g. only for instruction fetches. Such specifial-purpose TLBs may not
    contain some of this information if it is not necessary. For example,
    a TLB used only for instruction fetches need not contain information
    about the R/W and dirty flags).

    As another section, any TLB entries created by a logical processor are
    associated with the current PCID.

    Processors need not implement any TLBs. Processors that do implement TLBs
    may invalidate any TLB entry at any time. Software should not rely on the 
    existence of TLBs or on the retention of TLB entries.

### Details of TLB Use

  Because the TLBs cache entries only for linear addresses with translations,
  there can be a TLB entry for a page number only if the P flag is 1 and the
  reserved bits are 0 in each of the paging-structure entries used to
  translate that page number. In addition, the processor does not cache a
  translation for a page number unless the accessed flag is 1 in each of the
  paging-structure entries used during translation. Before caching a 
  translation, the processor sets any of these accessed flags that is not 
  already 1.

  The processor may cache translations required for prefetches and for
  accesses that are a result of speculative execution that would never
  actually occur in the executed code path.

  If the page number of a linear address corresponds to a TLB entry associated
  with the current PCID, the processor may use that TLB entry to determine
  the page frame, access rights, and other attribute for accesses to that
  linear address. In this case, the processor may not actually consult the
  paging structures in memory. The processor may retain a TLB entry unmodified
  even if software subsequently modifies the relevant paging-structure
  entries in memory. 

  In the paging structures specify a translation using a page larger then
  4 KBytes, some processors may cache multiple smaller-page TLB entries for
  that translation. Each such TLB entry would be associated with a page
  number corresponding to the smaller page sizes (e.g. bits 47:12 of a linear
  address with 4-level paging), even though part of that number (e.g. bits 
  20:12) is part of the offset with respect to the page specified by the 
  paging structure. The upper bits of the physical address in such a TLB entry
  are derived from the physical address in the PDE used to create the 
  translation, while the lower bits come from the linear address of the access
  for which the translation is created. There is no way for software to be
  aware that multiole translations for smaller pages have been used for a
  large page. For example, an execution of INVLPG for a linear address on sunch
  a page invalidates any and all smaller-page TLB entries for the translation
  of any linear address on that page.

  If software modifies that paging structure so that the page sizes used for
  a 4-KByte range of linear addresses changes, the TLBs may subsequently
  contain multiple translations for the address range (one for each page size).
  A reference to a linear address in the address range may use any of these
  translations. Which translation is used may vary from one execution to
  another, and the choice may be implementation-specific.

### Global Pages

  The IA-32 architecture also allow for global pages when the PGE flag (bit7)
  is 1 in CR4. If the G flag (bit 8) is 1 in a paging-structure entry that maps
  a page (either a PTE or a paging-structure entry in which the PS)
