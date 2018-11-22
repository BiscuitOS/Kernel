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
   

  
