EFLAGS: Current Status Register of Processor
---------------------------------------------------------------------

![EFLAGS Register](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel/MMU000002.png)

## Status Flags

The status flags (bit 0, 2, 4, 6, 7, and 11) of the `EFLAGS` register 
indicate the results of arithmetic instructions, such as the `ADD`, `SUB`,
`MUL`, and `DIV` instructions. The status flag functions are:

* **CF (bit 0)**
  
  **Carry flag**  -- Set if an arithmetic operation generates a carry or a
  borrow out of the most-significant bit of the result; Cleared otherwise.
  This flag indicates an overflow condition for unsigned-integer arithmetic.
  It is also used in multiple-precision arithmetic. `AAA`, `AAS`, `ADC`,
  `ADCX`, `ADD`, `DAA`, `DAS`, `BT`, `BTC`, `BTR`, `BTS`, `CLC`, `STC`,
  `MUL`, `SUB`, `SBB`, `SHL`, `SHR`, `SAL`, `SAR`, `RCL`, `RCR`, `ROR`, and
  `ROL` instructions will effect CF flag on an arithmetic operation.

## link

  [Intel Architectures Software Developer's Manual: Combined Volumes: 1 -- Chapter 3 Basic Execution Environment: 3.4 Basic Program Execution Register](https://software.intel.com/en-us/articles/intel-sdm)
