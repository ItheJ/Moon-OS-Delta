#include "panic.h"

extern unsigned int bg_col;
extern unsigned int text_col;

void PANIC(const char* msg, Registers* reg){
	asm volatile("cli");
	
	bg_col = 0x4;
	text_col = 0xF;
	
	csc();
	cls();
	
	push_text(" * * * Moon OS Delta fatal kernel stop * * *\n\n");
	
	push_text("The core was stopped unexpectedly!\nReason: ");
	push_text(msg);
	push_char('\n');
	
	push_text("LAST REGISTERS:\n EAX: ");
	push_h32(reg->eax);
	push_text("  ");
	push_text("EBX: ");
	push_h32(reg->ebx);
	push_text("  ");
	push_text("ECX: ");
	push_h32(reg->ecx);
	push_char('\n');
	
	push_text("EDX: ");
	push_h32(reg->edx);
	push_text("  ");
	push_text("ESI: ");
	push_h32(reg->esi);
	push_text("  ");
	push_text("EDI: ");
	push_h32(reg->edi);
	push_char('\n');
	
	push_text("EIP: ");
	push_h32(reg->eip);
	push_text("  ");
	push_text("ESP: ");
	push_h32(reg->esp);
	push_text("  ");
	push_text("EBP: ");
	push_h32(reg->ebp);
	
	if (streq("PAGE FAULT!", msg) == 0){
		unsigned int fault_addr;
		asm volatile("mov %%cr2, %0" : "=r"(fault_addr));

		push_char('\n');
		push_text(msg);
		push_text(" at ");
		push_h32(fault_addr);
	}
	
	push_text("\n\nSystem halted. Please report this error.\n");
	
	while(1) asm volatile("hlt");
}