TSS (Task-State Segment)
-----------------------------------------------------------

The processor state information needed to restore a task is saved in a system
segment called the task-state segment (TSS). Figure show the format of a TSS
for tasks designed for 32-bit CPUs. The field of a TSS are divided into two
main categories: dynamic fields and statis fields.

```
Task-State Segment (TSS)

31                                   15                                   0
+------------------------------------+------------------------------------+
|        I/O Map Base Address        |            Reserved            | T |
+------------------------------------+------------------------------------+
|              Reserved              |        LDT Segment Selector        |
+------------------------------------+------------------------------------+
|              Reserved              |                 GS                 |
+------------------------------------+------------------------------------+
|              Reserved              |                 FS                 |
+------------------------------------+------------------------------------+
|              Reserved              |                 DS                 |
+------------------------------------+------------------------------------+
|              Reserved              |                 SS                 |
+------------------------------------+------------------------------------+
|              Reserved              |                 CS                 |
+------------------------------------+------------------------------------+
|              Reserved              |                 ES                 |
+------------------------------------+------------------------------------+
|                                   EDI                                   |
+-------------------------------------------------------------------------+
|                                   ESI                                   |
+-------------------------------------------------------------------------+
|                                   EBP                                   |
+-------------------------------------------------------------------------+
|                                   ESP                                   |
+-------------------------------------------------------------------------+
|                                   EBX                                   |
+-------------------------------------------------------------------------+
|                                   EDX                                   |
+-------------------------------------------------------------------------+
|                                   ECX                                   |
+-------------------------------------------------------------------------+
|                                   EAX                                   |
+-------------------------------------------------------------------------+
|                                  EFLAGS                                 |
+-------------------------------------------------------------------------+
|                                   EIP                                   |
+-------------------------------------------------------------------------+
|                                CR3 (PDBR)                               |
+------------------------------------+------------------------------------+
|              Reserved              |                 SS2                |
+------------------------------------+------------------------------------+
|                                  ESP2                                   |
+------------------------------------+------------------------------------+
|              Reserved              |                 SS1                |
+------------------------------------+------------------------------------+
|                                  ESP1                                   |
+------------------------------------+------------------------------------+
|              Reserved              |                 SS0                |
+------------------------------------+------------------------------------+
|                                  ESP0                                   |
+------------------------------------+------------------------------------+
|              Reserved              |        Previous Task Linux         |
+------------------------------------+------------------------------------+
```

The processor updates dynamic fields when a task is suspended during a task
switch. The following are dynamic fields.

* **General-purpose register field** -- State of the EAX, ECX, EDX, EBX, ESP,
  EBP, ESI, and EDI register prior to the task switch.

* **Segment selector field** -- Segment selectors stored in the ES, CS, SS, DS,
  FS, and GS registers prior to the task switch.

* **EFLAGS register field** -- State of the EFLAGS register prior to the task
  switch.

* **EIP (instruction pointer) field** -- State of the EIP register prior to the
  task switch.

* **Previos task link field** -- Contains the segment selector for the TSS of 
  the previous task (updated on a task switch that initiated by a call, 
  interrupt, or exception). This field (which is sometimes called the back link
  field) permits a task switch back to the previous task by using the IRET 
  instrcution.

The processor reads the static fields, but does not normally change them. 
These fields are set up when a stask is created. The following are static 
fields:

* **LDT segment selector field** -- Contains the segment selector for the 
  task's LDT.

* **CR3 control register field** -- Contains the base physical address of the
  page directory to be used by the task. Control register CR3 is also known as
  the page-directory base register (PDBR).

* **Privilege level-0, -1, and -2 stack pointer fields** -- These stack
  pointers consist of a logical address made up of the segment selector for the
  stack segment (SS0, SS1, and SS2) and an offset into the stack (ESP0, ESP1,
  and ESP2). Note that the values in these fields are static for a particular
  task; whereas, the SS and ESP values will change if stack switching occurs
  within the task.

* **T (debug trap) flag (byte 100, bit 0)** -- When set, the T flag causes the 
  processor to raise a debug exception when a task switch to this task occurs.

* **I/O map base address field** -- Contain a 16-bit offset from the base of
  the TSS to the I/O permission bit map and interrupt redirection bitmap. When
  present, these maps are stored in the TSS at higher addresses. The I/O map
  base address points to the beginning of the I/O permission bit map and the 
  end of the interrupt redirection bit map.

If paging is used:

* Avoid placing a page boundary in the part of the TSS that the processor reads
  during a task switch (the first 104 bytes). The processor may not correctly
  perform address translations if a boundary occurs in this area. During a task
  switch, the processor reads and writes into the first 104 bytes of each TSS (
  using contiguous physical address beginning with the physical address of the
  first byte of the TSS). So, after TSS access begins, if part of the 104 bytes
  is not physically contiguous, the processor will access incorrect information
  without generating a page-fault exception.

* Pages corresponding to the previous task's TSS, the current task's TSS, and
  the descriptor table entries for each all should be marked as read/write.

* Task switches are carried out faster if the pages containing these structures
  are present in memory before the task switch is initialized.

## TSS Descriptor

The TSS, like all other segments, is defined by a segment descriptor. Figure
show the format of a TSS descriptor. TSS descriptors may only be placed in the
GDT; they cannot be placed in an LDT or the IDT.

An attempt to access a TSS using a segment selector with its TI flag set (which
indicates the current LDT) causes a general-protection exception (#GP) to be
generated during CALLs and JMPs; it causes an incalid TSS exception (#TS) 
during IRETs. A general-protection exception is also generated of an attempt is
made to load a segment selector for a TSS into a segment register.

![TSS Descriptor](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel/MMU000411.png)

The busy flag (B) in the type field indicates whether the task is busy. A busy
task is currently running or suspended. A type field with a value of 1001B 
indicates an inactive task; a value of 1011B indicates a busy task. Tasks are
not recursive. The processor uses the busy flag to detect an attempt to call
a task whose execution has been interrupted. To insure that there is only one
busy flag is associated with a task, each TSS should have only one TSS 
descriptor that points to it.

The base, limit, and DPL field and the granularity and present flags have
functions similar to their use in data-segment descriptor. When the G flag is
0 in a TSS descriptor for a 32-bit TSS, the limit field must have a value equal
to or greater than 67H, one byte less than the minimum size of a TSS.
Attempting to switch to a task whose TSS descriptor has a limit less than 67H
generates an invalid-TSS exception (#TS). A larger limit is required if an I/O
permission bit map is induded or if the operating system stores additional data.
The processor does not check for a limit generater than 67H on a task switch;
howerver, it does check when accessing the I/O permission bit map or interrupt
redirection bit map.

Any program or procedure with access to a TSS descriptor (that is, whose CPL is
numerically equal to or less than the DPL of the TSS descriptor) can dispatch
the task with a call or a jump.

In most systems, the DPLs of TSS descriptor are sett to values less then 3, so 
that only privileged software can perform task switching. However, in
multitasking applications, DPLs for some TSS descriptors may be set to 3 to
allow task switching at the application (or user) privilege level.

## Task Register

The task register holds the 16-bit segment selector and the entire segment
descriptor (32-bit base address, 16-bit segment limit, and descriptor 
attributes) for the TSS of the current task (See Figure). This information is
copied from the TSS descriptor in the GDT for the current task. See Figure 2
shows the path the processor uses to access the TSS (using the information in
the task register).

```

 Task Register

 15            0
 +-------------+  +----------------------------+---------------+-----------+
 |   Seg.Sel   |  | 32-bit linear base address | Segment limit | Attribute |
 +-------------+  +----------------------------+---------------+-----------+

```
The task register has a visible part (that can be read and changed by software)
and an invisible part (maintained by the processor and is inaccessible by
software). The segment selector in the visible portion points to a TSS
descriptor in the GDT. The processor uses the invisible portion of the task
register to cache the segment descriptor for the TSS. Caching these valuse
in a register maskes execution of the task more efficient. The **LTR** (load
task register) and **STR** (store task register) instructions load and read the
visible portion of the task register:

The LTR instruction loads a segment selector (source operand) into the task 
register that points to a TSS descriptor in the GDT. It then loads the 
invisible portion of the task register with information from the TSS descriptor.LTR is a privilege instruction that may be executed only when the CPL is 0. 
It's used during system initialization to put an initial value in the task
register. Afterwards, the contents of the task register are changed implicitly 
when a task switch occurs.

The STR (store task register) instruction stores the visible portion of the
task register in a general-purpose register or memory. This instruction can be 
executed by code running at any privilege level in order to identify the
currently running task. However, it is normally used only by operating sysytem
softare. (If CR4.UMIP = 1, STR can be executed only when CPL = 0).

On power up or reset of the processor, segment selector and base address are
set to default value of 0; the limit is set to FFFFH.

![Task Register](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel/MMU000412.png)

## Task-Gate Descriptor

A task-gate descriptor provides an indirect, protected reference to a task (See
Figure). It can be placed in the GDT, and LDT, or the IDT. The TSS segment 
selector field in a task-gate descriptor points to a TSS descriptor in the GDT.
The RPL in this segment selector is not used.

The DPL of a task-gate descriptor controls access to the TSS descriptor during
a task switch. When a program or procedure makes a call or jump to a task 
through a task gate, the CPL and the RPL field of the gate selector pointing
to the task gate must be less than or equal to the DPL of the task-gate
descritpor. Note that when a task gate is used, the DPL of the destination TSS
descriptor is not used.

![Task-Gate Descriptor](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel/MMU000413.png)
