OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(bootsect_start)

SECTIONS
{
	. = 0;
	.boot : { *(.bstext)}
	. = ASSERT(. <= 512, "Boot too big!");
}
