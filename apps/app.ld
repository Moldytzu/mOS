OUTPUT_FORMAT(elf64-x86-64)
ENTRY(_mmain)

SECTIONS
{
	. = 0x100000;
	.text :
	{
		*(.text)
	}

	.data :
	{
		*(.data)
	}

	.rodata :
	{
		*(.rodata)
	}

	__bss_start = .;
	.bss :
	{
		*(COMMON)
		*(.bss)
	}
	__bss_end = .;
}