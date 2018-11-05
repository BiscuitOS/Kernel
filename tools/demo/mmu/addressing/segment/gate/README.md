Gate
---------------------------------------

To provide controlled access to code segment with different privilege levels,
the processor provides special set of descriptors called gate descriptors. 
These are four kinds of gate descriptors:

* Call gates

* Trap gates

* Interrupt gates

* Task gates

# Call Gates

Call gates facilitate controlled transfers of program control between different
privilege levels. They are typically used only in operating systems or 
executives that use the privilege-level protection mechanism. Call gates are
also useful for transferring program control between 16-bit and 32-bit code 
segments.

![Call-Gate Descriptor](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel/MMU000404.png)

Figure show the format of a call-gate descriptor. A call-gate descriptor may
reside in the GDT or in an LDT, but not in the interrupt descriptor table (IDT
). It performs six functions:

* It specifies the code segment to be accessed.

* It defines an entry point for a procedure in the specified code segment.

* It specifies the privilege level required for a caller trying to access
  the procedure.

* If a stack switch occurs, it specifies the number of optional parameters to
  be copied between stacks.

* It defines the size of values to be pushed onto the target stack: 16-bit 
  gates force 16-bit pushes and 32-bit gates force 32-bit pushes.

* It specifies whether the call-gate descriptor is valid.

##### segment selector field 

The segment selector field in a call gate specifies the code segment to be 
accessed. 

##### offset field

The offset field specifies the entry point in the code segment. This entry 
point is generally to the first instruction of a specific procedure. 

##### DPL field 

The DPL field indicates the privilege level of the call gate, which in turn is
the privilege level required to access the selected procedure through the gate.

##### P flag

The P flag indicates whether the call-gate descriptor is valid. (The presence
of the code segment to which the gate points is indicated by the P flag in the
code segment descriptor.) 

##### Parameter count field

The parameter count field indicates the number of parameters to copy from the
calling procedures stack to the new stack if a stack switch occurs. The 
parameter count specifies the number of words for 16-bit call gates and 
doublewords for 32-bit call gates.

Note that the P flag in a gate descriptor is normally always set to 1. If it is
set to 0, a not present (#NP) exception is generated when a program attempts to
access the descriptor. The operating system can use the P flag for special 
purposes. For example, it could be used to track then number of times the gate
is used. Here, the P flag is initially set to 0 causing a trap to the not-
present exception handler. The exception handler then increments a counter and
sets the P flag to 1, so that no returning from the handler, the gate 
descriptor will be valid.

## Accessing a Code Segment Though a Call Gate

To access a call gate, a far pointer to the gate is provided as a target
operand in a CALL or JMP instruction. The segment selector from this pointer
identifies the call gate (See Figure); the offset from the pointer is required,
but not used or checked by the processor. (The offset can be set to any value.)

When the processor has accessed the call gate, it uses the segment selector 
from the call gate to locate the segment descriptor for the destination code
segment. (This segment descriptor can be in the GDT or the LDT.) It then 
combines the base address from the code-segment descriptor with the offset
from the call gate to form the linear address of the procedure entry point in
the code segment.

As shown in Figure, four different privilege levels are used to check the
validity of a program control transfer through a call gate:

* The CPL (current privilege level)

* The RPL (requestor's privilege level) of the call gate's selector.

* The DPL (descriptor privilege level) of the call gate descriptor.

* The DPL of the segment descriptor of the destination code segment.

The C flag (conforming) in the segment descriptor for the destination code 
segment is also checked.

![Call-Gate Mechanism](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel/MMU000408.png)

```
CS Register
+-----------------+-----+
|                 | CPL |------------o
+-----------------+-----+            |
                                     |
Call-Gate Selector                   |
+-----------------+-----+            |
|                 | RPL |---------o  |      +-----------------+
+-----------------+-----+         |  o----->|                 |
                                  |         |                 |
Call Gate (descriptor)            o-------->|                 |
+----------+-----+------+                   | Privilege Check |
|          | DPL |     -|------------------>|                 |
+----------+-----+------+                   |                 |
+-----------------------+         o-------->|                 |
|                       |         |         +-----------------+
+-----------------------+         |
                                  |
Destination Code-Segment          |
Descriptor                        |
Call Gate (descriptor)            |
+----------+-----+------+         |
|          | DPL |     -|---------o
+----------+-----+------+
+-----------------------+
|                       |
+-----------------------+
```

The privilege checking rules are different depending on whether the control
transfer was initiated with a CALL or a JMP instruction, as follow:

* CALL

  CPL <= call gate DPL; RPL <= call gate DPL

  Destination conforming code segment DPL <= CPL

  Destination nonconforming code segment DPL <= CPL

* JMP

  CPL <= call gate DPL; RPL <= call gate DPL

  Destination conforming code segment DPL <= CPL

  Destination nonconforming code segment DPL = CPL

The DPL field of the call-gate descriptor specifies the numerically highest 
privilege level from which a calling procesordure can access the call gate.

The RPL of the segment selector to a call gate must satisfy the same test as
the CPL of the calling procedure; that is, the RPL must be than or equal to 
the DPL of the call gate.

If the privilege checks between the calling procedure and call gate are 
successful, the procedure then checks the DPL of the code-segment descriptor
against the CPL of the calling procedure. Here, the privilege check rules vary
between CALL and JMP instructions.

More example of accessing call gates at various privilege levels, please see
**tools/demo/mmu/addressing/segment/privilege/pl.c** about **Call gates**.

