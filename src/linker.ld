ENTRY(start)
OUTPUT_FORMAT(elf32-i386)

SECTIONS {
	
	. = 1M;
	
	.multiboot 0x100000 : {
		*(.multiboot)
	}
	
	.text : {
		*(.text)
	}
	
	.rodata : {
		*(.rodata)
    }
	
	.data : {
		*(.data)
	}
	
	.bss : {
		__bss_start = .;
		*(COMMON)
		*(.bss)
		__bss_end = .;
	}
	
	/DISCARD/ : {
		*(.comment)
		*(.note*)
	}
}