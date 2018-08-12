Basic Program Execution Registers
-------------------------------------------------

IA-32 architecture provides 16 basic program execution register for use in
general system and application program. These registers can be grouped as
follow:

* General-purpose register

  These eight registers are available for storing operands and pointers.

* Segment registers

  These registers hold up to six segment selectors.
 
* EFLAGS (program status and control) register

  The `EFLAGS` register report on the status of the program being executed 
  and allows limited (application-program level) control of the processor.

* EIP (Instruction pointer) register

  The `EIP` register contains a 32-bit pointer to the next instruction to
  be executed.

## General-Purpose Register

  The 32-bit general-purpose register `EAX`, `EBX`, `ECX`, `EDX`, `ESI`
  `EDI`, `EBP`, and `ESP` are provided for holding the following items:

  * Operands for logical and arithmetic operations.

  * Operands for address calculations

  * Memory pointers

  Although all of these registers are available for general storage of
  operands, results, and pointers, caution should be used when referencing
  then `ESP` register. The `ESP` register holds the stack pointer and as a
  general rule should not be used for another purpose.

  Many instructions assign specific registers to hold operands. For example,
  string instructions use the contents of the `ECX`, `ESI`, and `EDI` 
  registers as operands. When using a segmented memory model, some 
  instructions assume that pointers in certain registers are relative to 
  specific segments. For instance, some instructions assume that a pointer
  in the `EBX` register points to a memory location in the `DS` segment.

  ```
    General-Purpos Register

    31                                                    0 
    +-----------------------------------------------------+
    |                                                     | EAX
    +-----------------------------------------------------+
    |                                                     | EBX
    +-----------------------------------------------------+
    |                                                     | ECX
    +-----------------------------------------------------+ 
    |                                                     | EDX
    +-----------------------------------------------------+
    |                                                     | ESI
    +-----------------------------------------------------+
    |                                                     | EDI
    +-----------------------------------------------------+
    |                                                     | EBP
    +-----------------------------------------------------+
    |                                                     | ESP
    +-----------------------------------------------------+

                            Segment Register

                            15                            0
                            +-----------------------------+
                            |                             | CS
                            +-----------------------------+
                            |                             | DS
                            +-----------------------------+
                            |                             | SS
                            +-----------------------------+
                            |                             | ES
                            +-----------------------------+
                            |                             | FS
                            +-----------------------------+
                            |                             | GS
                            +-----------------------------+


    Program Status and Control Register

    31                                                    0
    +-----------------------------------------------------+
    |                                                     | EFLAGS
    +-----------------------------------------------------+
    
    Instruction Pointer

    31                                                    0
    +-----------------------------------------------------+
    |                                                     | EIP
    +-----------------------------------------------------+
    
  ```
  
  The following is a summary of special uses for general-purpose register:

  * EAX

    Accumulator for operands and results data.

  * EBX

    Pointer to data in the DS segment

  * ECX

    Counter for string and loop operations

  * EDX

    I/O pointer

  * ESI

    Pointer to data in the segment pointed to by the DS register. source
    pointer for string operations

  * EDI

    Pointer to data (or destination) in the segment pointer to by the ES
    register. Destination pointer for string operations.

  * ESP

    Stack pointer (in the SS segment)

  * EBP

    Pointer to data on the stack (int the SS segment).

  As shown in above Figure, the lower 16 bits of the general-purpose register
  map directly to the register set found in the `8086` and Intel `286` 
  processors and can be referenced with the names `AX`, `BX`, `CX`, `DX`, 
  `BP`, `SI`, `DI` and `SP`. Each of the lower two bytes of the `EAX`, `EBX`,
  `ECX`, and `EDX` registers can be referenced by the names `AH`, `BH`, `CH`,
  and `DH` (high bytes) and `AL`, `BL`, `CL` and `DL` (low bytes).

  ```
    General-Purpose Register

    31                       16 15          8 7           0 16-bit  32-bit
    +--------------------------+-------------+------------+
    |                          |      AH     |     AL     |   AX     EAX
    +--------------------------+-------------+------------+
    |                          |      BH     |     BL     |   BX     EBX
    +--------------------------+-------------+------------+
    |                          |      CH     |     CL     |   CX     ECX
    +--------------------------+-------------+------------+
    |                          |      DH     |     DL     |   DX     EDX
    +--------------------------+--------------------------+
    |                          |             SI           |          ESI
    +--------------------------+--------------------------+
    |                          |             DI           |          EDI
    +--------------------------+--------------------------+
    |                          |             BP           |          EBP
    +--------------------------+--------------------------+
    |                          |             SP           |          ESP
    +--------------------------+--------------------------+

  ```

## Segment Register

  The segment register (`CS`, `DS`, `SS`, `ES`, `FS`, and `GS`) hold 16-bit
  segment selectors. A segment selector is a specical pointer that identifies
  a segment in memory. To access a particular segment in memory, the segment
  selecor for that segment must be present in the appropriate segment 
  register.

  When writing application code, programmers generally create segment selector
  with assembler directives and symbols. The assembler and other tools then
  create the actual segment selector values associated with these directives
  and symbols. If writing system code, programmers may need to create segment
  selectors directly. 

  How segment registers are used depends on the type of memory management 
  model that the operating system or executive is using. When using the flat
  (unsegmented) memory model, segment registers are loaded with segment 
  selectors that point to overlapping segments, each of which begins at
  address 0 of the linear address space (See Figure). These overlapping
  segments then comprise the linear address space for the prgoram. Typically,
  two overlapping segments are defined: one for code and another for data
  and stacks. The `CS` segment register points to the code segment and
  all the other segment registers point to the data and stack sengment.

  When using the segmented memory model, each segment register is ordinarily
  loaded with a different segment selecotr so that each segment register
  points to a different segment within the linear address space. At any time,
  a program can thus access to six segments in the linear address space.
  To access a segment no pointed to by one of the segment register, a program
  must first load the segment selector for the segment to be accessed into
  a segment register.

  ```
  Use of segment Registers for Flat Memory Model


                                                Linear Address
                                               Space for Program
                                               +----------------+
                                               |                |
                                               |                |
                                               |                |
                                               |  Overlapping   |
                                               |  Segmnet of up |
                                               |  to 4GBytes    |
   Segment Register                            |  Beginning at  |
  +----------------+                           |  Address 0     |
  |                | CS -----o                 |                |
  +----------------+         |                 |                |
  |                | DS -----o                 |                |
  +----------------+         |                 |                |
  |                | SS -----o                 |                |
  +----------------+         |                 |                |
  |                | ES -----o                 |                |
  +----------------+         |                 |                |
  |                | FS -----o                 |                |
  +----------------+         |                 |                |
  |                | GS -----o---------------> +----------------+
  +----------------+
  The segment selector in each segment 
  register pointer to an overlapping 
  segment in the  linear adddres space

  ```

  ```

                                                               }----o
                                                                    |
                                          +----------------+        |
                                          |                |        |
   Segment Register                       |  Code Segment  |        |
  +----------------+                      |                |        |
  |                | CS ----------------> +----------------+        |
  +----------------+                                                |
  |                | DS ----------------> +----------------+        |
  +----------------+                      |                |        |
  |                | SS --------------o   |  Data Segment  |        |
  +----------------+                  |   |                |        |
  |                | ES ----------o   |   +----------------+        |
  +----------------+              |   |                             |
  |                | FS --------o |   o-->+----------------+        |
  +----------------+            | |       |                |        |
  |                | GS -----o  | |       | Stack Segment  |        |
  +----------------+         |  | |       |                |  All Segment are   
                             |  | |       +----------------+  mapped to the 
                             |  | |                           same linear-
                             |  | o-----> +----------------+  address space                                 
                             |  |         |                |        |
                             |  |         |  Data Segment  |        |
                             |  |         |                |        |
                             |  |         +----------------+        |
                             |  |                                   |
                             |  o-------->+----------------+        |                             
                             |            |                |        |
                             |            |  Data Segment  |        |
                             |            |                |        |
                             |            +----------------+        |
                             |                                      |
                             o----------->+----------------+        |                             
                                          |                |        |
                                          |  Data Segment  |        |
                                          |                |        |
                                          +----------------+        |
                                                               }----o


  ```

  Each of the segment register is associated with one of threee types of
  storage: `code`, `data` or `stack`. For example, the `CS` register
  contains the segment selector for the **code segment**, where the
  instructions being executed are stored. The processor fetches instructions
  from the code segment, using a logical adress that consists of the segment
  selector in the `CS` register and the contents of the `EIP` register.
  The `EIP` register contains the offset within the code segment of the
  next instruction to be executed. The `CS` register cannot be loaded
  explicitly by an application program. Instead, it is loaded implicitly by
  instructions or internal processor operations that change program control
  (such as, procedure calls, interrupt handling, or task switching).

  The `DS`, `ES`, `FS` and `GS` register point to four **data segments**.
  The availability of four data segments permits efficient and secure
  access to different types of data structures. For example, four separate
  data semgents might be created: one for the data structures of the current
  module, another for the data exported from a higher-level module, a third
  for a dynamically created data structure, and a foure for data shared
  with another program. To access additional data segments, the application
  program must load segment selectors for these segments into the `DS`, `ES`,
  `FS`, and `GS` register, as needed.

  The `SS` register contains the segment selector for the stack segment, 
  where the procedure stack is stored for the program, task, or handler 
  currently being executed. All stack operations use the `SS` register to
  find the stack segment. Unlike the `CS` register, the `SS` register can
  be loaded explicitly, which permits application program to set up multiple
  stacks and switch among them.
