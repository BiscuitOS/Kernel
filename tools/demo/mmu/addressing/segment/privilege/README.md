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

## Privilege level checking when accessing data segments

To access operands in a data segment, the segment selector for the data segment
must be loaded into to the data segment register (DS, ES, FS, or GS) or into
the stack segment register (SS). (Segment registers can be loaded with the MOV,
POP, LDS, LES, LFS, LGS, and LSS instructions.) Before the processor loads a
segment selector into a segent register, it performs a privilege check (see 
Figure) by comparing the privilege levels of the currently running program or
task (the CPL), the RPL of the segment selector, and the DPL of the segment's
segment descriptor. The processor loads the segment selector into the segment
register if the DPL is numerically greater than or equal to both the CPL and
RPL. Otherwise, a general-protection fault is generated and the segment 
register is not loaded.

```
  CS Regsiter
  +-------------------------+-----+
  |                         | CPL |---------o
  +-------------------------+-----+         |
                                            |      +-----------------+
                                            |      |                 |
  Segment Selector for Data Segment         o----->|                 |
  +-------------------------+-----+                |                 |
  |                         | RPL |--------------->| Privilege Check |
  +-------------------------+-----+                |                 |
                                            o----->|                 |
                                            |      |                 |
  Data-Segment Descriptor                   |      +-----------------+
  +-------------+-----+-----------+         | 
  |             | DPL |           |---------o
  +-------------+-----+-----------+
  +-------------------------------+
  |                               |
  +-------------------------------+
```
Figure show four procedures (located in codes segments A, B, C, and D), each
running at different privilege levels and each attempting to access the same
data segment.

#### Case 0: CPL == RPL == DPL

The procedure in code segment A0 is able to access data segment E0 using 
segment selector E0, because the CPL of code segment A0 and the RPL of segment
selector E0 are equal to the DPL of data segment E0.

```
CPL == RPL == DPL == 0

+-----------------+       +------------------+        +-----------------+
| Code Segment A0 |       | Segment Sel. E0  |        | Data Segment E0 |
|                -|------>|                 -|------->|                 |
| CPL = 0         |       | RPL = 0          |        | DPL = 0         |
+-----------------+       +------------------+        +-----------------+
```

The procedure in code segment A1 is able to access data segment E1 using 
segment selector E1, because the CPL of code segment A1 and the RPL of segment 
selector E1 are equal to the DPL of data segment E1.

```
CPL == RPL == DPL == 1

+-----------------+       +------------------+        +-----------------+
| Code Segment A1 |       | Segment Sel. E1  |        | Data Segment E1 |
|                -|------>|                 -|------->|                 |
| CPL = 1         |       | RPL = 1          |        | DPL = 1         |
+-----------------+       +------------------+        +-----------------+
```

The procedure in code segment A2 is able to access data segment E2 using
segment selector E2, because the CPL of code segment A2 and the RPL of segment
selector E2 are equal to the DPL of data segment E2.

```
CPL == RPL == DPL == 2

+-----------------+       +------------------+        +-----------------+
| Code Segment A2 |       | Segment Sel. E2  |        | Data Segment E2 |
|                -|------>|                 -|------->|                 |
| CPL = 2         |       | RPL = 2          |        | DPL = 2         |
+-----------------+       +------------------+        +-----------------+
```

The procedure in code segment A3 is able to access data segment E3 using
segment selector E3, because the CPL of code segment A3 and the RPL of segment
selector E3 are equal to the DPL of data segment E3.

```
CPL == RPL == DPL == 3

+-----------------+       +------------------+        +-----------------+
| Code Segment A3 |       | Segment Sel. E3  |        | Data Segment E3 |
|                -|------>|                 -|------->|                 |
| CPL = 3         |       | RPL = 3          |        | DPL = 3         |
+-----------------+       +------------------+        +-----------------+
```


#### Case 1: CPL == RPL > DPL

The procedure in code segment B0 is able to access data segment E0 using segment
selector E0, because the CPL of code segment B0 and the RPL of segment selector
E0 are both numerically lower than (more privileged) the DPL of data segment E0.

```
+-----------------+       +------------------+        +-----------------+
| Code Segment B0 |       | Segment Sel. E0  |        | Data Segment E0 |
|                -|------>|                 -|------->|                 |
| CPL = 0         |       | RPL = 0          |        | DPL = 1         |
+-----------------+       +------------------+        +-----------------+
```

The procedure in code segment B1 is able to access data segment E1 using segment
selector E1, because the CPL of code segment B1 and the RPL of segment selector
E1 are both numerically lower than (more privileged) the DPL of data segment E1.

```
+-----------------+       +------------------+        +-----------------+
| Code Segment B1 |       | Segment Sel. E1  |        | Data Segment E1 |
|                -|------>|                 -|------->|                 |
| CPL = 0         |       | RPL = 0          |        | DPL = 2         |
+-----------------+       +------------------+        +-----------------+
```

The procedure in code segment B2 is able to access data segment E2 using segment
selector E2, because the CPL of code segment B2 and the RPL of segment selector
E2 are both numerically lower than (more privileged) the DPL of data segment E2.
   
```
+-----------------+       +------------------+        +-----------------+
| Code Segment B2 |       | Segment Sel. E2  |        | Data Segment E2 |
|                -|------>|                 -|------->|                 |
| CPL = 1         |       | RPL = 1          |        | DPL = 2         |
+-----------------+       +------------------+        +-----------------+
```

The procedure in code segment B3 is able to access data segment E3 using segment
selector E3, because the CPL of code segment B3 and the RPL of segment selector
E3 are both numerically lower than (more privileged) the DPL of data segment E3.

```
+-----------------+       +------------------+        +-----------------+
| Code Segment B3 |       | Segment Sel. E3  |        | Data Segment E3 |
|                -|------>|                 -|------->|                 |
| CPL = 0         |       | RPL = 0          |        | DPL = 3         |
+-----------------+       +------------------+        +-----------------+
```

The procedure in code segment B4 is able to access data segment E4 using segment
selector E4, because the CPL of code segment B4 and the RPL of segment selector
E4 are both numerically lower than (more privileged) the DPL of data segment E4.

```
+-----------------+       +------------------+        +-----------------+
| Code Segment B4 |       | Segment Sel. E4  |        | Data Segment E4 |
|                -|------>|                 -|------->|                 |
| CPL = 1         |       | RPL = 1          |        | DPL = 3         |
+-----------------+       +------------------+        +-----------------+
```

The procedure in code segment B5 is able to access data segment E5 using segment
selector E5, because the CPL of code segment B5 and the RPL of segment selector
E5 are both numerically lower than (more privileged) the DPL of data segment E5.

```
+-----------------+       +------------------+        +-----------------+
| Code Segment B5 |       | Segment Sel. E5  |        | Data Segment E5 |
|                -|------>|                 -|------->|                 |
| CPL = 2         |       | RPL = 2          |        | DPL = 3         |
+-----------------+       +------------------+        +-----------------+
```

#### Case 2: CPL > RPL == DPL

The procedure in code segment C0 is able to access data segment E0 using segment
selector E0, because the CPL of code segment C0 is numerically lower than (more
privilege) then RPL of segment selector E0, and the RPL of segment selector E0
is equal to DPL of data segment E0.

```
+-----------------+       +------------------+        +-----------------+
| Code Segment C0 |       | Segment Sel. E0  |        | Data Segment E0 |
|                -|------>|                 -|------->|                 |
| CPL = 0         |       | RPL = 1          |        | DPL = 1         |
+-----------------+       +------------------+        +-----------------+
```

The procedure in code segment C1 is able to access data segment E1 using segment
selector E1, because the CPL of code segment C1 is numerically lower than (more
privilege) then RPL of segment selector E1, and the RPL of segment selector E1
is equal to DPL of data segment E1.

```
+-----------------+       +------------------+        +-----------------+
| Code Segment C1 |       | Segment Sel. E1  |        | Data Segment E1 |
|                -|------>|                 -|------->|                 |
| CPL = 0         |       | RPL = 2          |        | DPL = 2         |
+-----------------+       +------------------+        +-----------------+
```

The procedure in code segment C2 is able to access data segment E2 using segment
selector E2, because the CPL of code segment C2 is numerically lower than (more
privilege) then RPL of segment selector E2, and the RPL of segment selector E2
is equal to DPL of data segment E2.

```
+-----------------+       +------------------+        +-----------------+
| Code Segment C2 |       | Segment Sel. E2  |        | Data Segment E2 |
|                -|------>|                 -|------->|                 |
| CPL = 0         |       | RPL = 3          |        | DPL = 3         |
+-----------------+       +------------------+        +-----------------+
```

The procedure in code segment C3 is able to access data segment E3 using segment
selector E3, because the CPL of code segment C3 is numerically lower than (more
privilege) then RPL of segment selector E3, and the RPL of segment selector E3
is equal to DPL of data segment E3.

```
+-----------------+       +------------------+        +-----------------+
| Code Segment C3 |       | Segment Sel. E3  |        | Data Segment E3 |
|                -|------>|                 -|------->|                 |
| CPL = 1         |       | RPL = 2          |        | DPL = 2         |
+-----------------+       +------------------+        +-----------------+
```

The procedure in code segment C4 is able to access data segment E4 using segment
selector E4, because the CPL of code segment C4 is numerically lower than (more
privilege) then RPL of segment selector E4, and the RPL of segment selector E4
is equal to DPL of data segment E4.

```
+-----------------+       +------------------+        +-----------------+
| Code Segment C4 |       | Segment Sel. E4  |        | Data Segment E4 |
|                -|------>|                 -|------->|                 |
| CPL = 1         |       | RPL = 3          |        | DPL = 3         |
+-----------------+       +------------------+        +-----------------+
```

The procedure in code segment C5 is able to access data segment E5 using segment
selector E5, because the CPL of code segment C5 is numerically lower than (more
privilege) then RPL of segment selector E5, and the RPL of segment selector E5
is equal to DPL of data segment E5.

```
+-----------------+       +------------------+        +-----------------+
| Code Segment C5 |       | Segment Sel. E5  |        | Data Segment E5 |
|                -|------>|                 -|------->|                 |
| CPL = 2         |       | RPL = 3          |        | DPL = 3         |
+-----------------+       +------------------+        +-----------------+
```

#### Case 3: CPL == RPL < DPL

The procedure in code segment D0 is not able to access data segment E0 using
segment selector E0, because the CPL of code segment D0 and the RPL of segment
selector E0 are both numerically greater than (less privilege) the DPL of data
segment E0. (The CPL of code segmetn D0 is equal to RPL of segment selecotr 
E0).

```
+-----------------+       +------------------+        +-----------------+
| Code Segment D0 |       | Segment Sel. E0  |        | Data Segment E0 |
|                -|------>|                 -|---X--->|                 |
| CPL = 1         |       | RPL = 1          |        | DPL = 0         |
+-----------------+       +------------------+        +-----------------+
```

The procedure in code segment D1 is not able to access data segment E1 using
segment selector E1, because the CPL of code segment D1 and the RPL of segment
selector E1 are both numerically greater than (less privilege) the DPL of data
segment E1. (The CPL of code segmetn D1 is equal to RPL of segment selecotr 
E1).

```
+-----------------+       +------------------+        +-----------------+
| Code Segment D1 |       | Segment Sel. E1  |        | Data Segment E1 |
|                -|------>|                 -|---X--->|                 |
| CPL = 2         |       | RPL = 2          |        | DPL = 0         |
+-----------------+       +------------------+        +-----------------+
```

The procedure in code segment D2 is not able to access data segment E2 using
segment selector E2, because the CPL of code segment D2 and the RPL of segment
selector E2 are both numerically greater than (less privilege) the DPL of data
segment E2. (The CPL of code segmetn D2 is equal to RPL of segment selecotr
E2).

```
+-----------------+       +------------------+        +-----------------+
| Code Segment D2 |       | Segment Sel. E2  |        | Data Segment E2 |
|                -|------>|                 -|---X--->|                 |
| CPL = 3         |       | RPL = 3          |        | DPL = 0         |
+-----------------+       +------------------+        +-----------------+
```

The procedure in code segment D3 is not able to access data segment E3 using
segment selector E3, because the CPL of code segment D3 and the RPL of segment
selector E3 are both numerically greater than (less privilege) the DPL of data
segment E3. (The CPL of code segmetn D3 is equal to RPL of segment selecotr
E3).

```
+-----------------+       +------------------+        +-----------------+
| Code Segment D3 |       | Segment Sel. E3  |        | Data Segment E3 |
|                -|------>|                 -|---X--->|                 |
| CPL = 2         |       | RPL = 2          |        | DPL = 1         |
+-----------------+       +------------------+        +-----------------+
```

The procedure in code segment D4 is not able to access data segment E4 using
segment selector E4, because the CPL of code segment D4 and the RPL of segment
selector E4 are both numerically greater than (less privilege) the DPL of data
segment E4. (The CPL of code segmetn D4 is equal to RPL of segment selecotr
E4).

```
+-----------------+       +------------------+        +-----------------+
| Code Segment D4 |       | Segment Sel. E4  |        | Data Segment E4 |
|                -|------>|                 -|---X--->|                 |
| CPL = 3         |       | RPL = 3          |        | DPL = 1         |
+-----------------+       +------------------+        +-----------------+
```

The procedure in code segment D5 is not able to access data segment E5 using
segment selector E5, because the CPL of code segment D5 and the RPL of segment
selector E5 are both numerically greater than (less privilege) the DPL of data
segment E5. (The CPL of code segmetn D5 is equal to RPL of segment selecotr
E5).

```
+-----------------+       +------------------+        +-----------------+
| Code Segment D5 |       | Segment Sel. E5  |        | Data Segment E5 |
|                -|------>|                 -|---X--->|                 |
| CPL = 3         |       | RPL = 3          |        | DPL = 2         |
+-----------------+       +------------------+        +-----------------+
```

#### Case 4: CPL < RPL == DPL

The procedure in code segment E0 is not able to access data segment E0 using
segment selector E0, because the CPL of code segment E0 both are numerically 
greater than (less privilege) the RPL of segment selector E0 and the DPL of the
data segment E0. (The RPL of segment selector E0 is equal to DPL of the data
segment E0).

```
+-----------------+       +------------------+        +-----------------+
| Code Segment E0 |       | Segment Sel. E0  |        | Data Segment E0 |
|                -|------>|                 -|---X--->|                 |
| CPL = 1         |       | RPL = 0          |        | DPL = 0         |
+-----------------+       +------------------+        +-----------------+
```

The procedure in code segment E1 is not able to access data segment E1 using
segment selector E1, because the CPL of code segment E1 both are numerically 
greater than (less privilege) the RPL of segment selector E1 and the DPL of the
data segment E1. (The RPL of segment selector E1 is equal to DPL of the data
segment E1).

```
+-----------------+       +------------------+        +-----------------+
| Code Segment E1 |       | Segment Sel. E1  |        | Data Segment E1 |
|                -|------>|                 -|---X--->|                 |
| CPL = 2         |       | RPL = 0          |        | DPL = 0         |
+-----------------+       +------------------+        +-----------------+
```

The procedure in code segment E2 is not able to access data segment E2 using
segment selector E2, because the CPL of code segment E2 both are numerically
greater than (less privilege) the RPL of segment selector E2 and the DPL of the
data segment E2. (The RPL of segment selector E2 is equal to DPL of the data
segment E2).

```
+-----------------+       +------------------+        +-----------------+
| Code Segment E2 |       | Segment Sel. E2  |        | Data Segment E2 |
|                -|------>|                 -|---X--->|                 |
| CPL = 3         |       | RPL = 0          |        | DPL = 0         |
+-----------------+       +------------------+        +-----------------+
```

The procedure in code segment E3 is not able to access data segment E3 using
segment selector E3, because the CPL of code segment E3 both are numerically
greater than (less privilege) the RPL of segment selector E3 and the DPL of the
data segment E3. (The RPL of segment selector E3 is equal to DPL of the data
segment E3).

```
+-----------------+       +------------------+        +-----------------+
| Code Segment E3 |       | Segment Sel. E3  |        | Data Segment E3 |
|                -|------>|                 -|---X--->|                 |
| CPL = 2         |       | RPL = 1          |        | DPL = 1         |
+-----------------+       +------------------+        +-----------------+
```

The procedure in code segment E4 is not able to access data segment E4 using
segment selector E4, because the CPL of code segment E4 both are numerically
greater than (less privilege) the RPL of segment selector E4 and the DPL of the
data segment E4. (The RPL of segment selector E4 is equal to DPL of the data
segment E4).

```
+-----------------+       +------------------+        +-----------------+
| Code Segment E4 |       | Segment Sel. E4  |        | Data Segment E4 |
|                -|------>|                 -|---X--->|                 |
| CPL = 3         |       | RPL = 1          |        | DPL = 1         |
+-----------------+       +------------------+        +-----------------+
```

The procedure in code segment E5 is not able to access data segment E5 using
segment selector E5, because the CPL of code segment E5 both are numerically
greater than (less privilege) the RPL of segment selector E5 and the DPL of the
data segment E5. (The RPL of segment selector E5 is equal to DPL of the data
segment E5).

```
+-----------------+       +------------------+        +-----------------+
| Code Segment E5 |       | Segment Sel. E5  |        | Data Segment E5 |
|                -|------>|                 -|---X--->|                 |
| CPL = 3         |       | RPL = 2          |        | DPL = 2         |
+-----------------+       +------------------+        +-----------------+
```

#### Case 5: CPL < DPL < RPL

The procedure in code segment F0 is not able to access data segment E0 using
segment selector E0. If the CPL of code segment F0 is numerically greate than 
(less privilege) the DPL of data segment E0, even the RPL of segment selector
E0 is numerically less than (more privilege) the DPL of data segment E0, the 
code segment F0 is able to access data segment E0.

```
+-----------------+       +------------------+        +-----------------+
| Code Segment F0 |       | Segment Sel. E0  |        | Data Segment E0 |
|                -|------>|                 -|---X--->|                 |
| CPL = 2         |       | RPL = 0          |        | DPL = 1         |
+-----------------+       +------------------+        +-----------------+
```

The procedure in code segment F1 is not able to access data segment E1 using
segment selector E1. If the CPL of code segment F1 is numerically greate than 
(less privilege) the DPL of data segment E1, even the RPL of segment selector 
E1 is numerically less than (more privilege) the DPL of data segment E1, the
code segment F1 is able to access data segment E1.

```
+-----------------+       +------------------+        +-----------------+
| Code Segment F1 |       | Segment Sel. E1  |        | Data Segment E1 |
|                -|------>|                 -|---X--->|                 |
| CPL = 3         |       | RPL = 0          |        | DPL = 1         |
+-----------------+       +------------------+        +-----------------+
```

The procedure in code segment F2 is not able to access data segment E2 using
segment selector E2. If the CPL of code segment F2 is numerically greate than
(less privilege) the DPL of data segment E2, even the RPL of segment selector
E2 is numerically less than (more privilege) the DPL of data segment E2, the
code segment F2 is able to access data segment E2.

```
+-----------------+       +------------------+        +-----------------+
| Code Segment F1 |       | Segment Sel. E1  |        | Data Segment E1 |
|                -|------>|                 -|---X--->|                 |
| CPL = 3         |       | RPL = 1          |        | DPL = 2         |
+-----------------+       +------------------+        +-----------------+
```

#### Case 6: CPL > DPL > RPL

The procedure in code segment G0 should be able to access data segment E0
because code segment D's CPL is numerically less than the DPL of data segment
E0. However, the RPL of segment selector E0 (which the code segment G0 
procedure is using to access data segment E0) is numerically greater than the
DPL of data segment E0, so access is not allowed.

```
+-----------------+       +------------------+        +-----------------+
| Code Segment G0 |       | Segment Sel. E0  |        | Data Segment E0 |
|                -|------>|                 -|---X--->|                 |
| CPL = 0         |       | RPL = 2          |        | DPL = 1         |
+-----------------+       +------------------+        +-----------------+
```

The procedure in code segment G1 should be able to access data segment E1
because code segment D's CPL is numerically less than the DPL of data segment
E1. However, the RPL of segment selector E1 (which the code segment G1
procedure is using to access data segment E1) is numerically greater than the
DPL of data segment E1, so access is not allowed.

```
+-----------------+       +------------------+        +-----------------+
| Code Segment G1 |       | Segment Sel. E1  |        | Data Segment E1 |
|                -|------>|                 -|---X--->|                 |
| CPL = 0         |       | RPL = 3          |        | DPL = 1         |
+-----------------+       +------------------+        +-----------------+
```

The procedure in code segment G2 should be able to access data segment E2
because code segment D's CPL is numerically less than the DPL of data segment
E2. However, the RPL of segment selector E2 (which the code segment G2
procedure is using to access data segment E2) is numerically greater than the
DPL of data segment E2, so access is not allowed.

```
+-----------------+       +------------------+        +-----------------+
| Code Segment G2 |       | Segment Sel. E2  |        | Data Segment E2 |
|                -|------>|                 -|---X--->|                 |
| CPL = 1         |       | RPL = 3          |        | DPL = 2         |
+-----------------+       +------------------+        +-----------------+
```

#### Case 7: CPL > RPL > DPL

The procedure in code segment H0 should be able to access data segment E0
because code segment D's CPL is numerically less than the DPL of data segment
E0. However, the RPL of segment selector E0 (which the code segment G0
procedure is using to access data segment E0) is numerically less than the
DPL of data segment E0, and the CPL of code segment G0 is numerically less 
than the RPL of segment selector E0, so access is allowed.

```
+-----------------+       +------------------+        +-----------------+
| Code Segment H0 |       | Segment Sel. E0  |        | Data Segment E0 |
|                -|------>|                 -|------->|                 |
| CPL = 0         |       | RPL = 1          |        | DPL = 2         |
+-----------------+       +------------------+        +-----------------+
```

The procedure in code segment H1 should be able to access data segment E1
because code segment D's CPL is numerically less than the DPL of data segment
E1. However, the RPL of segment selector E1 (which the code segment G1
procedure is using to access data segment E1) is numerically less than the
DPL of data segment E1, and the CPL of code segment G1 is numerically less 
than the RPL of segment selector E1, so access is allowed.

```
+-----------------+       +------------------+        +-----------------+
| Code Segment H1 |       | Segment Sel. E1  |        | Data Segment E1 |
|                -|------>|                 -|------->|                 |
| CPL = 0         |       | RPL = 2          |        | DPL = 3         |
+-----------------+       +------------------+        +-----------------+
```

The procedure in code segment H2 should be able to access data segment E2
because code segment D's CPL is numerically less than the DPL of data segment
E2. However, the RPL of segment selector E2 (which the code segment G2
procedure is using to access data segment E2) is numerically less than the
DPL of data segment E2, and the CPL of code segment G2 is numerically less
than the RPL of segment selector E2, so access is allowed.

```
+-----------------+       +------------------+        +-----------------+
| Code Segment H2 |       | Segment Sel. E2  |        | Data Segment E2 |
|                -|------>|                 -|------->|                 |
| CPL = 1         |       | RPL = 2          |        | DPL = 3         |
+-----------------+       +------------------+        +-----------------+
```
