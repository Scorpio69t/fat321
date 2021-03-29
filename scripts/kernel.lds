OUTPUT_FORMAT("elf64-x86-64", "elf64-x86-64", "elf64-x86-64")
OUTPUT_ARCH(i386:x86-64)
ENTRY(_start)
SECTIONS
{

	. = 0x100000 + 0xffff800000000000;
	.text : AT(0x100000)
	{
		_text = .;
		*(.head.text);
		*(.multiboot_header);
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

	. = ALIGN(8192);
  	.data.init_proc : { *(.data.init_proc) }

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
