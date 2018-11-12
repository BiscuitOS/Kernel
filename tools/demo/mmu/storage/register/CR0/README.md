CR0 (Control Register 0)
------------------------------------------------

Control register CR0 determine operating system mode of the processor and the
characteristics of the currently executing task. The register are 32 bits in
all 32-bit mode.

**CR0** contains system control flags that control operating mode and state of
the processor.

![CR0_Register](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel/MMU000414.png)

* CR0.PG

  **Paging(bit 31 of CR0)** -- Enables paging when set; disables paging when
  clear. When paging is disabled, all linear address are treated as physical
  address. The PG flag has no effect if the PE flag (bit 0 of register CR0) is
  not also set; setting the PG flag when the PE flag is clear causes a general-
  protection exception (#GP).

* CR0.CD

  **Cache Disable(bit 30 of CR0)** -- When the CD and NW flag are clear, 
  caching of memory locations for the whole of physical memory in the
  processor's internal (and external) caches is enabled. When the CD flag is
  set, caching is restricted as described in Table. To prevent the processor
  from accessing and updating its caches, the CD flag must be set and the
  caches must be invalidated so that no cache hits can occur.

* CR0.NW

  **Not Write-through(bit 29 of CR0)** -- When the NW and CD flags are clear,
  write-back or write-through is enabled for writes that hit the cache and
  invalidation cycles are enabled.

* CR0.AM

  **Alignment Mask(bit 18 of CR0)** -- Enables automatic alignment checking 
  when set; disables alignment checking when clear. Alignment checking is 
  performed only when the AM flag is set, the AC flag in the EFLAGS register
  is set, CPL is 3, and the processor is operating in either protected or 
  virtual-8086 mode.

* CR0.WP

  **Write Protect(bit 16 of CR0)** -- When set, inhibits supervisor-level 
  procedure from writing into read-only pages; when clear, allows supervisor-
  level procedures to write into read-only pages (regardless of the U/S bit
  setting). The flag facilitiates implementation of the copy-on-write method
  of creating a new process (forking) used by operating systems such as UNIX. 

* CR0.PE

  **Protection Enable(bit 0 of the CR0)** -- Enables protected mode when set;
  enables real-address mode when clear. This flag does not enable paging 
  directly. It only enables segment-level protection. To enable paging, both 
  the PE and PG flags must be set.
