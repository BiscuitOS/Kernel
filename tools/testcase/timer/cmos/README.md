CMOS (Complementary metal oxide semiconductor)
----------------------------------------------------

  Nonvolatile BIOS memory refers to a small memory on PC motherboards that
  it used to store BIOS settings. It is traditionally called `CMOS RAM`
  because it used a volatile, low-power complementary metal-oxide-
  semiconductor (CMOS) SRAM powered by a small `CMOS` battery when system
  and standby power is off. The typical NVRAM capacity is 256 bytes.

### Backgroud

  The CMOS memory is actually a 64 or 128 byte battery-backed RAM memory
  module that is a part of the system clock chip. Some IBM PS/2 models
  have the capability for a 2k (2048 byte) CMOS ROM Extension.

  First used with clock-calender cards for the IBM PC-XT, When the PC/AT
  (Advanced Technology) was introduced in 1985, the Motorola MC146818
  become a part of the motherboard. Since the clock only uses fourteen of
  the RAM bytes, the rest are available for storing system configuration
  data.

  Interestingly, the original IBM-PC/AT (Advanced Technology) standard for
  the region 10-3Fh is nearly universal with one notable exception: The 
  IBM PS/2 systems deviate considerably (Note: AMSTRAD 8086 machines were
  among the first to actively use the CMOS memory available and since they
  *predate* the AT, do not follow the AT standard).

  This is just another example of how IBM created a standard, lost control
  of it, tried to replace it, failed and lost market share in the process.

  Originally, the IBM PC/AT only made use of a small portion of CMOS memory
  and was defined in the IBM PC/AT Technical Reference Manual, specifically
  bytes 10h, 12h, 14h-18h, 2Eh-33h. The balance was left undefined but was
  quickly appropriated by various BIOS manufacturers for such user-
  selectable options such as wait states, clock speeds, initial boot drive
  selection, and password storage.

  Later, as CMOS memory requirements grew, newer clock chips with 128 bytes
  of RAM came into use. However the fact remains that once the AT standard
  was established, only IBM has tried to change the definitions of that
  first description.

### Accessing the CMOS

  The CMOS memory exists outside of the normal address space and cannot
  contain directly executable code. It is reachable through `IN` and `OUT`
  commands at port number 70h (112d) and 71h (113d). To read a CMOS byte,
  and `OUT` to port 70h is executed with the address of the byte to by 
  read and and `IN` from port 71h will then retrieve the requested 
  information. The following BASIC fragment will read 128 CMOS bytes and
  print them to the screen in 8 rows of 16 values.

  CMOS RAM space has an upper limit of 128 bytes because of the structure
  of port 70: only bits 0-6 are used for addressing, bit 7 is used to 
  enable (0) or disable (1) Non-Maskable Interrupt (NMI) and explains why
  IBM uses 80h or to read/write data & follow with a `throw-away` call.

  Note that if the CMOS only has 64 bytes available, addressing will 
  generally wrap and addresses from 40h-7fh will mirror 00-3fh. Output
  will be hexadecimal.

  ```
    CLS
    FOR i = 0 TO &H7F
    OUT &H70, i
    PRINT USING "\   \"; HEX$(INP(&H71));
    NEXT i
    PRINT " "
  ```

  Note: 
  
  Where not otherwise noted, all data points are expressed as BYTES
  these are eight bit values and are read from MSB to LSB e.g.
  
  ```
    0000 0000  0101 1010 binary would be written as 5Ah
    7654 3210  When only some bit are used this is represented with
               Xs e.g. bit 5-3 would be shown as 00xx x000
  ```

### link

  ```
    http://bochs.sourceforge.net/techspec/CMOS-reference.txt
    http://stanislavs.org/helppc/cmos_ram.html

  ```
