Segment
--------------------------------------------

The segmentation mechanism supported by the IA-32 architecture can be used
to implement a wide variety of system designs. These designs range from flat
models that make only minimal use of segmentation to protect programs to 
multi-segmented models that employ segmentation to create a robust operating
environment in which multiple programs and tasks can be executed reliably.

# Memory models

### Basic Flat Model

The simplest memory model for a system is the basic **flat model**, in which
the operating system and application programs have access to a continuous,
unsegmented address space. To the gratest extent possible, this basic flat
model hides the segmentation mechanism of the architecture from both the 
system designer and the application programmer.

To implement a basic flat memory model with the IA-32 architecture, at least
two segment descriptors must be created, one for referencing a code segment
and one for referencing a data segment. As Figure. Both of these segments,
however, are mapped to the entire linear address space: that is, both segment
descriptors have the same base address value of 0 and the same segment limit
of 4GBytes, By setting the segment limit to 4 GByte, the segmentation 
mechanism is kept from generating exceptions for out of limit memory 
references, even if no physical memory resides at a particular address. ROM (
EEPROM) is generally located at the top of the physical address space, 
because the processor begins execution at FFFF_FFF0H. RAM (DRAM) is placed
at the bottom of the address space because the initial base address for the 
DS data segment after reset initialization is 0.

```

 Segment Register                                      Linear Address Space
                                                       (or Physical Address)   
 +----------------+                               o---->+----------------+ Max
 |      CS       -|---o                           |     |      Code      |
 +----------------+   |                           |     +----------------+
                      |                           |     |                |
 +----------------+   |                           |     |                |
 |      SS       -|---o    Code- and Data-        |     |  Not Present   |
 +----------------+   |    Segment descriptor     |     |                |
                      |                           |     |                |
 +----------------+   |    +--------+-------+     |     +----------------+
 |      DS       -|---o    | Access | Limit |-----o     |                |
 +----------------+   |    +--------+-------+           | Data and Stack |
                      o--->|  Base Address  |           |                |
 +----------------+   |    +----------------+---------->+----------------+ 0
 |      ES       -|---o
 +----------------+   |                                 (Max: 0xFFFFFFFF)
                      |
 +----------------+   |
 |      FS       -|---o
 +----------------+   |
                      |
 +----------------+   |
 |      GS       -|---o
 +----------------+

```

### Protected Flat Model

The protected flat model is similar to the basic flat model, except the segment
limits are set to include only the range of address for which physical memory
actually exists (see Figure). A general-protection exception (#GP) is then
generated on any attempt to access nonexistent memory. This model a minimum
level of hardware protection against some kinds of program bugs.

```

                                                        Linear address space
 Segment Register          Segment Descriptor           (or Physical Memory)
 +----------------+        +--------+-------+     o---->+----------------+ Max
 |      GS       -|------->| Access | Limit |-----o     |      Code      |
 +----------------+        +--------+-------+           |                |
                           |  Base Address  |---------->+----------------+
 +----------------+        +----------------+           |                |
 |      GS       -|---o                                 |                |
 +----------------+   |                                 |                |
                      |                                 |                |
 +----------------+   |                                 |                |
 |      GS       -|---o                                 |                |
 +----------------+   |    Segment Descriptor     o---->+----------------+
                      |    +--------+-------+     |     |   Memory I/O   |
 +----------------+   |    | Access | Limit |-----o     +----------------+
 |      GS       -|---o--->+--------+-------+           |                |
 +----------------+   |    |  Base Address  |-----o     |                |
                      |    +----------------+     |     |                |
 +----------------+   |                           |     |                |
 |      GS       -|---o                           |     | Data and Stack |
 +----------------+   |                           |     |                |
                      |                           |     |                |
 +----------------+   |                           |     |                |
 |      GS       -|---o                           |     |                |
 +----------------+                               o---->+----------------+ 0

                                                        (Max: 0xFFFFFFFF)
```

More complexity can be added to this protected flat model to provide more
protection. For exception, for the paging mechanism to provide isolation 
between user and supervisor code and data, four segments need to be defined:
code and data segments at privilege level 3 for the user, and code and data
segments at privilege level 0 for the supervisor. Usually these segments all
overlay each other and start at address 0 in the linear address space. This
flat segmentation model along with a simple paging structure can protect the 
operating system from applications, and by adding a separate paging structure
structure for each task or process, it can also protect applications from
each other. 

### Multi-Segment Model

A multi-segment model (such as the Figure) uses the full capabilities of
the segmentation mechanism anism to provide hardware enforced protection
of code, data structures, and programs and tasks. Here, each program (or 
task) is given its own table of segment descriptors and its own segments.
The segments can be completely private to their assigned programs or shared
among programs. Access to all segments and to the execution environments
of individual programs running on the system is controlled by hardware.

![Multi-segment](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel/MMU000394.png)

Access checks can be used to protect not only against referencing an address
outside the limit of a segment, but also against performing disallowed 
operations in certain segments. For example, since code segments are designated
as read-only segments, hardware can be used to prevent writes into code 
segments. The access rights information created for segments can also be used
to set up protection rings or levels. Protection levels can be used to 
protect operating-system procedures from unauthorized access by application
programs.

# Address space

### Logical and linear addresses

At the system-architecture level in protected mode, the processor uses two
stages of address translation to arrive at a physical address: logical-address
translation and linear address space paging.

Even with the minimum use of segments, every byte in the processor's address
space is accessed with a logical address. A **logical address** consists of
a 16-bit segment selector and a 32-bit offset (As Figure). The segment 
selector identifies the segment the byte is located in and the offset specifies
the location of the byte in the segment relative to the base address of the
segment.

```
Logical-Address:

 15                            31                                  0
 +--------------+----+-----+   +-----------------------------------+
 |    Index     | TI | RPL | : |             Offset                |
 +--------------+----+-----+   +-----------------------------------+
```

The processor translates every logical address into a linear address. A linear
address is a 32-bit address in the processor's linear address space. Like
the physical address space, the linear address is a flat (unsegmented), 
2^32-byte address space, with addresses ranging from 0 to 0xFFFFFFFFH. The
linear address space contains all the segments and system tables defined
for a system.

To translate a logical address into a linear address, the processor does the
following:

1. Uses the offset in the segment selector to locate the segment descriptor 
   for the segment in the GDT or LDT and reads it into the processor. (This
   step is needed only when a new segment selector is loaded into a segment
   register.)

2. Examines the segment descriptor to check the access rights and range of
   the segment to insure that the segment is accessible and that the offset
   is within the limits of the segment.

3. Adds the base address of the segment from the segment descriptor to the 
   offset to form a linear address.

```
  Logical address

  15                      0   31                           0
  +-----------------------+   +----------------------------+
  |    Segment Selector   | : |           Offset           |
  +-----------------------+   +----------------------------+
    |                                        |
    |                                        |
    |                                        |
    |     Descriptor Table                   |
    |    +----------------+                  |
    |    |                |                  |
    |    |                |                  |
    |    |                |                  |
    |    |                |                  |
    |    |                |                  |
    |    |                |                  |
    |    |                |                  |
    |    +----------------+                  V
    |    |    Segment     |                +---+
    o--->|   Descriptor  -|--------------->| + |
         +----------------+                +---+
         |                |                  |
         +----------------+                  |
                                             |
                                             |
                                             |
                              32             V             0
                              +----------------------------+
                              |       Linear Address       |
                              +----------------------------+
```

# Register

### Segment Selector

A segment selector is a 16-bit identifier for a segment (See Figure). It does
not point directly to the segment, but instead points to the segment 
descriptor that defines the segment. A segment selector contains the following
items:

```
       15                                        3    2     0
       +-----------------------------------------+----+-----+
       |               Index                     | TI | RPL |
       +-----------------------------------------+----+-----+
                                                    A    A
                                                    |    |
       Table Indicator -----------------------------o    |
         0 = GDT                                         |
         1 = LDT                                         |
       Request Privilege Level (RPL) --------------------o
```

**index**  (Bit 3 through 15) -- Selects one of 8192 descriptor in the GDT or
           LDT. The processor multiplies the index value by 8 (the number of
           bytes in a segment descriptor) and adds the result to the base
           address of the GDT or LDT (from the GDTR or LDTR register).

**TI (table indicator) flag**
           (Bit 2) -- Specifies the descriptor table to use: clearing this
           flag selects the GDT; setting this flag selects the current LDT.

**Requested Privilege Level (RPL)**
           (Bits 0 and 1) -- Specifies the privilege level of the selector.
           The privilege level can range from 0 to 3, with 0 being the most
           privileged level.

The first entry of the GDT is not used by the processor. A segment selector
that points to this entry of the GDT (that is, a semgnet selector with an 
index of 0 and TI flag set to 0) is used as a **null segment selector**. The
processor does not generate an exception when a segment register (other than
the CS or SS registers) is loaded with a null selector. It does, however,
generate an exception when a segment register holding a null selector is used
to access memory. A null selector can be used to initialize unused segment
registers. Loading the CS or SS register with a null segment selector causes
a general-protection exception (#GP) to be generated.

Segment selectors are visible to application programs as part of a pointer
variable, but the values of selectors are usually assigned or modified by
link editors or links loaders, not application programs.

### Segment Register

To reduce address translation time and coding complexity, the processor 
provides register for holding up to 6 segment selectors (see Figure). Each
of these segment register support a specific kind of memory reference (code,
stack, or data). For virtually any kind of program execution to take place,
at least the code-segment (CS), data-segment (DS), and stack-segment (SS)
regsiter must be loaded with valid segment selectors. The processor also 
provides three additional data-segment registers (ES, FS, and GS), which can
be used to make additional data segments available to the currently executing
program (or task).

For a program to access a segment, the segment selector for the segment must
have been loaded in one of the segment registers. So, although a system can
define thousands of segments, only 6 can be available for immediate use.
Other segments can be made avaiable by loading their segment selectors during
program execution.

```
      Visible Part                    Hidden Part
  +--------------------+-----------------------------------------+
  |  Segment Selector  | Base Address, Limit, Access information | CS
  +--------------------+-----------------------------------------+
  |                    |                                         | SS
  +--------------------+-----------------------------------------+
  |                    |                                         | DS
  +--------------------+-----------------------------------------+
  |                    |                                         | ES
  +--------------------+-----------------------------------------+
  |                    |                                         | FS
  +--------------------+-----------------------------------------+
  |                    |                                         | GS
  +--------------------+-----------------------------------------+
```

Every segment register has a **visible** part and a **hidden** part. (The
hidden part is sometimes referred to as a "descriptor cache" or "shadow
register".) When a segment selector is loaded into the visible part of a 
segment register, the processor also loads the hidden part of the segment
register with the base address, segment limit, and access control information
from the segment descriptor pointed to by the segment selector. The information
cached in the segment register (visible and hidden) allows the processor to
translate addresses without taking extra bus cycles to read the base address
and limit from the segment descriptor. In systems in which multiple processors
have access to the same descriptor tables, it is the responsiblility of 
software to reload the segment register when the descriptor tables are 
modified. If this is not done, an old segment descriptor cached in segment
register might be used after its memory-resident version has been modified.

Two kinds of load instructions are provided for loading the segment registers:

1. Direct load instructions such as the MOV, POP, LDS, LES, LSS, LGS, and
   LFS instructions. These instructions explicitly reference the segment
   registers.

2. Implied load instructions such as the far pointer version of the CALL,
   JMP, and RET instruction, the SYSENTER and SYSEXIT instruction, and the
   IRET, INTn, INTO and INT3 instructions. These instructions change the 
   contents of the CS register (and sometimes other segment registers) as
   an incidental part of their operation.

The MOV instruction can also be used to store the visible part of a segment
register in a general-purpose register.

### Global Descriptor Table Register (GDTR)

The GDTR regsiter holds the base address (32 bits in protected mode) and the
16-bit table limit for the GDT. The base address specifies the linear address
of byte 0 of the GDT; the table limit specifies the number of bytes in the 
table.

```
GDTR

     47                             16 15                      0
     +--------------------------------+------------------------+
     |   32-bit linear Base address   |   16-bit Table limit   |
     +--------------------------------+------------------------+

```

The LGDT and SGDT instruction load and store the GDTR register, respectively.
On power up or reset of the processor, the base address is set to the default
value of 0 and the limit is set to 0xFFFFH. A new base address must be loaded
into the GDTR as part of the processor initialization process for protected-
mode operation.

### Local Descriptor Table Register (LDTR)

The LDTR register holds the 16-bit segment selector, base address (32 bits 
in protected mode), segment limit, and descriptor attributes for the LDT.
The base address specifies the linear address of byte 0 of the LDT segment;
the segment limit specifies the number of bytes in the segment.

```
LDTR

 15            0  
 +-------------+  +----------------------------+---------------+-----------+
 |   Seg.Sel   |  | 32-bit linear base address | Segment limit | Attribute |
 +-------------+  +----------------------------+---------------+-----------+
```

The LLDT and SLDT instructions load and store the segment selector part of
the LDTR register, respectively. The segment that contains the LDT must have
a segment descriptor in the GDT. When the LLDT instruction loads a segment
selector in the LDTR: the base address, limit, and descriptor attribute from
the LDT descriptor are automatically loaded in the LDTR.

When a task switch occurs, the LDTR is automatically loaded with the segment
selector and descriptor for the LDT for the new task. The contents of the 
LDTR are not automatically saved prior to writing the new LDT information 
into the register.

On power up or reset of the processor, the segment selector and base address
are set to the default value of 0 and the limit is set to 0x0FFFFH.

### IDTR Interrupt Descriptor Table Register

The IDTR register holds the base address (32 bits in protected mode) and 
16-bit table limit for the IDT. The base address specifies the linear address
of byte 0 of the IDT; the table limit specifies the number of bytes in the
table. The LIDT and SIDT instructions load and store the IDTR register, 
reppectively. On power up or reset of the processor, the base address is set
to the default value of 0 and the limit is set to 0x0FFFFH. The base address
and limit in the register can then be changed as part of the processor
initialization process.

```
IDTR

     47                             16 15                      0
     +--------------------------------+------------------------+
     |   32-bit linear Base address   |   16-bit Table limit   |
     +--------------------------------+------------------------+

```

### Task Register (TR)

The task register holds the 16-bit segment selector, base address (32 bits in
protected mode), segment limit, and descriptor attribute for the TSS of the
current task. The selector references the TSS descriptor in the GDT. The base
address specifies the linear address of byte 0 of the TSS; the segment limit
specifies the number of bytes in the TSS.

```
 Task Register

 15            0
 +-------------+  +----------------------------+---------------+-----------+
 |   Seg.Sel   |  | 32-bit linear base address | Segment limit | Attribute |
 +-------------+  +----------------------------+---------------+-----------+
```

The LTR and STR instructions load and store the segment selector part of the 
task register, respectively. When the LTR instruction loads a segment selector
in the task register, the base address, limit, and descriptor attribute from
the TSS descriptor are automatically loaded into the task register. On power
up or reset of the processor, the base address is set to the default value of
0 and the limit is set to 0xFFFFH.

When a task switch occurs, the task register is automatically loaded with the
segment selector and descriptor for the TSS for the new task. The contents
of the task register are not automatically saved prior to writing the new
TSS information into the register.

# Segment Descriptor

A segment descriptor is a data struction in a GDT or LDT that provides the
processor with the size and location of a segment, as well as access control
and status information. Segment descriptors are typically created by 
compilers, linkers, loaders, or the operating system or executive, but not 
application programs. Figure illustrates the general descriptor format for
all types of segment descriptors.

![Segment Descriptor](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel/MMU000395.png)

The flags and fields in a segment descriptor are as follows:

### Segment limit field

Specifies the size of the segment. The processor puts together the two 
segment limit fields to form a 20-bit value. The processor interprets the 
segment limit in one of two ways, depending on the setting of the **G** (
granularity) flag:

* If the granularity flag is clear, the segment size can range from 1 byte to 
  1 Mbyte, in byte increments.

* If the granularity flag is set, the segment size can range from 4KBytes
  to 4GBytes, in 4-KByte increments.

The processor uses the segment limit in two different ways, depending on
whether the segment is an expand-up or an expand-down segment. For expand-up 
segments, the offset in a logical address can range from 0 to the segment 
limit. Offsets greater than the segment limit generate general-protection 
exceptions (#GP, for all segments other than SS) or stack-fault exception (#SS 
for the SS segment). For expand-down segments, the segment limit has the 
reverse function; the offset can range from the segment limit plus 1 to 
FFFFFFFFH or FFFFH, depending on the setting of the B flag. Offsets less than 
or equal to the segment limit generate general-protection exceptions or 
stack-fault exceptions. Decreasing the value in the segment limit field for 
an expand-down segment allocates new memory at the bottom of the segment's 
address space, rather than at the top. IA-32 architecture stacks always grow 
downwards, making this mechanism convenient for expandable stacks.

### Base address field

Defines the location of byte 0 of the segment within the 4-GByte linear 
address space. The processor puts together the three base address fields
to from a single 32-bit value. Segment base addresses should be aligned to
16-byte boundaries. Although 16-byte alignment is not required, this 
alignment allows programs to maximize performance by aligning code and data
on 16-byte boundaries.
