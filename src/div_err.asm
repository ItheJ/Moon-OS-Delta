extern PANIC

section .data
	de_msg: db "DIVIDE ERROR!", 0
	
section .text
	global de_handler_entry

de_handler_entry:
	pushad ;save all registers
	
	push ds
	push es
	push fs
	push gs
	
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	
	;prepairing args for PANIC
	push de_msg
	
	push ss
	push gs
	push fs
	push es
	push ds
	push esp ; will be correct down the code
	push ebp
	push edi
	push esi
	push edx
	push ecx
	push ebx
	push eax
	
	mov eax, [esp + 60] ;get esp and push new esp insead of old esp
	mov [esp + 28], eax
	
	cld
	push esp
	
	push de_msg
	
	call PANIC
	
	;if panic return control
	
	add esp, 8
	
	pop gs
	pop fs
	pop es
	pop ds
	popad
	
	iret
	
section .note.GNU-stack noalloc noexec nowrite progbits