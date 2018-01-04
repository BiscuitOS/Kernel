Interrupt 1 -- Debug Exception (#DB)
----------------------------------------------------

### Description

  Indicates that one or more of several debug-exception conditions has
  been detected. Whether the exception is a fault or a trap depends on the
  condition.

  ```
    -----------------------------------------------------------------
    | Exception Condition                         | Exception Class |
    -----------------------------------------------------------------
    | Instrcution fetch breakpoint                | fault           |
    -----------------------------------------------------------------
    | Data read or write breakpoint               | Trap            |
    -----------------------------------------------------------------
    | I/O read or write breakpoint                | Trap            |
    -----------------------------------------------------------------
    | General detect condition (in conjunction    | Fault           |
    |   with in-circuit emulation)                |                 |
    -----------------------------------------------------------------
    | Single-step                                 | Trap            |
    -----------------------------------------------------------------
    | Task-switch                                 | Trap            |
    -----------------------------------------------------------------
  ```

### Exception Error Code

  None. An exception handler can examine the debug registers to determine
  while condition caused the exception.

### Saved Instruction Pointer

  * Fault

    Saved contents of CS and EIP registers point to the instruction
    that generated the exception.

  * Trap

    Saved contents of CS and EIP registers point to the instruction
    following the instruction that generated the exception.
           

### Program State Change

  * Fault

    A program-state change does not accompany the debug exception, because
    the exception occurs before the fault instruction is executed. The 
    program can resume normal execution upon returning from the debug
    exception handler.

  * Trap

    A program-state change does accompany the debug exception, because
    the instruction or task switch being executed is allowed to complete
    before the exception is generated. However, the new state of the program
    is not corrupted and execution of the program can continue reliably.

  Any debug exception inside and RTM region causes a transactional abort
  and, by default, redirects control flow to the fallback instruction
  address. If advanced debugging of RTM transactional regions has been
  enabled, any transactional abort due to a debug exception instead causes
  execution to roll back to just before the XBEGIN instruction and then
  delivers a #DB.

### File list
 
