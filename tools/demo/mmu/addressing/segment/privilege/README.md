Privilege Levels
-----------------------------------------------------

The processor's segment-protection mechanism recognizes 4 privilege levels,
numbered from 0 to 3. The greater numbers mean lesser privileges. Figure shows
these levels of privilege can be interpreted as rings of protection.

![Protection Rings](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel/MMU000405.png)

The center (reserved for the most privilege code, data, and stacks) is used for
the segments containingthe critical software, usually the kernel of an 
operating system. Outer rings are used for less critical software. (Systems
that use only 2 of the 4 possible privilege levels should use level 0 and 3).

The processor uses privilege levels to prevent a program or task operating at a
lesser privilege level from accessing a segment with a grater privilege, except
under controlled situations. When the processor detects a privilege level 
violation, it generates a general-protection exception (#GP).

To carry out privilege-level checks between code segments and data segments,
the processor recognizes the following three types of privilege levels:

* Current Privielge Level (CPL)

  The CPL is the privilege level of the currently executing program or task. 
  It is stored in bits 0 and 1 of the CS and SS segment register. Normally, 
  the CPL is equal to the privilege level of the code segment from which 
  instructions are being fetched. The processor changes the CPL when program
  control is transferred to a code segment with a different privilege level.
  The CPL is treated slightly differently when accessing conforming code 
  segments. Conforming code segments can be accessed from any privilege level
  that is equal to or numerically greater (less privileged) than the DPL of the
  conforming code segment. Also, the CPL is not changed when the processor 
  accesses a conforming code segment that has a different privilege level than
  CPL.

* Descriptor privilege level (DPL)

  The DPL is the privilege level of a segment or gate. It is stored in the DPL
  field of the segment or gate descriptor for the segment or gate. When the 
  currently executing code segment attempts to access a segment or gate, the
  DPL of the segment or gate is compared to the CPL and RPL of the segment or 
  gate selector (as descriptor later in this section). The DPL is interpreted
  differently, depending on the type of segment or gate being accessed:

  -- **Data segment** -- The DPL indicates the numerically highest privilege 
     level that a program or task can have to be allowed to access the segment.
     For example, if the DPL of a data segment is 1, only programs running at
     a CPL of 0 or 1 can access the segment.

  -- **Nonconforming code segment (without using a call gate)** -- The DPL
     indicates the privilege level that a program or task must be at to access
     the segment. For example, if the DPL of a nonconforming code segment is
     0, only programs running at a CPL of 0 can access the segment.

  -- **Call gate** -- The DPL indicates the numerically highest privilege level
     that the currently executing program or task can be at and still be able 
     to access the call gate. (This is the same access rule as for a data 
     segment.)

  -- **Conforming code segment and nonconforming code segment accessed 
     through a call gate** -- The DPL indicates the numberically lowest 
     privilege level that a program or task can have to be allowed to access
     the segment. For example, if the DPL of conforming code segment is 2,
     programs running at a CPL of 0 or 1 cannot access the segment.

  -- **TSS** -- The DPL indicates the numerically highest privilege level that
     the currently executing program or task can be at and still be able to 
     access the TSS. (This is the same access rule as for a data segment).
     
* Requested privilege level (RPL)

  The PRL is an override privilege level that is assigned to segment selector.
  It is stored in bits 0 and 1 of the segment selector. The processor checks
  the RPL along with the CPL to determine if access to a segment is allowed. 
  Even if the program or task requesting access to a segment has sufficient
  privilege to access the segment, access is denied if the RPL is not of
  sufficient privilege level. That is, if the RPL of a segment selector is 
  numerically greater than the CPL, the PRL overrides the CPL, and vice versa.
  The PRL can be used to insure that privileged code does not access a segment
  on behalf of an application program unless the program itself has access 
  privileges for that segment.

Privilage levels are checked when the segment selector of a segment descriptor
is loaded into a segment registger. The checks used for data access differ from
those used for transfers of program control among code segment; therefore, the
two kinds of accesses are considered separately in the following sections.

