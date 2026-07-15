#ifndef PANIC_H
#define PANIC_H

#include "../../drivers/vga/vga.h"
#include "../../mdstr/mdstr.h"
#include "../../drivers/keyboard/keyboard.h"
#include "../../sys/idt/idt.h"

typedef struct{
	unsigned int ds;
	unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
	unsigned int int_no, err_code;
	unsigned int eip, cs, eflags;
	unsigned int user_esp, user_ss;
} Registers;

void PANIC(const char* msg, Registers* reg);
void RESCUE_CONSOLE(const char* msg, Registers* reg);

#endif