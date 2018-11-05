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


