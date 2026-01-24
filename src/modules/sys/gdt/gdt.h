#ifndef GDT_H
#define GDT_H

#include "../../sys/mdfs/mdfs.h"

// gdt - init and need structs

struct gdt_entry{
	unsigned short limit_low;
	unsigned short base_low;
	unsigned char base_mid;
	unsigned char access;
	unsigned char granularity;
	unsigned char base_high;
} __attribute__((packed));

struct gdt_ptr {
	unsigned short limit;
	unsigned int base;
} __attribute__((packed));

// tss entry for work system in protected mode
struct tss_entry {
	unsigned int prev_tss;
	unsigned int esp0;
	unsigned int ss0;
	unsigned int esp1;
	unsigned int ss1;
	unsigned int esp2;
	unsigned int ss2;
	unsigned int cr3;
	unsigned int eip;
	unsigned int eflags;
	unsigned int eax;
	unsigned int ebx;
	unsigned int edx;
	unsigned int ecx;
	unsigned int esp;
	unsigned int ebp;
	unsigned int esi;
	unsigned int edi;
	unsigned int es;
	unsigned int cs;
	unsigned int ss;
	unsigned int ds;
	unsigned int fs;
	unsigned int gs;
	unsigned int ldt;
	unsigned short trap;
	unsigned short iomap_base;
} __attribute__((packed));

void gdt_ini();
void gdt_set_gate(int number, unsigned long base, unsigned long limit, unsigned char access, unsigned char gran);
extern void gdt_flush(unsigned int);

void tss_ini();
void gdt_set_tss(int number, unsigned int base, unsigned int limit);

#endif