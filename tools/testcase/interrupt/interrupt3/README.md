Interrupt 3 -- Breakpoint Exception (#BP)
----------------------------------------------------

### Description

  Indicates that a breakpoint instruction (INT 3, opcode CCH) was executed,
  causing a breakpoint trap to be generated. Typically, a debugger sets a 
  breakpoint by replacing the first opcode byte of an instruction with the
  opcode for the INT 3 instruction. (The INT 3 instruction is one byte
  long, which makes it easy to replace an opcode in a code segment in RAM
  with the breakpoint opcode). The operating system or a debugging tool
  can use a data segment mapped to the same physical address space as the
  code segment to place an INT 3 instruction in place where it is desired to
  call the debugger.

### Exception Error Code

  None

### Saved Instruction Pointer

  Saved contents of CS and EIP registers point to the instruction following
  the INT 3 instruction.

### Program State Change

  Even though the EIP points to the instruction following the breakpoint
  instruction, the state of the program is essentially unchanged because
  the INT 3 instruction does not effect any register or memory locations.
  The debugger can thus resume the suspended program by replacing the INT3
  instruction that caused the breakpoint with the original opcode and 
  decrementing the saved contents of the EIP register. Upon returning from
  the debugger, program execution resumes with the replaced instruction.

### File list
 
