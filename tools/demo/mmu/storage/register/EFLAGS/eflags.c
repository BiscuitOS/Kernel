/*
 * EFLAGS: Current Status Register of Processor
 *
 * (C) 2018.08.10 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <demo/debug.h>

#define CF_BIT              0x00
#define PF_BIT              0x02

/*
 * LAHF -- Load Status Flags into AH Register
 *
 *   LAHE
 *
 *   This instruction executes as described above in compatibility mode and
 *   legacy mode.
 *
 *   IF instrcution LAHF
 *       THEN
 *           AH <--- EFLAGS(SF:ZF:0:AF:0:PF:1:CF)
 *   FI
 *
 *   The state of the flags in the EFLAGS register is not affected.
 */
static __unused int eflags_LAHF(void)
{
    unsigned char AH;

    /*
     * Layout of AH register
     *
     * +-----+----------------------------------------+
     * | Bit | Descriptor                             |
     * +-----+----------------------------------------+
     * | 0   | CF flag                                |
     * +-----+----------------------------------------+
     * | 1   | Reserved as 1                          |
     * +-----+----------------------------------------+
     * | 2   | PF flag                                |
     * +-----+----------------------------------------+
     * | 3   | Reserved as 0                          |
     * +-----+----------------------------------------+
     * | 4   | AF flag                                |
     * +-----+----------------------------------------+
     * | 5   | Reserved as 0                          |
     * +-----+----------------------------------------+
     * | 6   | ZF flag                                |
     * +-----+----------------------------------------+
     * | 7   | SF flag                                |
     * +-----+----------------------------------------+
     */
    __asm__ ("lahf\n\r"
             "mov %%ah, %0" : "=m" (AH) :);
    printk("EFLAGS: LAHF to AH: %#x\n", AH);

    return 0;
}

/*
 * SAHF -- Store AH into Flags
 *
 *   SAHF
 *
 *   Loads the SF, ZF, AF, PF, and CF flags of the EFLAGS register with value
 *   from the corresponding bits in the AH register (bit 7, 6, 4, 2, and 0,
 *   respectively). Bit 1, 3, and 5 of register AH are ignored; the
 *   corresponding reserved bits (1, 3, and 5) in the EFLAGS register remain.
 *
 *   IF instrcution SAHF
 *       THEN
 *           EFLAGS(SF:ZF:0:AF:0:PF:1:CF) <---- AH
 *   FI
 *
 *   The SF, Zf, AF, PF, and CF flags are loaded with values from the AH
 *   register. Bits 1, 3, and 5 of the EFLAGS register are unaffected, with
 *   values remaining 1, 0, and 0, respectively.
 */
static __unused int eflags_SAHF(void)
{
    unsigned char AH;

    __asm__ ("lahf\n\r"
             "mov %%ah, %0" : "=m" (AH) :);

    /*
     * Layout of AH register
     *
     * +-----+----------------------------------------+
     * | Bit | Descriptor                             |
     * +-----+----------------------------------------+
     * | 0   | CF flag                                |
     * +-----+----------------------------------------+
     * | 1   | Reserved as 1                          |
     * +-----+----------------------------------------+
     * | 2   | PF flag                                |
     * +-----+----------------------------------------+
     * | 3   | Reserved as 0                          |
     * +-----+----------------------------------------+
     * | 4   | AF flag                                |
     * +-----+----------------------------------------+
     * | 5   | Reserved as 0                          |
     * +-----+----------------------------------------+
     * | 6   | ZF flag                                |
     * +-----+----------------------------------------+
     * | 7   | SF flag                                |
     * +-----+----------------------------------------+
     */
    __asm__ ("sahf" :: "a" (AH));
    return 0;
}

/*
 * PUSHF -- Push EFLAGS Register onto the Stack
 *
 *   PUSHF
 *
 *   Decrements the stack pointer by 4 (if the current operand-size attribute
 *   is 32) and pushes the entire contents of the EFLAGS register onto the 
 *   stack, or decrements the stack pointer by 2 (if the operand-size 
 *   attribute is 16) and pushes the lower 16 bits of the EFLAGS register (
 *   that is, the EFLAGS register) onto the stack. These instructions reverse 
 *   the operation of the POPF instructions.
 *
 *   When copying the entire EFLAGS register to the stack, the VM and RF flags
 *   (bit 16 and bit 17) are not copied; instead, the values for these flags
 *   are cleared in the EFLAGS image stored on the stack.
 *
 *   The PUSHF (push flags) mnemonics reference the same opcode. The PUSHF
 *   instrcution is intended for use when the operand-size attribute is 16
 *   and the PUSHFD instruction for when the operand-size attribute is 32.
 *   Some assemblers may force the operand size to 16 when PUSHF is used
 *   and to 32 when PUSHFD is used. Others may treat these mnemonics as
 *   synonyms (PUSHF/PUSHFD) and used the current setting of the operand-size
 *   attribute to determine the size of values to be pushed from the stack,
 *   regardless of the mnemonic used.
 */
static __unused int eflags_PUSHF(void)
{
    unsigned int EFLAGS;

    __asm__ ("pushf\n\r"
             "popl %%eax" : "=a" (EFLAGS) :);

    printk("EFLAGS: %#x\n", EFLAGS);
    return 0;
}

/*
 * POPF -- Pop Stack into EFLAGS Register
 *
 *   POPF
 *
 *   Pops a doubleword from the top of the stack (if the current operand-size
 *   attribute is 32) and stores the value in the EFLAGS register, or pops
 *   a word from the top of the stack (if the operand-size attribute is 16)
 *   and stores it in the lower 16 bits of the EFLAGS register (that is, the
 *   EFLAGS register). These instructions reverse the operation of the PUSHF
 *   instructions.
 */
static __unused int eflags_POPF(void)
{
    unsigned int EFLAGS;

    __asm__ ("pushf\n\r"
             "popl %%eax" : "=a" (EFLAGS) :);

    __asm__ ("pushl %%eax\n\r"
             "popfl" :: "a" (EFLAGS));
    return 0;
}

/*
 * CF (bit 0) -- Carry flag
 *
 *   Carry flag -- Set if an arithmetic operation generates a carry or a
 *   borrow out of the most-significant bit of the result; cleared otherwise.
 *   This flag indicates an overflow condition for unsigned-integer 
 *   arithmetic. It is also used in multiple-precision arithmetic.
 */
static __unused int eflags_CF(void)
{
    unsigned char  __unused CF;
    unsigned short __unused AX;
    unsigned short __unused BX;
    unsigned short __unused CX;
    unsigned short __unused DX;
    unsigned int   __unused EAX;

#ifdef CONFIG_DEBUG_CF_AAA
    /* 
     * AAA -- ASCII Adjust After Addition 
     *   Adjust the sum of two unpacked BCD value to create an unpacked BCD
     *   result. The AL register is the implied source and destination
     *   operand for this instruction. The AAA instruction is only useful
     *   when it follows an ADD instruction that adds (binary addition) two
     *   unpacked BCD values and stores a byte result in the AL register. 
     *   The AAA instruction then adjust the contents of the AL register to
     *   contain the correct 1-digit unpacked BCD result.
     *
     *   If the addition produces a decimal carry, the AH register increments
     *   by 1, and the CF and AF flags are set. If there was no decimal 
     *   carry, the CF and AF flags are clear and the AH register is 
     *   unchanged. In either case, bits 4 through 7 of the AL register are
     *   set to 0.
     *
     *   IF instruction AAA
     *       IF ((AL AND 0FH) > 9) or (AF = 1)
     *           THEN
     *               AX <---- AX + 106H
     *               AF <---- 1
     *               CF <---- 1
     *           ELSE
     *               AF <---- 0
     *               CF <---- 0
     *       FI
     *       AL <---- AL AND 0FH
     *   FI
     *
     * More information about BCD, see: tools/demo/Data/Base/BCD/README.md
     *
     * JC  -- Jump if Carry bit is set.
     * JNC -- Jump if Carry bit is clear.
     */ 
    __asm__ ("mov $0x9, %%ax\n\r"
             "add $0x1, %%al\n\r"
             "aaa\n\r"
             "jc CF_SET\n\r"
             "jnc CF_CLEAR\n\r"
      "CF_SET:\n\r"
             "mov $1, %%ebx\n\r"
             "jmp out\n\r"
    "CF_CLEAR:\n\r"
             "mov $0, %%ebx\n\r"
             "out:\n\r"
             "mov %%ebx, %0\n\r"
             "mov %%ax, %1" : "=m" (CF), "=m" (AX):);

    if (CF)
        printk("CF has carry on AAA instruction. Unpacked BCD: %#x\n", AX);

#endif

#ifdef CONFIG_DEBUG_CF_AAS
    /*
     * AAS -- ASCII adjust AL after subtraction
     *   Adjust the result of the subtraction of two unpacked BCD value to 
     *   create a unpacked BCD result. The AL register is the implied source
     *   and destination operand for this instruction. The AAS instruction
     *   is only useful when it follows a SUB instruction that subtracts (
     *   binary subtraction) one unpacked BCD value from another and stores
     *   a byte result in the AL register. The AAA instruction then adjusts
     *   the contents of the AL register to contain the correct 1-digit
     *   unpacked BCD result.
     *
     *   If the subtraction produced a decimal carry, the AH register 
     *   decrements by 1, and the CF and AF flags are set. If no decimal
     *   carry occurred, the CF and AF flags are cleared, and the AH register
     *   is unchanged. In either case, the AL register is left with its top
     *   four bits set to 0.
     *
     *   IF ((AL AND 0FH) > 9) or (AF = 1)
     *       THEN
     *           AX <---- AX - 6
     *           AH <---- AH - 1
     *           AF <---- 1
     *           CF <---- 1
     *           AL <---- AL AND 0FH
     *       ELSE
     *           CF <---- 0
     *           AF <---- 0
     *           AL <---- AL AND 0FH
     *   FI
     *
     * More information about BCD, see: tools/demo/Data/Base/BCD/README.md
     *
     * JC  -- Jump if Carry bit is set.
     * JNC -- Jump if Carry bit is clear.
     */
    __asm__ ("mov $8, %%ax\n\r"
             "sub $9, %%al\n\r"
             "aas\n\r"
             "jc CF_SET\n\r"
             "jc CF_CLEAR\n\r"
      "CF_SET:\n\r"
             "mov $1, %%ebx\n\r"
             "jmp out\n\r"
    "CF_CLEAR:\n\r"
             "mov $0, %%ebx\n\r"
         "out:\n\r"
             "mov %%bx, %0\n\r"
             "mov %%ax, %1" : "=m" (CF), "=m" (AX));

    /*
     * Subtraction is done by adding the ten's complement of the subtrachend.
     * To illustrate signed unpacked BCD subtraction, as "8 - 9":
     * 
     * In signed unpacked BCD, 8 is '0000 1000'. The ten's complement of 9
     * can be obtained by taking the nine's complement of 9, and then adding
     * one. So "9 - 9 = 0", and "0 + 1 = 1" By preceding 1 in BCD by 
     * the negative sign code, the number "-9" can be represented. So, "-9"
     * in signed BCD is "0000 1001 0000 0001".
     *
     * Now that both numbers are represented in signed BCD, they can be added
     * together:
     * 
     *                                      0000 1000
     *                                              8
     *                          + 0000 1001 0000 0001
     *                                    9         1
     *                          ---------------------
     *                          = 0000 1001 0000 1001
     *                                    9         9
     *
     * To represent the sign of a number in BCD, the number "0000" is unsed
     * to represent a positive number, and "1001" is used to represent a 
     * negative number. So adding 6 to the invalid entries result in the 
     * following:
     *
     *                            0000 1001 0000 1001
     *                                    9         9
     *                          + 0000 0000 0000 0000
     *                                    0         0
     *                          ---------------------
     *                          = 0000 1001 0000 1001
     *                                    9         9
     *
     * Thus the result of the subtraction if "0000 1001 0000 1001" 
     * (-9). To check the answer, note that the first digit is 9, which means
     * negative. This seems to be correct, since "8 - 9" should result in a 
     * negative number. To check the rest of the digits, represent them in
     * decimal. "0000 1001" is 9. Then ten's complement of 9 is "10 - 9"
     * = "9 - 9 + 1" = "0 + 1" = 1. so the calculated answer is "-1".
     */
    if (CF)
        printk("CF has carry on AAS instruction. Unpacked BCD: %#x\n", AX);

#endif

#ifdef CONFIG_DEBUG_CF_ADC
    /*
     * ADC -- Add with Carry
     *
     * Adds the destination operand (first operand), the source operand (
     * second operand), and the carry (CF) flag and stores the result in the
     * destination operand. The destination operand can be a register or a
     * memory location; the source operand can be an immediate, a register,
     * or a memory location. (However, two memory operand cannot be used in
     * one instruction.) The state of the CF flag represents a carry from
     * a previous addition. When an immediate value is used as an operand,
     * it is sign-extended to the length of the destination operand format.
     *
     * The ADC instruction does not distinguish between signed or unsigned
     * operands. Instead, the processor evaluates the result for both data
     * types and sets the OF and CF flags to indicate a carry in the signed
     * unsigned result, respectively. The SF flags indicates the sign of the
     * signed result.
     *
     * DEST <---- DEST + SRC + CF
     *
     */
    __asm__ ("mov $0xFFFF, %%ax\n\r"
             "add $0x1, %%ax\n\r"
             "mov $0x0, %%bx\n\r"
             "adc $0x1, %%bx\n\r"
             "mov %%bx, %0" : "=m" (BX) :);
    printk("ADC: BX = BX(0) + 1 + CF = %#x\n", BX);
#endif

#ifdef CONFIG_DEBUG_CF_ADCX
    /*
     * ADCX -- Unsigned integer addition of two operands with Carry flags
     *
     * Performs an unsigned addition of the destination operand (first 
     * operand), the source operand (second operand) and the carry-flag (CF)
     * and stores the result in the destination operand. The destination 
     * operand is a general purpose register, whereas the source operand
     * can be a general-purpose register or memory location. The state of
     * CF can represent a carry from a previous addition. The instruction
     * sets the CF flag with the carry generated by the unsigned addition
     * of the operands.
     *
     * The ADCX instruction is executed in the context of multi-precision 
     * addition, where we add a series of operands with a carry-chain. At
     * the beginning of a chain of additions, we need to make sure the CF
     * is in a desired initial state. Often, this inital state need to be
     * 0, which can be achieved with an instruction to zero the CF.
     *
     * CF:DEST[31:0] <---- DEST[31:0] + SRC[31:0] + CF
     */
    __asm__ ("movl $0xFFFFFFFF, %%eax\n\r"
             "adcx $0x1, %%eax\n\r"
             "jc CF_SET\n\r"
             "jnc CF_CLEAR\n\r"
      "CF_SET:\n\r"
             "mov $1, %%bx\n\r"
             "jmp out\n\r"
    "CF_CLEAR:\n\r"
             "mov $0, %%bx\n\r"
         "out:\n\r"
             "mov %%bx, %0\n\r"
             "movl %%eax, %1"
             : "=m" (CF), "=m" (EAX) :);
    if (CF)
        printk("ADCX: 0xFFFFFFFF + 0x1 + CF = %#x\n", EAX);
#endif

#ifdef CONFIG_DEBUG_CF_ADD
    /*
     * ADD -- Addition with two operands.
     *
     * Adds the destination operand (first operand) and the source operand (
     * second operand) and then stores the result in the destination operand.
     * The destination operand can be a register or a memory location; the 
     * source operand can be an immediate, a register, or a memory location.
     * (However, two memory operands cannot be used in one instruction.) When
     * an immediate value is used as an operand, it it sign-extended to the
     * length of the destination operand format.
     * 
     * The ADD instruction performs integer addition. It evaluates the result
     * for both signed and unsigned integer operands and sets the CF and
     * OF flags to indicate a carry (overflow) in the signed or unsigned 
     * result, respectively. The SF flag indicates the sign of the signed 
     * result.
     *
     * DEST <---- DEST + SRC
     *
     * The OF, SF, ZF, AF, CF, and PF flags are set according to the result.
     */
    __asm__ ("mov $0xFFFF, %%ax\n\r"
             "add $1, %%ax\n\r"
             "jc CF_SET\n\r"
             "jnc CF_CLEAR\n\r"
      "CF_SET:\n\r"
             "mov $1, %%bx\n\r"
             "jmp out\n\r"
    "CF_CLEAR:\n\r"
             "mov $0, %%bx\n\r"
         "out:\n\r"
             "mov %%bx, %0\n\r"
             "mov %%ax, %1"
             : "=m" (CF), "=m" (AX) :);
    if (CF) 
        printk("ADD: Carry on 0xFFFF + 0x1 = %#x\n", AX);
#endif

#ifdef CONFIG_DEBUG_CF_BT
    /*
     * BT -- Bit Test
     *
     * Select the bit in a bit string (specified with the first operand, 
     * called the bit base) at the bit-position designated by the bit offset
     * (specified by the second operand) and stores the value of the bit
     * in the CF flag. The bit base operand can be a register or memory 
     * location; the bit offset operand can be a register or an immediate
     * value:
     *
     * * If the bit base operand specifies a register, the instruction takes
     *   the modulo 16, 32, or 64 of the bit offset operand (modulo size 
     *   depends on the mode and register size).
     *
     * * If the bit base operand specifies a memory location, the operand
     *   represents the address of the byte in memory that contains the bit
     *   base (bit 0 of the specified byte) of the bit string. The range of
     *   the bit position that can be referenced by the offset operand depends
     *   on the operand size.
     *
     * CF <---- Bit(BitBase, BitOffset)
     * 
     * The CF flag contains the value of the selected bit. The ZF flag is 
     * unaffected. The OF, SF, AF, and PF flags are undefined.
     */
    DX = 0x3;
    __asm__ ("mov %1, %%ax\n\r"
             "bt $0, %%ax\n\r"
             "jc CF_SET\n\r"
             "jnc CF_CLEAR\n\r"
      "CF_SET:\n\r"
             "mov $1, %%bx\n\r"
             "jmp out\n\r"
    "CF_CLEAR:\n\r"
             "mov $0, %%bx\n\r"
         "out:\n\r"
             "mov %%bx, %0"
             : "=m" (CF) : "m" (DX));
    if (CF)
        printk("BT: %#x test bit 0 is set\n", DX);
    else
        printk("BT: %#x test bit 0 is cleared\n", DX);
#endif

#ifdef CONFIG_DEBUG_CF_BTC
    /*
     * BTC -- Bit Test and Complement
     *
     * Selects the bit in a bit string (specified with the first operand, 
     * called the bit base) at the bit-position designated by the bit offset
     * operand (second operand), stores the value of the bit in the CF flag,
     * and complements the selected bit in the bit string. The bit base 
     * operand can be a register or a memory location; the bit offset operand
     * can be a register or an immediate value.
     *
     * CF <---- Bit(BitBase, BitOffset)
     * Bit(BitBase, BitOffset) <---- BOT Bit(BitBase, BitOffset)
     *
     * The CF flag contains the value of the selected bit before it is 
     * complemented. The ZF flag is unaffected. The OF, SF, AF, and PF flags
     * are undefined.
     */
    CX = 0x3;
    __asm__ ("mov %2, %%ax\n\r"
             "btc $0, %%ax\n\r"
             "jc CF_SET\n\r"
             "jnc CF_CLEAR\n\r"
      "CF_SET:\n\r"
             "mov $1, %%bx\n\r"
             "jmp out\n\r"
    "CF_CLEAR:\n\r"
             "mov $0, %%bx\n\r"
         "out:\n\r"
             "mov %%bx, %0\n\r"
             "mov %%ax, %1"
             : "=m" (CF), "=m" (AX) : "m" (CX));
    if (CF) 
        printk("BTC: %#x Bit 0 has set and invert bit 0 as %#x\n", CX, AX);
    else
        printk("BTC: %#x Bit 0 has cleared and invert bit 0 as %#x\n", 
                                            CX, AX);
#endif

#ifdef CONFIG_DEBUG_CF_BTR
    /*
     * BTR -- Bit Test and Reset
     *
     * Selector the bit in a bit string (specified with the first operand,
     * called the bit base) at the bit-position designated by the bit offset
     * operand (second operand), stores the value of the bit in the CF flag,
     * and clears the selected bit in the bit string to 0.
     *
     * CF <---- Bit(BitBase, BitOffset)
     * Bit(BitBase, BitOffset) <---- 0
     *
     * The CF flag contains the value of the selected bit before it is 
     * cleared. The ZF flag is unaffected. The OF, SF, AF, and PF flags
     * are undefined.
     */
    DX = 0x203;
    __asm__ ("mov %2, %%ax\n\r"
             "btr $0, %%ax\n\r"
             "jc CF_SET\n\r"
             "jnc CF_CLEAR\n\r"
      "CF_SET:\n\r"
             "mov $1, %%bx\n\r"
             "jmp out\n\r"
    "CF_CLEAR:\n\r"
             "mov $0, %%bx\n\r"
         "out:\n\r"
             "mov %%bx, %0\n\r"
             "mov %%ax, %1"
             : "=m" (CF), "=m" (AX) : "m" (DX));
    if (CF)
        printk("BTR: %#x Bit 0 has set and clear bit 0 as %#x\n", DX, AX);
    else
        printk("BTR: %#x Bit 0 has clear and clear bit 0 as %#x\n", DX, AX);
#endif

#ifdef CONFIG_DEBUG_CF_BTS
    /*
     * BTS -- Bit Test and Set
     *
     * Selects the bit in a bit string (specified with the first operand,
     * called the bit base) at the bit-position designated by the bit offset
     * operand (second operand), stores the value of the bit in the CF flag,
     * and sets the selected bit in the bit string to 1.
     *
     * CF <---- Bit(BitBase, BitOffset)
     * Bit(BitBase, BitOffset) <---- 1
     *
     * The CF flag contains the value of the selected bit before it is set.
     * The ZF flag is unaffected. The OF, SF, AF, and PF flags are undefined.
     */
    DX = 0x203;
    __asm__ ("mov %2, %%ax\n\r"
             "bts $0, %%ax\n\r"
             "jc CF_SET\n\r"
             "jnc CF_CLEAR\n\r"
      "CF_SET:\n\r"
             "mov $1, %%bx\n\r"
             "jmp out\n\r"
    "CF_CLEAR:\n\r"
             "mov $0, %%bx\n\r"
         "out:\n\r"
             "mov %%bx, %0\n\r"
             "mov %%ax, %1"
             : "=m" (CF), "=m" (AX) : "m" (DX));
    if (CF)
        printk("BTS: %#x Bit 0 has set and set bit 0 as %#x\n", DX, AX);
    else
        printk("BTS: %#x Bit 0 has clear and set bit 0 as %#x\n", DX, AX);
#endif

#ifdef CONFIG_DEBUG_CF_CLC
    /*
     * CLC -- Clear Carry Flag
     *
     * Clears the CF flag in the EFLAGS register. Operation is the same in
     * all modes.
     *
     * CF <---- 0
     *
     * The CF flag is set to 0. The OF, ZF, SF, AF, and PF flags are
     * unaffected.
     */
    __asm__ ("clc\n\r"
             "jc CF_SET\n\r"
             "jnc CF_CLEAR\n\r"
      "CF_SET:\n\r"
             "mov $1, %%bx\n\r"
             "jmp out\n\r"
    "CF_CLEAR:\n\r"
             "mov $0, %%bx\n\r"
         "out:\n\r"
             "mov %%bx, %0"
             : "=m" (CF) :);
    if (CF)
        panic("CLC: CF doesn't clear on EFLAGS register");
    else
        printk("CLC: CF has cleared on EFLAGS register\n");
#endif

#ifdef CONFIG_DEBUG_CF_STC
    /*
     * STC -- Set Carry Flag
     *
     * Sets the CF flag in the EFLAGS register. Operation is the same in 
     * all modes.
     *
     * CF <---- 1
     *
     * The CF flag is set. The OF, ZF, SF, AF, and PF flags are unaffected.
     */
    __asm__ ("stc\n\r"
             "jc CF_SET\n\r"
             "jnc CF_CLEAR\n\r"
      "CF_SET:\n\r"
             "mov $1, %%bx\n\r"
             "jmp out\n\r"
    "CF_CLEAR:\n\r"
             "mov $0, %%bx\n\r"
         "out:\n\r"
             "mov %%bx, %0"
             : "=m" (CF) :);
    if (CF)
        printk("STC: CF has set on EFLAGS register\n");
    else
        panic("STC: CF has cleared on EFLAGS register");
#endif

    return 0;
}

static int debug_eflags(void)
{
#ifdef CONFIG_DEBUG_EFLAGS_LAHF
    eflags_LAHF();
#endif

#ifdef CONFIG_DEBUG_EFLAGS_SAHF
    eflags_SAHF();
#endif

#ifdef CONFIG_DEBUG_EFLAGS_PUSHF
    eflags_PUSHF();
#endif

#ifdef CONFIG_DEBUG_EFLAGS_POPF
    eflags_POPF();
#endif

#ifdef CONFIG_DEBUG_EFLAGS_CF
    eflags_CF();
#endif

    return 0;
}
late_debugcall(debug_eflags);
