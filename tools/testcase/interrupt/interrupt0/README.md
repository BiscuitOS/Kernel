Interrupt 0 -- Divide Error Exception (#DE)
----------------------------------------------------

### Description

  Indicates the divisor operand for a DIV or IDIV instruction is 0 or that
  the result cannot be represented in the number of bits specified for
  the destination operand.

### Exception Error Code

  None

### Saved Instruction Pointer

  Saved contents of CS and EIP registers point to the instruction that
  generated the exception.

### Program State Change

  A program-state change does not accompany the divide error, because
  the exception occurs before the faulting instruction is executed.

### File list
 
