CR4: Control Register 4
---------------------------------------------

Control register CR4 determine operating mode of the processor and the 
characteristics of the currently executing task. CR4 contains a group of flags
that enable several architecture extensions, and indicate operating system or
executive support for specific processor capabilities.

![CR4_Register](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel/MMU000416.png)

* CR4.PSE

  **Page Size Extensions (bit4 0f CR4)** -- Enables 4-MByte pages with 32-bit
  paging when set; restricts 32-bit paging to pages of 4 KBytes when clear.

* CR4.PAE

  **Physical Address Extension (bit 5 of CR4)** -- When set, enables paging to
  produce physical addresses with more than 32 bits. When clear, restricts
  physcial addresses to 32-bits. PAE must be set before entering IA-32e mode.

* CR4.PGE

  **Page Global Enable (bit 7 of CR4)** -- Enables the global page feature when
  set; disables the global page feature when clear. The global page feature 
  allows frequently used or shared pages to be marked as global to all users (
  done with the global flag, bit 8, in a page-director or page-table entry).
  Global pages are not flushed from the translation-lookaside buffer (TLB) on a
  task switch or a write to register CR3.

  When enabling the global page feature, paging must be enabled (by setting the
  PG flag in control register CR0) before the PEG flag is set. Reversing the
  sequence may affect program correctness, and processor performance will be
  impacted.

* CR4.PCIDE

  **PCID-Enable Bit (bit 17 of CR4)** -- Enables processor-context identifiers
  (PCIDs) when set.

* CR4.SMEP

  **SMEP-Enable Bit (bit 20 of CR4)** -- Enables supervisor-mode execution 
  prevention (SMEP) when set.

* CR4.SMAP

  **SMAP-Enable Bit (bit 21 of CR4)** -- Enables supervisor-mode access
  prevention (SMAP) when set.

* CR4.PKE

  **Protection-Key-Enable Bit (bit 22 of CR4)** -- Enables 4-level paging to
  associate each linear address with a protection key. The PKRU register 
  specifies, for each protection key, whether user-mode linear addresses with
  that protection key can be read or written. This bit also enables access to
  the PKRU register using the RDPKRU and WRPKRU instructions.
