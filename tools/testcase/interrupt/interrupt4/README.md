Interrupt 4 -- Overflow Exception (#OF)
----------------------------------------------------

### Description

  Indicates that an overflow trap occurred when an INTO instruction was
  executed. The INTO instruction checks the state of the OF flag in the 
  EFLAGS register. If the OF flag is set, an overflow trap is generated.

  Some arithmetic instructions (such as the ADD and SUB) perform both 
  signed and unsigned signed arithmetic. These instructions set the OF
  and CF flags in the EFLAGS register to indicate signed overflow and
  unsigned overflow, respectively. When performing arithmetic on signed 
  operands, the OF flag can be tested directly or the INTO instruction
  can be used. The benefit of using the INTO instruction is that if the
  overflow exception is detected, an exception handler can be called 
  automatically to handler the overflow condition.

### Exception Error Code

  None

### Saved Instruction Pointer

  The saved contents of CS and EIP register point to the instruction
  following the INTO instruction.

### Program State Change

  Even though the EIP points to the instruction following the INTO 
  instruction, the state of the program is essentially unchanged because
  the INTO instruction does not affect any register or memory location.
  The program can thus resume normal execution upon returning from the 
  overflow exception handler.

### File list
 
