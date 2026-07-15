#include "panic.h"

extern unsigned int bg_col;
extern unsigned int text_col;
extern unsigned int mlt_inf;

extern volatile unsigned int sys_args[5];
extern void check_comm(const char* comm);

void PANIC(const char* msg, Registers* reg){
	asm volatile("cli");
	if (!sys_args[1]){
		bg_col = 0x4;
		text_col = 0xF;
	
		csc();
	}
	
	cls();
	
	if (!sys_args[1]){
		push_text(" * * * Moon OS Delta fatal kernel stop * * *\n\n");
	}
	else {
		push_text(" * * * Moon OS Delta critical error * * *\n\n");
	}
	
	push_text("The core was stopped unexpectedly!\nReason: ");
	push_text(msg);
	push_char('\n');
	
	push_text("LAST REGISTERS:\nEAX: ");
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
	
	if (!sys_args[1]){
		push_text("\n\nSystem halted. Please report this error.\n");
	
		while(1) asm volatile("hlt");
	}
	else {
		asm volatile("cli");
		push_text("\n\nAttempting to recover the system...\n");
		
		outb(0x21, inb(0x21) & ~(1 << 1));
		
		idt_ini();

		push_text("\nSystem recovered. You can try to continue working.\n");
		push_text("WARNING: The system may be unstable! Reset is recommended.");
		
		RESCUE_CONSOLE(msg, reg);
		
	}
}

void RESCUE_CONSOLE(const char* msg, Registers* reg){
	
	extern char input_buf_exec[BUFFER_SIZE];
	extern int input_mode;
	extern int input_exec_ready;
	extern unsigned int buf_id;
	
	setmemory(input_buf_exec, 0, sizeof(input_buf_exec));
				
	input_mode = 1;
	input_exec_ready = 0;
	buf_id = 0;
	input_buf_exec[0] = '\0';
	
	idt_ini();
	while (1){
		
		push_text("\nMDRESCUE :: ");
				
		pic_remap();
				
		while (!input_exec_ready){
			asm volatile("hlt");
		}
		input_exec_ready = 0;
		
		if (input_buf_exec[0] == 'l'){
			check_comm("logoff");
		}
		else if (input_buf_exec[0] == 'r'){
			check_comm("rest");
		}
		else if (input_buf_exec[0] == 't'){
			check_comm("time --hms");
			check_comm("time --dmy");
		}
		else if (input_buf_exec[0] == 'h'){
			push_text("\n  l - logoff; t - time; r - reset;\n  h - help; c - clear screen; e - error");
		}
		else if (input_buf_exec[0] == 'c'){
			cls();
		}
		else if (input_buf_exec[0] == 'e'){
			push_text("\nReason: ");
			push_text(msg);
			push_char('\n');
			
			push_text("Registers:\nEAX: ");
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
		}
	}
}