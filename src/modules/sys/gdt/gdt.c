#include "gdt.h"

extern struct gdt_entry gdt[6];
extern struct gdt_ptr gp;
extern struct tss_entry tss;

void gdt_set_gate(int number, unsigned long base, unsigned long limit, unsigned char access, unsigned char gran){
	gdt[number].base_low = base & 0xFFFF;
	gdt[number].base_mid = (base >> 16) & 0xFF;
	gdt[number].base_high = (base >> 24) & 0xFF;
	gdt[number].limit_low = (limit & 0xFFFF);
	gdt[number].granularity = ((limit >> 16) & 0x0F);
	gdt[number].granularity |= (gran & 0xF0);
	gdt[number].access = access;
}

void gdt_ini(){
	gp.limit = (sizeof(struct gdt_entry) * 6) - 1;
	gp.base = (unsigned int)&gdt;
	
	gdt_set_gate(0, 0, 0, 0, 0);
	
	gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
	
	gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);
	
	//gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);
	//gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);
	
	gdt_set_gate(5, (unsigned int)&tss, sizeof(tss), 0x89, 0x40);
	
	gdt_flush((unsigned int)&gp);
}

void tss_ini(){
	tss.esp0 = 0x00100000;
	tss.ss0 = 0x10;
	setmemory(&tss, 0, sizeof(tss));
	tss.iomap_base = sizeof(tss);
}

void gdt_set_tss(int number, unsigned int base, unsigned int limit){
	gdt[number].base_low = base & 0xFFFF;
	gdt[number].base_mid = (base >> 16) & 0xFF;
	gdt[number].base_high = (base >> 24) & 0xFF;
	gdt[number].limit_low = limit & 0xFFFF;
	gdt[number].granularity = (limit >> 16) & 0x0F;
	gdt[number].access = 0x89;
}