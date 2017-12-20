8253 Programmable Interval Timer
--------------------------------------------------

### Function Description

  The 8253 is programmable interval timer/counter sepcifically designed
  for use with the Intel Microcomputer systems. Its function is that
  of a general purpose, multi-timing element that can be treated as
  an array of I/O ports in the system software.

  The 8253 solves one of the most common problem in any microcomputer
  system, the generation of accurate time delays under software control.
  Instead of setting up timing loops in system software, the programmer
  configures the 8253 to match his requirements, initializes one of the
  counters of the 8253 with the desired quantity, then upon command the
  8253 will count out the delay and interrupt the CPU when it has 
  completed its tasks. It is easy to see that the software overhead
  is minimal and that multiple delays can easily be maintained by 
  assignment of priority levels.

  Other counter/timer functions that are non-delay in nature but also
  common to most microcompures can be implemented with the 8253.

  * Programmable Rate Generator
 
  * Event Counter

  * Binary Rate Multiplier

  * Real Time Clock

  * Digital One-Shot

  * Complex Motor Controller

### Bus Buffer

  The 3-state, bi-directional, 8-bit buffer is used to interface the 8253
  to the system data bus. Data is transmitted or received by the buffer
  upon execution of INPUT or OUTPUT CPU instructions. The Data Bus Buffer
  has three basic functions.

  1. Programming the MODES of the 8253

  2. Loading the count registers.

  3. Reading the count values.

### Read/Write Logic

  The Read/Write Logic accepts inputs from the system bus and in turn 
  generates control signals for overall device operation. It is enabled or
  disabled by CS so that no operation can occur to change the function 
  unless the devices has been selected by the system logic.

  * Read

    A `low` on this input informs the 8253 that the CPU is inputting data
    in the form of a counters value.

  * Write

    A `low` on this input informs the 8253 that the CPU is outputting data
    in the form of mode information or loading counters.

  * A0,A1

    These inputs are normally connected to the address bus. Their function
    is to select one of the three counters to be operated on and to address
    the control word register for mode selection.
  
  * CS (Chip Select)

    A `low` on this input enables the 8253. No reading or writting will occur
    unless the device is selected. The `CS` input has no effect upon the 
    actual operation of the counters.

### Control Word Register

  The Control Word Register is selected when `A0`, `A1` are `11`. It then
  accepts information from the data bus buffer and stores it in a register.
  The information stored in this register controls the operation MODE of
  each counter, selection of binary or BCD counting and the loading of
  each count register.

  The Control Word Register can only be written into. no read operation
  of its counters is available.

### Counter0, Counter1 and Counter2

  There three functional blocks are identical in operation so only a single
  counter will be described. Each Counter consists of a single, 16-bit,
  pre-settable, DOWN counter. The counter can operate in either binary
  or BCD and its input, gate and output are configured by the selection of
  MODES stored in the Control Word Register.

  The counters are fully independent and each can have separate MODE
  configuration and counting operation, binary or BCD. Also, there are
  special feature in the control word that handle the loading of the count
  value so that software overhead can be minimized for these functions.

  The reading of the counters of each counter is available to the programmer
  with simple READ operations for event counting applications and special
  commands and logic are included in the 8253 so that the counters of each
  counter can be read `on the fly` without having to inhibit the clock
  input.

### Debug on BiscuitOS

  BiscuitOS support online debug 8253 on `qemu`, developer should open
  Kernel-macro when configure kernel. Shortly, follow these step to
  enable Intel 8253 on system.

  1. Enale specifical Kernel-macro

     Invoke `make menuconfig` on top on source tree, and enable
     specifical item as follow figures.

     ```
       make menuconfig
     ```

     First, select `Kernel hacking`

     ![Alt text](https://github.com/EmulateSpace/PictureSet/blob/master/CMOS/CMOS0.png)

     Then, set `Debug/Running kernel` as `Y` and select `TestCase
     configuration`

     ![Alt text](https://github.com/EmulateSpace/PictureSet/blob/master/CMOS/CMOS1.png)

     Next, set `Testcase for kernel function` as `Y` and select
     `Timer and CMOS clock`

     ![Alt text](https://github.com/EmulateSpace/PictureSet/blob/master/CMOS/CMOS2.png)

     Finally, set `Test Timer and CMOS clock` and `Intel 8253 Timer` as `Y`.

     ![Alt text](https://github.com/EmulateSpace/PictureSet/blob/master/CMOS/8253_1.png)

  2. Enable debug demo code

     The main code for CMOS RTC on `*/tools/testcase/timer/8253/8253.c`,
     Developer can add test code on `debug_8253_common`, such as:

     ```
       /* common entry on 8253 */
       void debug_8253_common(void)
       {
           /* Add item */

           /* Ignor warning */
           if (1) {
               unsigned short value;

               /* Counter1, LSB/MSB, ONE_SHOT, 16Bit */
               intel8253_write_ctr(INTEL8253_COUNTER2, MODE0_INTERRUPT,
                                   DATA_ALL, F_16BIT);
               /* Write LSB data */
               intel8253_write_LSB(INTEL8253_COUNTER2, 0x20);
               /* Write MSB data */
               intel8253_write_MSB(INTEL8253_COUNTER2, 0x11);
               /* Read from Counter 1 */
               value = intel8253_read(PORT_COUNTER2);
               printk("Value1 %#x\n", value);

               /* Re-Write new value */
               intel8253_write(INTEL8253_COUNTER2, 0x1102);
               value = intel8253_read(PORT_COUNTER2);
               printk("Value2 %#x\n", value);
           }
       }

     ```

  3. Running test code

     If you configure correctly, you can run CMOS RTC demo code on qemu,
     such as:

     ```
       make
       make start
     ```

     ![Alt text](https://github.com/EmulateSpace/PictureSet/blob/master/CMOS/8253_2.png)



