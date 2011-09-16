/*
 * ld directives for the user mode program
 */

FORCE_COMMON_ALLOCATION

SECTIONS {
	/* Beginning of program space */
	. = 0x20000000; /* 512 MB */

	_start = .;

	.text : {
		*(.text)
		_etext = .;
		PROVIDE (etext = .);
	}

	.data : {
		__CTOR_LIST__ = .;
		LONG((__CTOR_END__ - __CTOR_LIST__) / 4 - 2)
		*(.ctors)
		LONG(0)
		__CTOR_END__ = .;
		__DTOR_LIST__ = .;
		LONG((__DTOR_END__ - __DTOR_LIST__) / 4 - 2)
		*(.dtors)
		LONG(0)
		__DTOR_END__ = .;

		*(.fini)
		*(.anno)
		*(.rodata)
		*(.rodata.str1.1)
		*(.rodata.str1.4)
		*(.rodata.str1.32)
		*(__ex_table)
		*(.rodata.cst8)
		*(.data)
		_edata  =  .;
	}

	.bss : {
		__bss_start = .;
		*(.bss)
		*(COMMON)
	}

	_end = .;

	/DISCARD/ : {
		*(.comment)
	}
}
