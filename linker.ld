load_addr = 0x80000000 + 1m;
kernel_stack_size = 512k;

ENTRY(_start)

SECTIONS
{
	. = load_addr;
	/*PROVIDE( __global_pointer$ = . + 2048m);*/
	PROVIDE( __global_pointer$ = .);

	.text ALIGN(4k): {
		*(.text)
	}

	.rodata ALIGN(4k): {
		*(.rodata)
		*(.srodata)
	}

	.data ALIGN(4k): {
		*(.data)
		*(.sdata)
	}

	.bss ALIGN(4k) (NOLOAD): {
		PROVIDE(kernel_bss_start = .);
		*(.bss)
		*(.sbss)
		PROVIDE(kernel_bss_end = .);
	}

	.stack ALIGN(4k) (NOLOAD): {
		. += kernel_stack_size;
		PROVIDE(kernel_stack_top = .);
	}

	PROVIDE(kernel_end = .);
}
