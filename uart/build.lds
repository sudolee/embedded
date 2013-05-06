OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")

ENTRY(_start)
SECTIONS
{
	. = 0x0;

	. = ALIGN(4);
	.text :
	{
		*(.text.uart*)
		*(.text*)
	}

	. = ALIGN(4);
	.data :
	{
		*(.data*)
	}

	. = ALIGN(4);
	.bss :
	{
		*(.bss*)
	}

    /DISCARD/ : { *(.dynstr*) }
    /DISCARD/ : { *(.dynamic*) }
    /DISCARD/ : { *(.plt*) }
    /DISCARD/ : { *(.interp*) }
    /DISCARD/ : { *(.gnu*) }
    /DISCARD/ : { *(.comment*) }
}
