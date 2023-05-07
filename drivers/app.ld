OUTPUT_FORMAT(elf64-x86-64)
ENTRY(_mdrvmain)

SECTIONS
{
	. = 0xA000000000;
	.text :
	{
		*(.text)
	}

	. += CONSTANT(MAXPAGESIZE);
	
	.data :
	{
		*(.data)
	}
	
	. += CONSTANT(MAXPAGESIZE);
	
	.rodata :
	{
		*(.rodata)
	}
	
	. += CONSTANT(MAXPAGESIZE);
	
	.bss :
	{
		*(COMMON)
		*(.bss)
	}
	
	. += CONSTANT(MAXPAGESIZE);
}