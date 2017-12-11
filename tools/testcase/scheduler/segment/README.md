Segment Registers and Segment Selectors
-----------------------------------------------------------

  To reduce address translation time and coding complexity, the processor
  provides registers for holding up to 6 segment selectors (See Figure).
  Each of these segment register support a spcific kind of memory 
  reference (code, stack, or data). For virtually any kind of prgoram
  execution to take place, at least the code-segment (CS), data-segment (DS),
  and stack-segment (SS) registers must be loaded with vaild segment 
  selectors. The processor also provides thress additional data-segment
  registers (ES, FS and GS), which can be used to make additional data
  segments available to the currently executing program (or task).

  For a program to access a segment, the segment selector for the segment
  must have been loaded in one of the segment registers. So, although a 
  system can define thousands of segments, only 6 can be available for 
  immediate use. Other segments can be made available by loading their
  segment selectors into these registers during program execution.

  ```
       Visible Part            Hidden Part
    ---------------------------------------------------------------   
    | Segment Selector  | Base Address, Limit, Access Information | CS
    ---------------------------------------------------------------
    |                   |                                         | SS
    ---------------------------------------------------------------
    |                   |                                         | DS
    ---------------------------------------------------------------
    |                   |                                         | ES
    ---------------------------------------------------------------
    |                   |                                         | FS
    ---------------------------------------------------------------
    |                   |                                         | GS
    ---------------------------------------------------------------

  ```

  Every segment register has a "visible" part and a "hidden" part. (The 
  hidden part is sometimes referred to as a "descriptor cache" or "shadow
  register".) When a segment selector is loaded into the visible part of a 
  segment register, the processor also loads the hidden part of the segment
  register with the base address, segment limit, and access control
  information from the segment descriptor pointed to by the segment 
  selector. The information cached in the segment register (visible and 
  hidden) allows the processor to translate addresses without taking exta
  bus cycles to read the base address and limit from the segment descriptor.
  In systems in which multiple processors have access to the same descriptor
  tables, it is the responsibility of software to reload the segment 
  register when the descriptor table are modified. If this is not done, an
  old segment descriptor cached in segment register might be used after
  its memory-resident version has been modified.

  Two kinds of load instructions are provided for loading the segment
  registers:

  * Direct load instructions such as the `MOV, POP, LDS, LES, LSS, LGS`
    and `LFS` instructions explicitly reference the segment reigsters.

  * Implied load instruction such as the far pointer versions of the 
    `CALL, JMP` and `RET` instructions, the `SYSENTER` and `SYSEXIT`
    instructions, and `IRET, INTn, INTO` and `INT3` instructions. These
    instructions change the contents of the `CS` register (and sometimes
    other segment registers) as an incidental part of their operation.

  The `MOV` instruction can also be used to store the visible part of a
  segment register in a general-purpose register. 
