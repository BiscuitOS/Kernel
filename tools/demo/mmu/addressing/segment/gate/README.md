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

# Stack Switching

Whenever a call gate is used to transfer program control to a more privileged
nonconforming code segment (that is, when the DPL of the nonconforming 
destination code segment is less than the CPL), the processor automatically
switches to the stack for the destination code segment's privilege level. This
stack switching is carried out to prevent more privileged procedures from 
crashing due to insufficient stack space. It also prevents less privileged 
procedures from interfering (by accident or intent) with more privileged 
procedures through a shared stack.

Each task must define up to 4 stacks: one for applications code (running at 
privilege level 3) and one for each of the privilege level 2,1, and 0 that are
used. (If only two privilege levels are used [3 and 0], then only two stacks
must be defined.) Each of these stacks is located in a separate segment and is
identified with a segment selector and an offset into the stack segment (a
stack pointer).

The segment selector and stack pointer for the privilege level 3 stack is
located in the SS and ESP registers, respectively, when privilege-level-3 code
is being executed and is automatically stored on the called procedure's stack
when a stack switch occurs.

Pointers to privilege level 0, 1, and 2 stacks are stored in the TSS for the 
currently running task (See Figure). Each of these pointer consists of a
segment selector and a task pointer (loaded into the ESP register). These
initial pointers are strictly read-only values. The procedure does not change
them while the task is running. They are used only to create new stacks when
calls are made to more privilege levels (numberically lower privilege levels).
These stack are disposed of when a return is made from the called procedure.
The next time the procedure is called, a new stack is created using the inital
stack pointer. (The TSS does not specify a stack for privilege level 3 because
the processor does not allow a transfer of program control from a procedure
running at a CPL of 0, 1, or 2 to a procedure running at a CPL of 3, except
on a return.)

The operating system is responsible for creating stacks and stack-segment 
descriptor for all the privilege levels to be used and for loading initial
pointers for these stacks into the TSS. Each stack must be read/write
accessible (as specified in the type field of its segment descriptor) and must
contain enough space (as specified in the limit field) to hold the following 
items:

* The contents of the SS, ESP, CS, and EIP registers for the calling procedure.

* The parameters and temporary variables required by the called procedure.

* The EFLAGS register and error code, when implicit calls are made to an
  exception or interrupt handler.

The stack will need to require enough spece to contain many frames of these
items, because procedures often call other procedures, and an operating system
may support nesting of multiple interrupts. Each stack should be large enough
to allow for the worst case nesting scenario at its privilege level.

(If the operating system does not use the processor's multitasking mechanism,
it still must create at least on TSS for this stack-related purpose.)

When a procedure call through a call gate results in a change in privilege
level, the processor performs the following steps to switch stacks and begin
execution of the called procedure at a new privilege level:

1. Uses the DPL of the destination code segment (the new CPL) to select a 
   pointer to the new stack (segment selector and stack pointer) from the TSS.

2. Reads the segment selector and stack pointer for the stack to be switched to
   from the current TSS. Any limit violations detected while reading the 
   stack-segment selector, stack pointer, or stack-segment descriptor cause
   an invalid TSS (#TS) exception to be generated.

3. Checks the stack-segment descriptor for the proper privilege and type and
   generates an invalid TSS (#TS) exception if violations are detected.

4. Temporarily saves the current values of the SS and ESP registers.

5. Loads the segment selector and stack pointer for the new stack in the SS
   and ESP register.

6. Pushes the temporarily saved valued for the SS and ESP registers (for the
   calling procedure) onto the new stack.

7. Copies the number of parameter specified in the parameter count field of the
   call gate from the calling procedure's stack to the new stack. If the count
   is 0, no parameters are copied.

8. Pushes the return instruction pointer (the current contents of the CS and 
   EIP registers) onto the new stack.

9. Loads the segment selector for the new code segment and the new instruction
   pointer from the call gate into the CS and EIP registers, respectively, and
   begins execution of called procedure.

```

    Calling Procedure's Stack               Called Procedure's Stack

     |                     |                 |                    |
     +---------------------+                 +--------------------+
     |     Parameter 1     |                 |     Calling SS     |
     +---------------------+                 +--------------------+
     |     Parameter 2     |                 |     Calling ESP    |
     +---------------------+                 +--------------------+
     |     Parameter 3     |<---- ESP        |     Parameter 1    |
     +---------------------+                 +--------------------+
     |                     |                 |     Parameter 2    |
     +---------------------+                 +--------------------+
     |                     |                 |     Parameter 3    |
     +---------------------+                 +--------------------+
     |                     |                 |     Calling CS     |
     +---------------------+                 +--------------------+
     |                     |                 |     Calling EIP    |<--- ESP
     +---------------------+                 +--------------------+
     |                     |                 |                    |
     +---------------------+                 +--------------------+
     |                     |                 |                    |
     +---------------------+                 +--------------------+
     |                     |                 |                    |
```
