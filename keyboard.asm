global keyboard_handler_entry
extern keyboard_handler

keyboard_handler_entry:
	pushad
	
	sub esp, 12
	
	call keyboard_handler
	
	add esp, 12
	
	mov al, 0x20
	out 0x20, al
	
	popad
	iretd
	
section .note.GNU-stack noalloc noexec nowrite progbits