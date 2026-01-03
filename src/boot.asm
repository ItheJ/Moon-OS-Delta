; Boot file for Moon OS Delta (32 bit system)

bits 32

section .multiboot
align 4
	dd 0x1BADB002
	dd 0x00000000
	dd 0xE4524FFE
	
section .text
global start
extern krnl_run ; define krnl_run function from kernel.c

start:
	cli
	mov esp, stack_top ; set the stack
	
	mov eax, 0
	mov ebx, 0
	
	call krnl_run ; call kernel function from kernel.c
	hlt
	
section .bss
	align 16
	
stack_bottom:
	resb 8192; stack size
stack_top:
 
section .note.GNU-stack noalloc noexec nowrite progbits