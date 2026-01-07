; Boot file for Moon OS Delta (32 bit system)
bits 32

section .multiboot
align 4
	dd 0x1BADB002
	dd 0x00000000
	dd 0xE4524FFE
	
section .data
	mlt_inf: dd 0
	
section .text

global start
global mlt_inf
extern krnl_run
start:
	cli
	mov esp, stack_top ; set the stack
	
	mov eax, 0
	mov [mlt_inf], ebx 
	
	call krnl_run ; call kernel function from kernel.c
	hlt
	
section .bss
	align 16
stack_bottom:
	resb 8192
stack_top:

section .note.GNU-stack noalloc noexec nowrite progbits