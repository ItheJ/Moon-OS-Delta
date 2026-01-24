#include "turn_off.h"

extern int hlt_input_mode;

void logoff(){
	
	outw(0x604, 0x2000);
	
	outw(0xB004, 0x2000);
	
	push_text("\nSystem halted. Now you can turn off the computer.");
	asm volatile("cli");
	while(1){
		asm volatile("hlt");
	}
}

void rest(){
	push_text("\nSystem is rebooting...");
	
	while(inb(0x64) & 0x02);
	outb(0x64, 0xFE);
	
	asm volatile ("cli");
	while(1){
		asm volatile ("hlt");
	}
}

void hltmode(){
	push_text("\nNow system entering sleep mode. Press ESC to wake up...\n");
	
	asm volatile("sti");
	
	unsigned char scancode = 0;
	hlt_input_mode = 1;
	
	idt_ini();
	pic_remap();
	while(hlt_input_mode){
		asm volatile ("hlt");
	}
	
	push_text("System waking up...");
}