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
