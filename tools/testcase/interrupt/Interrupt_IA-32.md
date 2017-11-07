Interrupt and Execption Handling
------------------------------------------------------

This chapter describes the interrupt and exception-handling mechanism
when operating in protected mode on an Intel IA-32 processor. Most of
the inforation provided here also applies to interrupt and exception
mechanisms used in real-address, virtual-8086 mod.

### Contents

  1. Interrupt and Exception overview

  2. Exception and Interrupt vector

  3. Source of Interrupts

  4. Source of Exceptions

  5. Exception classification

  6. Program or Task restart

  7. Nonmaskable Interrupt

  8. Enable and Disable Interrupts

  9. Priority among simultaneous Execption and Interrupt

  10. Interrupt Descriptor Table (IDT)

  11. IDT Descriptors

  12. Exception and Interrupt Handling

  13. ERROR Code

  14. Exception and Interrupt Reference

### 1. Interrupt and Exception overview

  Interrupts and exceptions are event that indicate that a condition
  exists somewhere in the system, the processor, or within the 
  currently executing from the currently running program or task to
  a special software routine or task called an interrupt handler or 
  an exception handler. The action taken by a processor in response
  in response to an interrupt or exception is referred to as servicing
  or handling the interrupt or exception.

  Interrupts occur at random times during the execution of a program,
  in the response to signals from hardware. System hardware uses
  interrupts to handle event external to the processor, such as requests
  to service peripheral devices. Software can also generate interrupts
  by excuting the INT n instruction.

  Exceptions occur when the processor detects an error condition while
  executing an instruction, such as division by zero. The processor
  detects a variety of error conditions including protection violations,
  page faults, and internal machine faults.

  When an interrupt is received or an exception is detected, the currently
  running procedure or task is suspended while the processor executes 
  an interrupt or exception handler. When execution of the handle is
  complete, the processor resumes execution of the interrupt procedure
  or task. The resumption of the interrupted procedure or task happens
  without loss of program continuity, unless recovery from an exception
  was not possible or an interrupt caused the currently running program to
  be terminated.

  This chapter describes the processor's interrupt and exception-handling
  mechanism, when operating in protected mode. A description of the
  exceptions and the conditions that cause them to be generated is given
  at the end of this chapter.

### 2. Exception and Interrupt vectors

  To aid in handling exception and interrupts, each architecturally 
  defined exception and each interrup condition requiring specical
  handling by the processor is assigned a unique identification number,
  called a vector number. The processor uses the vector number assigned
  to an exception or interrupt as an index into the interrupt descriptor
  table (IDT). The table provides the entry point to an exception or
  interrupt handler.

  The allowable range for vector number is 0 to 255. Vector numbers in
  the range 0 through 31 are reserved by the IA-32 architectures for 
  architecture-defined exceptions and interrupts. Not all of the vector
  numbers in this range have a currently defined function. The unassigned
  vector numbers in this range are reserved. Do not use the reserved
  vector numbers.

  Vector numbers in the range 32 to 255 are designated as user-defined
  interrupts and are not reserved by the IA-32 architecture. These interrupts
  are generally assigned to external I/O devices to enable those devices
  to send interrupts to the processor through one of the external hardware
  interrupt mechanisms.


### 3. Source of Interrupts

  The processor receives interrupts from two sources:

  * External (hardware generated) interrupts.

  * Software-generated interrupts.

#### 3.1 External Interrupt

  Extern al interrupts are received through pins on the processor or
  through the local APIC. The primary interrupt pins on IA-32 are the 
  LINT[1:0] pins, which are connected to the local APIC. When the local
  APIC is enabled, the LINT[1:0] pins can be programmed through the APIC's
  local vector table (LVT) to be associated with any of the processor's
  exception or interrupt vectors.

  When the local APIC is global/hardware disabled, these pins are configured
  as INTR and NMI pins, respectively. Asserting the INTR pin signals the
  processor that an external interrupt has occurred. The processor reads
  from the system bus the interrupt vector number provided by an external
  interrupt controller, such as an 8259A. Asserting the NMI pin signals a
  non-maskable interrupt (NMI), which is assigned to interrupt vector 2.

  **Table 1-1. Protected-Mode Exception and Interrupts**

  ![IDT_0](https://github.com/EmulateSpace/PictureSet/blob/master/interrupt/interrupt0.png)
  ![IDT_1](https://github.com/EmulateSpace/PictureSet/blob/master/interrupt/interrupt0.png)

#### 3.2 Maskable Hardware Interrupts

  Any external interrupt that is delivered to the processor by means of the
  INTR pin or through the local APIC is called a maskable hardware interrupt.
  Maskable hardware interrupts that can be delivered through the INTR pin
  include all IA-32 architecture defined interrupt vectors from 0 through
  255. those that can be delivered through the local APIC include interrupt
  vectors 16 though 255.

  The IF flag in the EFLAGS register permits all maskable hardware interrupts
  to be masked as a group. Note that when interrupts 0 through 15 are
  delivered through the local APIC, the APIC indicates the receipt of
  an illegal vector.

#### 3.3 Software-Generated Interrupts

  The INT n instruction permits interrupts to be generated from within 
  software by supplying an interrupt vector number as an operand. For example,
  the INT 35 instruction forces an implicit call to the interrupt handler
  for interrupt 35.

  Any of the interrupt vectors from 0 to 255 can be used as parameter in
  this instruction. If the processor's perdefined vector is used, however,
  the response of the processor will not be the same as it would be from
  an NMI interrupt generated in the normal manner. If vector number 2 (the 
  NMI vector) is used in this instruction, the NMI interrupt handler is 
  called, but the processor's NMI-handle hardware is not activated.

  Interrupt generated in software with the INT n instruction cannot be
  masked by the IF flags in the EFLAGS register.

### 4. Sources of Exception

  The processor receives exception from three sources.

  * Processor-detected program-error exception

  * Software-generated exception

  * Machine-check exception

#### 4.1 Program-Error exception

  The processor generates one or more exception when it detects program
  errors during the excution in a application program or the operating
  system or executive. IA-32 architectures define a vector number for
  each processor-detectable exception. Exception are classified as
  faults, traps, and aborts.

#### 4.2 Software-Generated Exception

  The INTO, INT 3, and BOUND instructions permit exceptions to be geenerated
  in software. There instructions allow checks for exception conditions to
  be performed at points in the instruction stream. For example, INT3 
  causes a breakpoint exception to be generated.

  The INT n instruction can be used to emulate exception in software. But
  there is a limitation. If INT n provides a vector for one of the 
  architecturally-defined exception, the processor generates an interrupt to
  the correct vector (to access the exception handler) but does not push
  an error code on the stack. This is true even if the associated 
  hardware-generated exception normally produces an error code. The exception
  handler will still attempt to pop an error code from the stack while
  handling the exception. Because no error code was pushed, the handler will
  pop off and discard the EIP instead (in place of the missing error code).
  This sends the return the wrong location.

#### 4.3 Machine-Check Exception

  The P6 family and Pentium processors provide both internal and external
  machine-check mechanisms for checking the opeation of the internal hardware
  and bus transactions. These mechanisms are implementation dependent. When
  a machine-check error is detected, the processor signals a machine-check
  exception (Vector 18) and return an error code.

### 5 Exception Classifications

  Exceptions are clasified as faults, traps, or aborts depending on the way 
  they are reported and whether the instruction that caused the exception
  can be restarted without loss of program or task continuity.

  * Fault

    A fault i san exception that can generally be corrected and that, once
    corrected, allows the program to be restarted with no loss of continuity.
    When a fault is reported, the processor restores the machine state
    to state prior to the beginning of execution of the faulting instruction.
    The return address (saved contents of the CS and EIP registers) for the
    fault handler points to the faulting instruction, rather than to the 
    instruction following the faulting instruction.

  * Traps

    A trap is an exception that is reported immediately following the 
    execution. Traps allow execution of a program or task to be continued
    without loss of program continuity. The return address for the trap 
    handler points to the instruction to be executed after the trapping
    instruction.

  * Aborts

    An abort is an exception that does not always report the precise location
    of the instruction causing the exception and does not allow a restart
    of the program or task that caused the exception. Abort are used to 
    report severe errors, suce as hardware errors and inconsistent or illegal
    values in system tables.

  **Note**

    One exception subset normally reported as a fault is not restartable. Such
    exceptions result in loss of some processor state. For example, executing
    a POPAD instruction where the stack frame crosses over the end of the 
    stack segment causes a fault to be reported. In this situation, the 
    exception handler sees that the instruction pointer (CS:EIP) has been
    restored as if the POPAD instruction had not been executed. However,
    internal processor state (the general-purpose registers) will have been
    modified. Such cases are considered programming errors. An application
    causing this class of exceptions should be terminated by the operating 
    system.

### 6. Program or Task restart

  To allow the restarting of program or task following the handling of an
  exception or an interrupt, all exceptions (except abort) are guaranteed
  to report exceptions on an instruction boundary. All interrupts are
  guaranteed to be taken on an instruction boundary.

  For fault-class exceptions, the return instruction pointer (saved when the
  processor generates an exception) points to the faulting instruction. So,
  when a program or task is restarted following the handling of a fault,
  the faulting instruction is restarted (re-executed). Restarting the faulting
  instruction is commonly used to handle exceptions that are generated when
  access to an operand is blocked. The most common example of this type of
  fault is page-fault exception (#PF) that occurs when a program or task 
  references an operand located on a page that is not in memory. When a 
  page-fault exception occurs, the exception handler can load the page into
  memory and resume execution of the program or task by restarting the faulting
  instruction. To insure that the restart is handled transparently to the 
  currently executing program or task, the processor saves the necessary 
  registers and stack pointers to allow a restart to the state prior to the 
  execution of the faulting instruction.

  For trap-class exceptions, the return instruction pointer points to the 
  instruction following the trapping instruction. If a trap is detected during
  an instruction which transfer execution, the return instruction pointer 
  reflects the transfer. For example, if a trap is detected while executing
  a JMP instruction, the return instruction pointer points to the destination
  of the JMP instruction, not to the next address past the JMP instruction.
  All trap exceptions allow program or task restart with no loss of continity.
  For example, the overflow exception is a trap exception. Here, the return
  instruction pointer points to the instruction following the INTO instruction
  that tested EFLAGS OF (overflow) flag. The trap handler for this exception
  resolves the overflow condition. Upon return from the trap handler, 
  program or task execution continues at the instruction following the INTO 
  instruction.

  The abort-class exception do not support reliable restarting of the program
  or task. About handlers are designed to collect diagnostic information
  about the state of the processor when the abort exception occurred and then
  shut down the application and system as gracefully as possible.

  Interrupts rigorously support restarting of interrupted programs and tasks
  without loss of continuity. The return instruction pointer points saved
  for an interrupt points to the next instruction to be executed at the
  instruction boundary where the processor took the interrupt. If the 
  instruction just executed has a repeat prefix, the interrupts is taken
  at the end of the current iteration with the registers set to execute the
  next iteration.

### 7. Nonmaskable Interrupt (NMI)

  The nonmaskable interrupt (NMI) can be generated in either of two ways:

  * External hardware asserts the NMI pin.

  * The processor receives a message on the system bus or the APIC serial
    bus with a delivery mode NMI.

  When the processor receives a NMI from either of these sources, the
  processor handles it immediately by calling the NMI handler pointed to by
  interrupt vector number 2. The processor also invokes certain hardware
  conditions to insure that no other interrupts, including NMI interrupts,
  are received until the NMI handler has completed executing.

  Also, when an NMI is received from either of the above source, it cannot be
  masked by the IF flag in the EFLAGS register.

  It is possible to issue a makeable hardware interrupt (through the INTR pin)
  to vector 2 to invoke the NMI interrupt handler. However, this interrupt
  will not truly be an NMI interrupt. A true NMI interrupt that actives the
  processor's NMI-handling hardware can only be delivered through one of the 
  mechanisms list above.

#### 7.1 Handling Multiple NMIs

  While an NMI interurpt handler is executing, the processor blocks delivery 
  of subsequent NMI until the next execution of the IRET instruction. This 
  blocking of NMI prevents nested of the NMI handler. It is recommended
  that the NMI interrupt handler be accessed through an interrupt gate to 
  disable maskable hardware interrupts.

  An execution of the IRET instruction unblocks NMIs even if the instruction
  causes a fault. For example, if the IRET instruction executes with EFALGS
  VM = 1 and IOPL of less then 3, a general-protection execption is generated.
  In such case, NMIs are unmaked before the exception handler is invoked.

### 8. Enable and Disable Interrupts

  The processor inhibits the generation of some interrupts, depending on the 
  state of the processor and of the IF and RF flags in the EFLAGS register,
  as described in the following section.

#### 8.1 Masking Maskable Hardware Interrupts

  The IF flag can disable the servicing of maskable hardware interrupts
  received on the processor's on the processor's INTR pin or through the local
  APIC. When the IF flag is clear, the processor inhibit interrupts 
  delivered to the INTR pin or through the local APIC from generating an 
  internal interrupt request. When the IF flag is set, interrupt delivered
  to the INTR or through the local APIC pin are processed as normal external
  interrupts.

  The IF flag does not affect non-maskable interrupts (NMIs) delivered to the 
  NMI pin or delivery mode NMI messages delivered through the local APIC, nor 
  does it affect processor generated exceptions. As with the other flags
  in the FFLAGS register, the processor clear the IF flag in response to 
  a hardware reset.

  The fact that the group of maskable hardware interrupts includes the reserved
  interrupt and exception vectors 0 through 32 can potentially cause confusion.
  Architecturally, when the IF flag is set, an interrupt for any of the vector
  from 0 through 32 can be delivered to the proceesor through the INTR pin
  and any of the vectors from 16 through 32 can be delivered through the local
  APIC. The processor will then generate an interrupt and call the interrupt 
  or exception handler pointer to by the vector number. So for example,
  it is possible to invoke the page-fault handler through the INTR pin (by
  means of vector 14). However, this is not a true page-fault exception.
  It is an interrupt. As with the INT n instruction, when an interrupt is
  generated through the INTR pin to an exception vector, the processor does
  not push an error code on the stack, so the exception handler may not operate
  correctly.

  The IF flag can be set or cleared with the STI (set interrupt-enable flags)
  and CLI (clear interrupt-enable flag) instructions, respectively. These
  instructions may be executed only if the CPL is equal to or less than the 
  IOPL. A general-protection exception (#GP) is generated if they are executed
  when the CPL is greater than the IOPL.

  The IF flag is also effected by the following operations:

  * The PUSHF instruction stores all flags on the stack, where they can be
    examined and modified. The POPF instruction can be used to load the 
    modified flags back into the EFLAGS register.

  * Task switches and the POPF and IRET instructions load the EFLAGS register.
    therefore, they can be used to modify the setting of the IF flag.

  * When an interrupt is handled though an interrupt gate, the IF flag is 
    automatically cleared, which disables maskable hardware interrupts.(If
    an interrupt is handled through a trap gate, the IF flag is not cleared.)

#### 8.2 Masking Instruction Breakpoints

  The RF (resume) flag in the EFLAGS register controls the response of the 
  processor to instruction-breakpoint conditions.

  When set, it prevents an instruction breakpoint from generating a debug
  exception (#DB). When clare, instruction breakpoints will generate debug
  exception loop on an instruction-breakpoint.

#### 8.3 Masking Exceptions and Interrupt when Switching Stack

  To switch to a different stack segment, software oftern uses a pair of
  instruction, for example:

  ```
    MOV SS, AX
    MOV ESP, Sack_TOP
  ```

  If an interrupt or exception occurs after the segment selector has been
  loaded into the SS register but before the ESP register has been loaded,
  these two parts of the logical address into the stack space are inconsistent
  for the duration of the interrupt or exception handler.

  To prevent this situation, the processor inhibits interrupts, debug
  exceptions, and single-step trap exceptions after either a `MOV` to `SS`
  instruction or a `POP` to `SS` instruction, until the instruction boundary
  following the next instruction is reached. All other faults may still be
  generated. If the `LSS` instruction is used to modify the contents of the
  `SS` register (which is the recommended method of modifying this register),
  this problem does not occur.

### Priority Among Simultaneous Exception and Interrupts

  If more than one exception or interrupt is pending at an instruction 
  boundary, the processor services them in a predictable order.

  **Table2 Priority Among Simultaneous Exception and Interrupts**

  ![IDT_Prio_1](https://github.com/EmulateSpace/PictureSet/blob/master/interrupt/interrupt_priori_table.png)

  While priority among these classes listed in Table2 is consistent throughout
  the architecture, exception within each class are implementation-dpendent 
  and may vary from processor to processor. The processor first servies a
  pending exception or interrupt from the class which has the highest priority,
  transferring execution to the first instruction of the handler. Low priority
  execptions are discarded. Lower priority interrupt are held pending.
  Discarded exceptions are re-generated when the interrupt handler returns
  execution to the point in the program or task where the exception and/or
  interrupts occurred.

### Interrupt Descriptor Table (IDT)

  The interrupt descriptor table (IDT) associates each exception or interrupt
  vector with a gate descriptor for the procedure or task used to service
  the associated exception or interrupt. Like the GDT and LDTs, the IDT is an
  array of 8-byte descriptors (in protected mode). Unlink the GDT, the first
  entry of the IDT may contain a descriptor. To form an index into the IDT,
  the processor scales the exception or interrup vector by eight (the number
  of bytes in a gate descriptor). Because there are only 256 interrupt or
  exception vectors, the IDT need not contain more than 256 descriptors. It
  can contain fewer then 256 descriptors, because descriptors are required
  only for the interrupt and exception vectors that may occur. All empty
  descriptor slots in the IDT should have the present flag for the descriptor
  set to 0.

  The base address of the IDT should be aligned on an 8-byte boundary to
  maximize performance of cache line fils. The limit value is expressed in 
  bytes and is added to the base address to get the address of the last valid
  byte. A limit value of 0 results in exactly 1 valid byte. Because IDT 
  entries are always eight bytes long, the limit should always be one
  less than an integral multiple of eight (that is, 8N -1).

  The IDT may reside anywhere in the linear address space. As Figure3, the 
  processor locates the IDT using the IDTR register. This register holds
  both a 32-bit base address and 16-bit limit for the IDT.

  The LIDT (load IDT register) and SIDT (store IDT register) instructions
  load and store the contents of the IDTR register, respectively. The 
  LIDT instruction loads the IDTR register with the base address and limit
  held in a memory operand. This instruction can be executed only when the 
  CPL is 0. It normally is used by the initiailzation code of an operating
  system when creating an IDT. An operating system also may use it to change
  from one IDT to another. The SIDT instruction copies the base and limit
  value store in IDTR to memory. This instruction can be executed at any
  privilege level.

  If a vector references a descriptor beyond the limit of the IDT, a 
  general-protection exception (#GP) is generated.

  **Note**

    Because interrupts are delivered to the processor core only once, an 
    incorrectly configured IDT could resoult in incomplete interrupt handling
    and/or the blocking of interrupt delivery.

    IA-32 architecture rules need to be followed for setting up IDTR base/
    limit/access field and each field in the gate desciptors. The same apply
    for the Intel 64 architecture. This includes implicit referencing the 
    destination code segment through the GDT or LDT and accessing the stack.

  **Figure3**

  ![IDT_IDTA_1](https://github.com/EmulateSpace/PictureSet/blob/master/interrupt/interrupt_IDT.png)

### IDT Descriptors

  The IDT may contain ayn of three kinds of gate descriptors:

  * Task-gate descriptor

  * Interrupt-gate descriptor

  * Trap-gate descriptor

  Figure4 show the formats for the task-gate, and trap-gate descriptors. The
  format of a task gate used in an IDT is the same as that of a task gate used
  in the GDT or and LDT. The task gate contains the segment selector for a
  TSS for an exception and/or interrupt handler task.

  Interrupt and trap gates are very similar to call gates. They contain a far
  pointer (segment selector and offset) that the processor uses to transfer 
  program execution to handler procedure in an exception- or interrupt-handler
  code segment. These gates differ in the way the processor handles the IF
  flag in the EFLAGS register.

  **Figure4**

  ![IDT_IDT_SELECTOR](https://github.com/EmulateSpace/PictureSet/blob/master/interrupt/interrup_IDTSE.png)

### 12. Exception and Interrupt Handling

  The processor handles calls to exception- and interrupt-handlers similar to
  the way it handles calls with a CALL instruction to a procedure or a task.
  When responding to an exception or interrupt, the processor uses the 
  exception or interrupt vector as an index to a descriptor in the IDT. If the 
  index points to an interrupt gate or trap gate, the processor calls the
  exception or interrupt handler in a manner similar to `CALL` to a call
  gate. If index points to a task gate, the processor executes a task switch
  to the exception- or interrupt-handler task in a manner similar to a `CALL`
  to a task gate.

#### 12.1 Exception- or Interrupt-Handler Procedures

  An interrupt gate or trap gate references an exception- or interrupt-handler
  procedure that runs in the context of the currently executable code segment
  in either the CDT or the current LDT. The offset field of the gate
  descriptor points to the beginning of the exception- or interrupt-handling
  procedure.

  **Figure3x**

  ![IDT_IDT_HANDLE](https://github.com/EmulateSpace/PictureSet/blob/master/interrupt/interrupt_procedure.png)
  
  When the processor performs a call to the exception- or interrupt-handler 
  procedure:

  * If the handler procedure is going to be executed at a numerically lower
    privilege level, a stack switch occurs. When the stack switch occurs:

    a) The segment selector and stack pointer for the stack to be
       
       used by the handler are obtained from the TSS for the currently

       executing task. On this new stack, the processor pushes the 
   
       stack segment selector and stack pointer of the interrupted

       procedure.

    b) The processor then saves the current state of the EFLAGS, CS,

       and EIP registers on the new stack. (See Figure3x1)

    c) If an exception causes an error code to be saved, it is pushed

       on the new stack after the EIP value.

  * If the handler procedure is going to be executed at the same privilege
    level as the interrupted procedure:

    a) The processor saves the current state of the EFLAGS, CS, and EIP

       registers on the current stack (See Figure3x1)

    b) If an exception causes an error code to be saved, it is pushed

       on the current stack after the EIP value

  **Figure3x1**

  ![IDT_IDT_HANDLE_R](https://github.com/EmulateSpace/PictureSet/blob/master/interrupt/interrupt_handle_R.png)

  To return from an exception- or interrupt-handler procedure, the handbler
  must use the IRET (or IRETD) instruction. The IRET instruction is similar
  to the RET instruction except that is restores the saved flags into the 
  EFLAGS register. The IOPL field of the EFLAGS register is restored only
  if the CPL is 0. The IF flag is changed only if the CPL is less than or 
  equal to the IOPL.

  If a stack switch occurred when calling the handler procedure, the IRET
  instruct switches back to the interrupt procedure's stack on the return.

##### 12.1.1 Protection of Exception- and Interrupt-Handler Procedures

  The privilege-level protection for excepton- and interrupt-hanbler procedures
  is similar to that used for ordinary procedure calls when through a call
  gate. The processor does not permit transfer of execution to an exception-
  or interrupt-handler procedure in a less privileged code segment (numerically
  greater privilege level) than the CPL.

  An attempt to violate this rule results in a general-protection exception (
  #GP). The protection mechanism for exception- and interrupt-handler
  procedures is different in the follow ways:

  * Because interrupt and exception vectors have no RPL, the RPL is not check
    on implicit calls to exception and interrupt handlers.

  * The processor checks the DPL of the interrupt or gate only if an exception
    or interrupt is generated with an INT n, INT 3, or INTO instruction.
    Here, the CPL must be less then or equal to the DPL of the gate. This
    restriction prevents application programs or procedures running at 
    privilege level 3 from using a software interrupt to access critical 
    exception handlers, such as the page-fault handler, providing that those
    handlers are placed in more privileged code segment (numerically lower
    privilege level). For hardware-generated interrupts and processor-detected
    exceptions, the processor ignores the DPL of interrupt and trap gates.

  Because exceptions and interrupts generally do not occur at predictable
  times, these privilege rules effectively impose restrictions on the 
  privilege levels at which exception and interrupt-handler procedures can
  run. Either of the following techniques can be used to avoid privilege-level
  violations.

  * The exception or interrupt handler can be placed in a conforming code
    segment. This technique can be used for handlers that only need to access
    data available on the stack (for example, divide error exceptions). If
    the handler needs data from a data segment, the data segment needs to 
    be accessible from privilege level 3, which would make it unprotected.

  * The handler can be placed in a nonconforming code segment with privilege
    level 0. This handler would always run, regardless of the CPL that the
    interrupted program or task is running at.

##### 12.1.2 Flag Usage By Exception- or Interrupt-handler Procedure

  When accessing an excepiton or interrupt handler through eigther and 
  interrupt gate or trap, the processor clear the TF flag in the EFLAGS
  register after it saves the contents of the FFLAGS register on the stack.(
  On calls to exception and interrupt handlers, the processor also clear
  the VM, RF, and NT flags in the EFLAGS register, after they are saved on
  the stack.) Clearning the TF flag prevents instruction tracing from affecting
  interrupt reponse. A subsequent IRET instruction restores the TF (and VM, RF,
  and NT) flag to the values in the saved contents of the EFLAGS register
  on the stack.

  The only different between an interrupt gate and a trap gate is the way the 
  processor handler the IF flag in the EFLAGS register. When accessing an 
  exception- or interrupt-handling procedure through an interrupt gate, the
  processor clears the IF flag to prevent other interrupts from interfering
  with the current interrupt handler. A subsequent IRET isntruction restores
  the IF flag to its value in the saved contents of the EFLAGS register on
  the stack. Accessing a handler procedure through a trap gate does not affect
  the IF flag.

#### 12.2 Interrupt Tasks

  When an exception or interrupt handler is accessed through a task gate in
  the IDT, as task switch results. Handing an exception or interrupt with
  a separate task offers several advantages:

  * The entire context of the interrupted program or task is saved 
    automatically.

  * A new TSS permits the handler to use a new privilege level 0 stack when
    handling the exception or interrupt. If an exception or interrupt 
    occurs when the current privilege level 0 stack is corrupted, accessing
    the handler through a task gate can prevent a system crash by providing
    the handler with a new privilege level 0 stack.

  * The handler can be further isolated from other task's by giving it a
    separate address space. This is done by giving it a separate LDT.

  The disadvantage of handling an interrupt with a separate task is that
  the amount of machine state that must be saved on a task switch makes
  it slower that unsing an interrupt gate, resulting in increased interrupt
  latency.

  A task gate in the IDT references a TSS descriptor in the GDT. A switch to 
  the handler task is handled in the same manner as an ordinary task switch.
  The link back to the interrupted task is stored in the previous task link
  feild of the handler task's TSS. If an exception caused an error code to be
  generated, this error code is copied to the stack of the new task.

  When exception- or interrupt-hander tasks are used in an operation system,
  there are actually two machanisms that can be used to dispatch tasks: the 
  software scheduler (part of the operation system) and the hardware scheduler(
  part of the processor's interrupt machanism. The software scheduler needs
  to accommodate interrupt tasks that may be dispatched when interrupts
  are enable.

  **NOTE**

    Because IA-32 architecture tasks are not re-entrant, an interrupt-handler
    task must disable interrupt between the time it completes handling the 
    interrupt and the time it executes the IRET instruction. This action
    prevents another interrupt from occurring while the interrupt task's TSS
    is still marked busy, which would cause a general-protection (#GP)
    exception.

  ![IDT_IDT_TASK](https://github.com/EmulateSpace/PictureSet/blob/master/interrupt/interrupt_task.png)

#### 13. ERROR CODE

  When an exception condition is related to a specific segment selector or IDT
  vector, the processor pushs an error code onto the stack of the exception 
  handler (whether it is a precedure or task). The error code has the format
  `Figure6`. The error code resembles a segment selector. However, instead of 
  a TI flag and RPL field, the error code contains 3 flags:

  * EXT

    External event (bit 0) - When set, indicates that the exception occurred
    during delivery of an event external to the program, such as an interrupt
    or earlier exception.

  * IDT

    Descriptor location (bit 1) - When set, indicates that the index portion
    of the error code refers to a gate descriptor in the IDT. When clear,
    indicates that the index refers to a descriptor in the GDT or the current
    LDT.

  * TI

    GDT/LDT (bit 2) - Only used when the IDT flag is clear. When set, the TI
    flag indicates that the index portion of the error code refers to a 
    segment or gate descriptor in the LDT. When clear, it indicates that
    the index refers to a descriptor in the current GDT.

    ```

    31_____________________________________3___2___1___0
    |             |                        | T | I | E |
    |  Reserved   | Segment selector index | I | D | X |
    |_____________|________________________|___|_T_|_T_|

    ```

  The Segment selector index field provides an index into the IDT, GDT, or
  current LDT to the segment or gate selector being referenced by the error
  code. In some cases the error code is null (all bits are clear execpt 
  possibly EXT). A null error code indicates that the error code was not caused
  by a reference to a specific segment or that a null segment selector was
  referenced in an operation.

  The format of the error code is different for page-fault exception (#PF).
  
  The error code is pushed on the stack as a doubleword or word (depending on
  the default interrupt, trap, or task gate size). To keep the stack aligned
  for doubleword pushes, the upper half of the error code is reserved. Note
  that the error code is not popped when the IRET instruction is executed to
  return from an exception handler, so the handler must remove the error code
  before execution a return.

  Error codes are not pushed on the stack for exceptions that are generated 
  externally (with the INTR or LINT[1:0] pins) or the INT n instruction, 
  even if an error code is normally produced for those exception.

### 14. Exception and Interrupt Reference
 
  The following section describe condition which generate exception and 
  interrupts. They are arraned in the order of vector number.

  * interrupt 0 - Divide Error Exception (#DE)

    see idt_0.c
  

