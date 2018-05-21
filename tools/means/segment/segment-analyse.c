#include <stdio.h>
#include <stdlib.h>
#include "segment.h"

static struct seg_desc desc0;

/*
 * System descriptor type
 *   Table shows the encoding of the type field for system-segment descriptor
 *   and gate descriptors. Note that system descriptors in IA-32 mode are
 *   16 bytes instead of 8 bytes.
 *
 * ------------------------------------------------------------------------
 * |     Type field        |           Description                        |    
 * ------------------------------------------------------------------------
 * | Dec | 11 | 10 | 9 | 8 |             32-bit Mode                      |
 * ------------------------------------------------------------------------
 * |  0  | 0  |  0 | 0 | 0 | Reserved                                     |    
 * ------------------------------------------------------------------------
 * |  1  | 0  |  0 | 0 | 1 | 16-bit TSS (Available)                       |    
 * ------------------------------------------------------------------------
 * |  2  | 0  |  0 | 1 | 0 | LDT                                          |    
 * ------------------------------------------------------------------------
 * |  3  | 0  |  0 | 1 | 1 | 16-bit TSS (Busy)                            |    
 * ------------------------------------------------------------------------
 * |  4  | 0  |  1 | 0 | 0 | 16-bit Call Gate                             |    
 * ------------------------------------------------------------------------
 * |  5  | 0  |  1 | 0 | 1 | Task Gate                                    |    
 * ------------------------------------------------------------------------
 * |  6  | 0  |  1 | 1 | 0 | 16-bit Interrupt Gate                        |    
 * ------------------------------------------------------------------------
 * |  7  | 0  |  1 | 1 | 1 | 16-bit Trap Gate                             |    
 * ------------------------------------------------------------------------
 * |  8  | 1  |  0 | 0 | 0 | Reserved                                     |    
 * ------------------------------------------------------------------------
 * |  9  | 1  |  0 | 0 | 1 | 32-bit TSS Avaiable                          |    
 * ------------------------------------------------------------------------
 * | 10  | 1  |  0 | 1 | 0 | Reserved                                     |    
 * ------------------------------------------------------------------------
 * | 11  | 1  |  0 | 1 | 1 | 32-bit TSS (Busy)                            |    
 * ------------------------------------------------------------------------
 * | 12  | 1  |  1 | 0 | 0 | 32-bit Call Gate                             |    
 * ------------------------------------------------------------------------
 * | 13  | 1  |  1 | 0 | 1 | Reserved                                     |    
 * ------------------------------------------------------------------------
 * | 14  | 1  |  1 | 1 | 0 | 32-bit Interrupt Gate                        |    
 * ------------------------------------------------------------------------
 * | 15  | 1  |  1 | 1 | 1 | 32-bit Trap Gate                             |
 * ------------------------------------------------------------------------    
 *
 * @desc: segment descriptor
 *
 * @return: 0 is correct.
 *          1 is data or code segment descriptor
 */
static int system_gate_type(struct seg_desc *desc)
{
    if (desc->flag & 0x01)
        return 1;

    switch (desc->type) {
    case 1:
        printf("16-bit TSS (Available)\n");
        break;
    case 2:
        printf("LDT\n");
        break;
    case 3:
        printf("16-bit TSS (Busy)\n");
        break;
    case 4:
        printf("16-bit Call Gate\n");
        break;
    case 5:
        printf("Task Gate\n");
        break;
    case 6:
        printf("16-bit Interrupt Gate\n");
        break;
    case 7:
        printf("16-bit Trap Gate\n");
        break;
    case 9:
        printf("32-bit TSS (Available)\n");
        break;
    case 11:
        printf("32-bit TSS (Busy)\n");
        break;
    case 12:
        printf("32-bit Call Gate\n");
        break;
    case 14:
        printf("32-bit Interrupt Gate\n");
        break;
    case 15:
        printf("32-bit Trap Gate\n");
        break;
    default:
        /* Reserved */
        break;
    }
    return 0; 
}

/*
 * Segment Descriptor Type
 *   When the S(descriptor type) flag in a segment descriptor is set,
 *   the descriptor is for either a code or data segment. The highest order
 *   bit of the the type field (bit 11 of second double word of the 
 *   segment descriptor) then determines whether the descriptor is for a
 *   data segment (clear) or a code segment (set).
 *
 * @desc: struction of segment descriptor
 *
 * @return: 0 is a code or data segment
 *          1 is a system segment
 *          2 is first segment descriptor on GDT
 */
static int segment_descriptor_type(struct seg_desc *desc)
{

    if (!desc) {
        printf("The segment is first member: NULL\n");
        return 2;
    }

    if (!(desc->flag & 0x1)) {
        /* system segment */
        system_gate_type(desc);
        return 1;
    }

    /*
     * For data segments, the three low-order bits of the type field (bits
     * 8, 9 and 10) are interpreted as accessed (A), write-enable (W), and 
     * expansion-direction (E). See Table for a description of the encodeing
     * of the bits in the byte field for code and data segments. Data 
     * segments can be read-only or read/write segments, depending on the
     * setting of the write-enable bit.
     *
     * Table. Code- and Data-Segment Types
     * ----------------------------------------------------------------------
     * |      Type Field       | Descriptor Type | Description              |
     * ----------------------------------------------------------------------
     * | Dec | 11 | 10 | 9 | 8 |                 |                          |
     * |     |    | E  | W | A |                 |                          |
     * ----------------------------------------------------------------------
     * |  0  | 0  | 0  | 0 | 0 |      Data       | Read-Only                |
     * ----------------------------------------------------------------------
     * |  1  | 0  | 0  | 0 | 1 |      Data       | Read-Only, accessed      |
     * ----------------------------------------------------------------------
     * |  2  | 0  | 0  | 1 | 0 |      Data       | Read/Write               |
     * ----------------------------------------------------------------------
     * |  3  | 0  | 0  | 1 | 1 |      Data       | Read/Write, accessed     |
     * ----------------------------------------------------------------------
     * |  4  | 0  | 1  | 0 | 0 |      Data       | Read/Write, expand-down  |
     * ----------------------------------------------------------------------
     * |  5  | 0  | 1  | 0 | 1 |      Data       | Read-Only, expand-down   |
     * |     |    |    |   |   |                 | accessed                 |
     * ----------------------------------------------------------------------
     * |  6  | 0  | 1  | 1 | 0 |      Data       | Read/Write, expand-down  |
     * ----------------------------------------------------------------------
     * |  7  | 0  | 1  | 1 | 1 |      Data       | Read/Write, expand-down  |
     * |     |    |    |   |   |                 | accessed                 |
     * ----------------------------------------------------------------------
     * |     |    | C  | R | A |                 |                          |
     * ----------------------------------------------------------------------
     * |  8  | 1  | 0  | 0 | 0 |      Code       | Execute-Only             |
     * ----------------------------------------------------------------------
     * |  9  | 1  | 0  | 0 | 1 |      Code       | Execute-Only, accessed   |
     * ----------------------------------------------------------------------
     * | 10  | 1  | 0  | 1 | 0 |      Code       | Execute/Read             |
     * ----------------------------------------------------------------------
     * | 11  | 1  | 0  | 1 | 1 |      Code       | Execute/Read, accessed   |
     * ----------------------------------------------------------------------
     * | 12  | 1  | 1  | 0 | 0 |      Code       | Execute-Only, conforming |
     * ----------------------------------------------------------------------
     * | 13  | 1  | 1  | 0 | 1 |      Code       | Execute-Only, conforming |
     * |     |    |    |   |   |                 | accessed                 |
     * ----------------------------------------------------------------------
     * | 14  | 1  | 1  | 1 | 0 |      Code       | Execute/Read, conforming |
     * ----------------------------------------------------------------------
     * | 15  | 1  | 1  | 1 | 1 |      Code       | Execute/Read, conforming |
     * |     |    |    |   |   |                 | accessed                 |
     * ----------------------------------------------------------------------
     */
    if (desc->type & 0x8) {
        /* Code segment */
        printf("Code Segment:");
        /* Accssed?*/
        if (desc->type & 0x1)
            printf(" Accessed");
        /* Execute-only or Execute/Read ? */
        if (desc->type & 0x2)
            printf(" Execute/Read");
        else
            printf(" Execute-only");
        /* conforming ? */
        if (desc->type & 0x4)
            printf(" conforming");
        printf("\n");
    } else {
        /* Data segment */
        printf("Data Segment:");
        /* Accessed ? */
        if (desc->type & 0x1)
            printf(" Accessed");
        /* Read-Only or Read/Write ? */
        if (desc->type & 0x2)
            printf(" Read/Write");
        else
            printf(" Read-Only");
        /* expand-down ? */
        if (desc->type & 0x4)
            printf(" expand-down");
        printf("\n");
    }
}

/*
 * Parse Segment Descriptor
 *
 * 31--------24---------20------------15------------------7-----------0
 * | Base 31:24|G|D/B|AVL|segLimt 19:16|P| DPL | S | Type |Base 23:16 |
 * --------------------------------------------------------------------
 * | Base Address 15:0                 | Segment Limit 15:0           |
 * -------------------------------------------------------------------- 
 */
static void parse_Segment_descriptor(void)
{
    unsigned short desc[4];
    int i;

    printf("\nNote! Please input 0x.. , Bitwidth 16bit (LSB to MSB)\n");
    printf("e.g.: 0x200\n\n");
    for (i = 0; i < 4; i++) {
        printf("Input Segment Descriptor[%d]:\n>>  ", i);
        scanf("%x", (unsigned int *)&desc[i]);
    }

    /*
     * Base address fields
     *   Defines the location of byte 0 of the segment within the 4-GByte
     *   linear address space. The processor puts together the three
     *   base address fields to form a single 32-bit value. Segment base
     *   addresses should by aligned to 16-bytes boundaries. Although
     *   16-byte alignment is not required, this alignment allows programs
     *   to maximize performance by aligning code and data on 16-byte
     *   boundaries.  
     */
    desc0.base = desc[1] | ((desc[2] & 0xFF) << 16) |
                 (((desc[3] >> 8) & 0xFF) << 24);
    /*
     * Segment limit field
     *   Specify the size of the segment. The processor puts together
     *   two segment limit fields to from a 20-bit value. The processor
     *   interprets the segment limit in one of two ways, depending on
     *   the setting of the G(granularity) flag:
     *   -> If the granularity flag is clear, the segment size can range
     *      from 1 byte to 1M byte, in byte increments.
     *   -> If the granularity flag is set, the segment size can range 
     *      from 4 KBytes to 4GBytes, in 4-KByte increments.
     *   The processor uses the segment limit in two different ways,
     *   depending on whether the segment is an expand-up or an expand-down
     *   segment. for more information about segment types. For expand-up
     *   segments, the offset in a logical address can range from 0 to the
     *   segment limit. Offsets greater than the segment limit generate 
     *   general-protection exceptions(#GP, for all segment other than SS)
     *   or stack-fault exceptions (#SS for the SS segment). For expand-down
     *   segments, the segment limit has the reverse function. The offset
     *   can range from the segment limit plus 1 to 0xFFFFFFFF or 0xFFFFH,
     *   depending on the setting of the B flag. Offset less then or equal
     *   to the segment limit generate general-protection exception or
     *   stack-fault exceptions. Decreasing the value in the segment limit
     *   field for an expand-down segment allocates new memory at the bottom
     *   of the segment's address space, rather than at the top. IA-32
     *   architecture stacks always grow downwards, making this mechanism
     *   convenient for expandable stacks.
     */
    desc0.limit = desc[0] | ((desc[3] & 0xF) << 16);
    /*
     * DPL (Descriptor privilege level) field
     *   Specifies the privilege level of the segment. The privilege level
     *   can range from 0 to 3, with 0 being the most privilege level.
     *   The DPL is used to control access to the segment.
     */
    desc0.dpl = (desc[2] >> 13) & 0x3;
    /*
     * Type field
     *   Indicates the segment or gate type and specifies the kind of 
     *   access that can be made to the segment and the direction of 
     *   growth. The interpretation of this field depends on whether the 
     *   descriptor type flag specifies an application (code or data)
     *   descriptor or a system descriptor. The encoding of the type field
     *   is different for code, data, and system descriptors.
     */
    desc0.type = (desc[2] >> 8) & 0xF;
    /*
     * S (descriptor) flag
     *   Specify whether the segment descriptor is for a system segment(S
     *   flag is clear) or code or data segment (S flag is set).
     * 
     *   desc->flag:0
     */
    desc0.flag |= (desc[2] >> 12) & 0x1;
    /*
     * P (segment-present) flag
     *   Indicates whether the segment is present in memory (set) or not
     *   present (clear). If this flag is clear, the processor generates
     *   a segment-not-present exception(#NP) when a segment selector that
     *   points to the segment descriptor is loaded into a segment register.
     *   Memory management software can use this flag to control which 
     *   segments are actully loaded into physical memory at a given time.
     *   It offers a control in a addition to paging for managing vitual
     *   memory.
     *
     *   When this flag is clear, the operating system or executive is free
     *   to use the locations marked "Available" to store its own data, such
     *   as information regarding the whereabouts of the missing segment.
     *
     *   31--------16-15--14-13-12--11----8-7--------------------------0
     *   | Available | O | DPL | S | Type  | Available                 |
     *   ---------------------------------------------------------------
     *   |                 Available                                   |
     *   ---------------------------------------------------------------
     *
     *   desc->flag:1
     */
    desc0.flag |= ((desc[2] >> 15) & 0x01) << 1;
    /*
     * D/B (default operation size/ default stack pointer size/ or 
     *      upper bound) flag
     *   Performs different functions depending on whether the segment 
     *   descriptor is an executable code segment, an expand-down data
     *   segment, or a stack segment. (This flag should always be set to 1
     *   for 32-bit code and data segments and to 0 for 16-bit code and 
     *   data segments).
     *
     *   -> Executable code segment:
     *      The flag is called the D flag and it indicates the default 
     *      length for effective addresses and operands referenced by
     *      instructions in the segment. If the flag is set, 32-bit address
     *      and 32-bit or 8-bit operands are assumed. If it is clear, 16-bit
     *      addresses and 16-bit or 8-bit operands are assumed.
     *      The instruction prefix 66H can be used to select an operand size
     *      other than the default, and the prefix 67H can be used select 
     *      an address size other than the default.
     *
     *   -> Stack segment (data segment pointer to by the SS register)
     *      The flag is called the B(big) flag and it specifies the size of
     *      the stack pointer used for implicit stack operations (such as
     *      pushes, pops, and calls). If the flag is set, a 32-bit stack
     *      pointer is used, which is store in the 32-bit ESP register.
     *      If the flag is clear, a 16-bit stack pointer is used, which is
     *      stored in the 16-bit SP register. If the stack segment is set up
     *      to be an expand-down data segment, the B flag also specifies the
     *      upper bound of the stack segment.
     *
     *   -> Expand-down data segment
     *      The flag is called the B flag and it specifies the upper bound 
     *      of the segment. If the flag is set, the upper bound is 
     *      0xFFFFFFFFH (4 GBytes). If the flag is clear, the upper bound
     *      is 0xFFFFH (64 KBytes).
     *
     *    desc->flag:2
     */
    desc0.flag |= ((desc[3] >> 7) & 0x01) << 0x2;
    /*
     * G (granularity) flag
     *   Determines the scaling of the segment limit field. When the 
     *   granularity flag is clear, the segment limit is interpreted in byte
     *   units. When flag is set, the segment limit is interpreted in 4-KByte
     *   units. (This flag does not affect the granularity of the base 
     *   address. it is always byte granular). When the granularity flag is
     *   set, the twelve least significant bits of an offset are not tested
     *   when checking the offset against the segment limit. For example,
     *   when the granularity flag is set, a limit of 0 results in valid
     *   offsets from 0 to 4095.
     *
     *   desc->flag:3
     */
    desc0.flag |= ((desc[3] >> 8) & 0x01) << 0x3;
    /*
     * L (64-bit code segment) flag
     *   In IA-32e mode, bit 21 of the second doubleword of the segment
     *   descriptor indicates whether a code segment contains native 64-bit
     *   code. A value of 1 indicates instructions in this code segment
     *   are executed in 64-bit mode. A value of 0 indicates the instructions
     *   in this code segment are executed in compatibility mode. If L-bit
     *   is set, then D-bit must be cleard. When not in IA-32e mode or for
     *   non-code segments, bit 21 is reserved and should always be set to 0.
     *
     *   desc->flag:4
     */
    desc0.flag |= ((desc[3] >> 6) & 0x01) << 0x04;
    /*
     * Available and reserved bits
     *   Bit 20 of the second doubleword of the segment descriptor is
     *   available for use by system software.
     *
     *   desc->flag:5
     */
    desc0.flag |= ((desc[3] >> 4) & 0x01) << 0x05;

    printf("\n************************\n");
    system_gate_type(&desc0);
    segment_descriptor_type(&desc0);
    printf("Base Addess: %#x\n", (unsigned int)desc0.base);
    printf("Limit: %#x\n", (unsigned int)desc0.limit);
    printf("************************\n");
}

int main()
{
    int sel;

    printf("***************************************************\n");
    printf("******Segment Descriptor Analyse tools*************\n");
    printf("***************************************************\n");
    printf("\n");
    printf("Select Segment descriptor type:\n");
    printf("** 0. GDT Segment Descriptor\n");
    printf("** 1. LDT\n");
    printf("** 2. DS\n");
    printf("** 3. CS\n");
    printf("** 4. TR\n");
    printf(">> ");
    scanf("%d", &sel);

    switch (sel) {
    case 0:
        parse_Segment_descriptor();
        break;
    default:
        printf("Invalid selcect!\n");
        return -1;
    }
    return 0;
}
