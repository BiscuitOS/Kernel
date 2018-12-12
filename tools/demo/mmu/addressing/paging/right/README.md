Access Rights on Paging
--------------------------------------------

There is a translation for a linear address if the processor described 32-bit
paging mode completes and produces a physical address. Whether an access is 
permitted by a translation is determined by the access rights specified by the
paging-structure entries controlling the translation; Paging-mode modified in
CR0, CR4, and the IA32_EFER MSR; EFLAGS.AC; and the mode of the access.

# Determination of Access Rights

Every access to a linear address is either a supervisor-mode access or a user-
mode access. For all instruction fetures and most data accesses, the 
distinction is determined by the current privilege level (CPL): accesses made
while CPL < 3 are supervisor-mode accesses, while accesses made while CPL = 3
are user-mode accesses.

Some operations implicitly access system data structures with linear address;
the resulting accesses to those data structure are supervisor-mode accesses
regardless of CPL. Examples of such accesses include the following:

* Accesses to the global descriptor table (GDT) or local descriptor table (LDT)
  to load a segment descriptor.

* Accesses to the interrupt descriptor table (IDT) when delivering an interrupt
  or exception.

* Accesses to the task-state segment (TSS) as part of a task switch or change 
  of CPL. 

All these accesses are called implicit supervisor-mode accesses regardless of
CPL. Other accesses made while CPL < 3 are called explicit supervisor-mode
accesses.

Access rights are also controlled by the mode of a linear address as specified
by the paging-structure entries controlling the translation of the linear
address. If the U/S flag (bit 2) is 0 in at least one of the page-structure
entries, the address is a supervisor-mode address. Otherwise, the address is a
user-mode address.

The following items detail how paging determines access rights:

### supervisor-mode accesses

-- Data may be read (implicitly or explicitly) from any supervisor-mode 
   address.

-- Data reads from user-mode pages.

```
   Access rights depend on the value of CR4.SMAP:

   * If CR4.SMAP = 0, data may be read from any user-mode address with a 
     protection key for which read access is permitted.

   * If CR4.SMAP = 1, access rights depend on the value of EFLAGS.AC and 
     whether the access is implicit or explicit:

     -- If EFLAGS.AC = 1 and the access is explicit, data may be read from any
        user-mode address with a protetion key for which read access is 
        permitted.

     -- If EFLAGS.AC = 0 or access is implicit, data may not be read from any
        user-mode address.
```

-- Data writes to supervisor-mode address.

```
   Access right depend on the value of CR0.WP:

   * If CR0.WP = 0, data may be written to any supervisor-mode address.

   * If CR0.WP = 1. data may be written to any supervisor-mode address with
     a translation for which the R/W flag (bit 1) is 1 in every paging-
     structure entry controlling the translation; data may not be written to
     any supervisor-mode address with a translation for which the R/W flag is
     0 in any paging-structure entry controlling the translation.
```    

-- Data write to user-mode address.

```
   Access right depend on the value of CR0.WP:

   * If CR0.WP = 0. access rights depend on the value of CR4.SMAP:

     -- If CR4.SMAP = 0, data may be written to any user-mode address with a
        protection key for which write access is permitted.

     -- If CR4.SMAP = 1, access rights depend on the value of EFLAGS.AC and
        whether the access is implicit or explicit:

        * If EFLAGS.AC = 1 and the access is explicit, data may be written to
          any user-mode address with a protection key for which write access
          is permitted.

        * If EFLAGS.AC = 0 or the access is implicit, data may not be written
          to any user-mode address.

   * If CR0.WP = 1, access rights depend on the value of CR4.SMAP:

     -- If CR4.SMAP = 0, data may be written to any user-mode address with a 
        translation for which the R/W flag is 1 in every paging-structure entry
        controlling the translation and with a protection key for which write
        access is permitted; data may not be written to any user-mode address
        with a translation for which the R/W flag is 0 in any paging-structure
        entry controlling the translation.

     -- If CR4.SMAP = 1, access rights depend on the value of EFLAGS.AC and 
        whether the access is implicit or explict:

        * If EFLAGS.AC = 1 and the access is explicit, data may be written to
          any user-mode address with a translation for which the R/W flag is 1
          in every paging-structure entry controlling the translation and with
          a protection key for which write access is permitted; data may not
          be written to any user-mode address with a translation for which the
          R/W flag is 0 in any paging-structure entry controlling the
          translation.

        * If EFLAGS.AC = 0 or the access is implicit, data may not be written
          to any user-mode address.
```
  
### User-mode access

-- Data read

```
   Access rights depends on the mode of the linear address:

   * Data may be read from any user-mode address with a protection key for 
     which read access is permitted.

   * Data may not be read from any supervisor-mode address.
```

-- Data write

```
   Access rights depend on the mode of the linear address:

   * Data may be written to any user-mode address with a translation for which
     the R/W flag is 1 in every paging-structure entry controlling the 
     translation and with a protection key for which write access is permitted.
     
   * Data may not be written to any supervisor-mode address.
```

A processor may cache information from the paging-structure entries in TLB and
paging-structure caches. These structures may include information about access
rights. The processor may enforce access rights based on the TLBs and 
paging-structure caches instead of on the paging structure in memory.

This fact implies that, if software modifies a paging-structure entry to change
access rights, the processor might not use that change for a subsequent access
to an effected linear address.
