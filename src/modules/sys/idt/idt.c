#include "idt.h"

extern int df_handler_entry(void);
extern int de_handler_entry(void);
extern int inv_handler_entry(void);
extern int pf_handler_entry(void);
extern int gp_handler_entry(void);

extern int keyboard_handler_entry();

extern struct IDTent idt[256];

void set_idt_ent(unsigned char num, unsigned int handler){
	idt[num].offset_low = handler & 0xFFFF;
	idt[num].selector = 0x08;
	idt[num].zero = 0;
	idt[num].type_attr = 0x8E;
	idt[num].offset_high = (handler >> 16) & 0xFFFF;
}
void idt_ini() {
	set_idt_ent(0x08, (unsigned int)df_handler_entry); //Double fault
	set_idt_ent(0x00, (unsigned int)de_handler_entry); //Divide Error
	set_idt_ent(0x06, (unsigned int)inv_handler_entry); //Invalid Opcode
	set_idt_ent(0x0D, (unsigned int)gp_handler_entry); // General Protection fault
	set_idt_ent(0x0E, (unsigned int)pf_handler_entry); //Page fault 
	
	set_idt_ent(0x21, (unsigned int)keyboard_handler_entry); // keyboard
	
	struct {
		unsigned short limit;
		unsigned int base;
	} __attribute__((packed)) idtr = {sizeof(idt) -1, (unsigned int)idt};
	
	asm volatile("lidt %0" : : "m"(idtr));
	
	asm volatile("sti");
}
