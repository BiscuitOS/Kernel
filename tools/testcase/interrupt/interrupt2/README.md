Interrupt 2 -- NMI Interrupt
----------------------------------------------------

### Description

  The nonmaskable interrupt (NMI) is generated externally by asserting
  the processor's NMI pin or through an NMI request set by the I/O
  APIC to the local APIC. This interrupt causes the NMI interrupt 
  handler to be called.

### Exception Error Code

  Not applicable.

### Saved Instruction Pointer

  The processor always takes an NMI interrupt or instruction boundary.
  The saved contents of CS and EIP registers point to the next
  instruction to be executed at the point the interrupt is taken.

### Program State Change

  The instruction executing when an NMI interrupt is received is completed
  before the NMI is generated. A program or task can thus be restarted upon
  returning from an interrupt handler without loss of continuity, provided
  the interrupt handler saves the state of the processor before handling 
  the interrupt restores the processor's state prior to return.

### File list
 
