ENTRY("start")
OUTPUT_FORMAT("binary")

MEMORY
{
	C_CODE_SECTORS (rwx) : ORIGIN = 0x8000, LENGTH = 11k
}

SECTIONS
{
	. = ORIGIN(C_CODE_SECTORS);

	__data_start__ = .;

	.text :
	{
		*(.text.start);
		*(.text);
	}

	/* Delete useless, commentary sections from the output file */
	/DISCARD/ :
	{
		*(.comment*);
		*(.note*);
		*(.interp*);
		*(.eh_frame*);
	}

	__data_end__ = .;
}
