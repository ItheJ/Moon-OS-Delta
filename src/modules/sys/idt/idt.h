#ifndef IDT_H
#define IDT_H

struct IDTent {
	unsigned short offset_low;
	unsigned short selector;
	unsigned char zero;
	unsigned char type_attr;
	unsigned short offset_high;
} __attribute__((packed));

void set_idt_ent(unsigned char num, unsigned int handler);
void idt_ini();

#endif