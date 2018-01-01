Segment Descriptor Tables (GDT or LDT)
--------------------------------------

  A segment descriptor table is an array of segment descriptors (See Figure).
  A descriptor table is variable in length and can contain up to 8192(2^13)
  8 byte descriptors. There are two kinds of descriptor tables:

  * The global descriptor table (GDT)

  * The local descriptor table (LDT)

  Each system must have one GDT defined, which may be used for all programs
  and tasks in the system. Optionally, one or more LDTs can be defined.
  For example, an LDT can be defined for each separate task being run, or
  some or all tasks can share the same LDT.

  ![Alt text](https://github.com/EmulateSpace/PictureSet/blob/master/gdt/GDT00.png)

  The `GDT` is not a segment itself. instead, it is a data struct in linear
  address space. The base linear address and limit of the GDT must be loaded
  into `GDTR` register. The base address of the GDT should be aligned on 
  eigth-byte boundary to yield the best processor performance. The limit
  value for the GDT is expressed in bytes. As with segments, the limit
  value is added to the base address to get the address of the last valid
  byte. A limit value of 0 results in exactly one valid byte. Because
  segment descriptors are always 8 bytes long, the GDT limit should always
  be on less than an integral multiple of eight (that is, 8N - 1).

  The first descriptor in the GDT is not used by the processor. A segment
  selector to this "null descriptor" does not generate an exception when
  loaded into a data-segment register (DS, ES, FS or GS), but it always
  generates a general-protection exception (#GP) when an attempt is made
  to access memory using the descriptor. By initializing the segment
  registers with this segment selector, accidental reference to unused 
  segment registers can be guaranteed to generate an exception.

  The LDT is located in a system segment of the LDT type. The GDT must
  contain a segment descriptor for the LDT segment. If the system
  supports multiple LDTs, each must have a separate segment selector and 
  segment descriptor in the GDT. The segment descriptor for an LDT can
  be located anywhere in the GDT.

  An LDT is accessed with its segment selector. To eliminate address
  translations when accessing the LDT, the segment selector, base linear
  address, limit, and access right of the LDT are stored in LDTR register.

  When the GDTR register is stored (using the SGDT instruction), a 48-bit
  "pseudo-descriptor" is stored in memory. To avoid alignment check faults
  in user mode (privilege level 3), the pseudo-descriptor should be
  located at an odd word address (that is, address MOD 4 is equal to 2).
  This causes the processor to store an aligned word, followed by an
  aligned doubleword. User-mode programs normally do not store pseudo-
  descriptors, but the possibility of generating an alignment an
  alignment check fault can be avoided by aligning pseudo-descriptors in
  this way. The same alignment should be used when storing the IDTR
  register using the SIDT instruction. When storing the LDTR or task
  register (using the SLDT or STR instruction, respectively), the pseudo-
  descriptor should be located at a doubleword address (that is addreess
  MOD 4 is equal to 0).

  ```
    47------------------------------16-15-------------------------0
    |  32-bit Base Address            |    Limit                  |
    ---------------------------------------------------------------
  ```

  When operating in protected mode, all memory accesses pass through
  either gloabl descriptor table (GDT) or and optional local 
  descriptor table (LDT) as shown in Figure. There tables contain
  entries called segment descriptors. Segment descriptors provide
  the base address of segments well as access right, type, and usage
  information.

  ![Alt text](https://github.com/EmulateSpace/PictureSet/blob/master/gdt/IA32_system-level_Registers.png)

  Each segment descriptor has an associated segment selector. A segment
  selector provides the software that uses it with an index into the 
  GDT or LDT (the offset of its associated segment descriptor), a
  global/local flag (determines whether the selector points to the
  GDT or the LDT), and access rights information. and access rights 
  information.

  To access a byte in a segment, a segment selector and an offset must
  be supplied. The segment selector provides access to the segment
  descriptor for the segment (in the GDT or LDT). From the segment
  descriptor, the processor obtains the base address of the segment
  in the linear address space. The offset then provide the location
  of the byte relative to base address. This mechanism can be used
  to access any valid code, data, or stack segment, provided the segment
  is accessible from the current privilege (CPL) at which the processor
  is operating. The CPL is defined as the protection level of the 
  currently executing code segment.

  Above Figure. The solid arrows in the figure indicate a linar 
  address, dashed lines indicated a segment selector, and the dotted
  arrows indicate a physical address. For simplicity, many of the 
  segment selector are shown as direct pointers to a segment. However,
  the actual path from a segment selector to its associated segment
  is always through a GDT or LDT.

  The linear address of the base of the GDT is contained in the GDT
  register (GDTR). the linear address of the LDT is contained in the 
  LDT register(LDTR).

### Debug on BiscuitOS

  BiscuitOS support online debug GDT/LDT on `qemu`, developer should open
  Kernel-macro when configure kernel. Shortly, follow these step to
  enable Stack on system.

  1. Enale specifical Kernel-macro

     Invoke `make menuconfig` on top of source tree, and enable
     specifical item as follow figures.

     ```
       make menuconfig
     ```

     First, select `Kernel hacking`

     ![Alt text](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/BiscuitOS_common_Kbuild.png)

     Then, set `Debug/Running kernel` as `Y` and select `TestCase
     configuration`

     ![Alt text](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel_hacking/kernel_hacking.png)

     Next, set `Testcase for kernel function` as `Y` and select
     `Task scheduler`

     ![Alt text](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel_hacking/testcase/TestCase.png)

     Finally, set `Test Task scheduler` and `GDT,LDT,GDTR,LDTR,IDTR,TR,
     Segment Descriptor` as `Y`.

     ![Alt text](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel_hacking/testcase/task/TASK_GDT.png)

  2. Enable debug demo code

     The main code for GDT/LDT on `*/tools/testcase/scheduler/gdt/gdt.c`,
     Developer can add test code on `debug_gdt_common`, such as:

     ```
       /* debug gdt common enter */
       void debug_gdt_common(void)
       {
           /* add test item here */

           /* ignore warning for un-used */
           if (1) {
               unsigned short limit;
               unsigned long base;

               parse_idtr(&limit, &base);
               parse_tr(&limit, &base);
               parse_stack_segment_descriptor();
               parse_code_segment_descriptor();
           }
       }

     ```

  3. Running test code

     If you configure correctly, you can run GDT/LDT demo code on qemu,
     such as:

     ```
       make
       make start
     ```

     ![Alt text](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel_hacking/testcase/task/TASK_GDT_running.png)
