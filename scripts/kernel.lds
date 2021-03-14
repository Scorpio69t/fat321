ENTRY(_start)
SECTIONS
{

	. = 0x100000 + 0xffff800000000000;
	.text : AT(0x100000)
	{
		_text = .;
		*(.multiboot_header);
        *(.head.text);
		*(.text)
		_etext = .;
	}

	. = ALIGN(8);

	.data :
	{
		_data = .;
		*(.data)
		_edata = .;
	}

	.rodata :
	{
		_rodata = .;
		*(.rodata)
		_erodata = .;
	}

	.bss :
	{
		_bss = .;
		*(.bss)
		_ebss = .;
	}

	_end = .;
}
