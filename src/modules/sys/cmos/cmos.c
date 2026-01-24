#include "cmos.h"

unsigned long long get_memory_size(){
	unsigned long long total;
	total = read_cmos(0x30) << 8 | read_cmos(0x31);
	return total;
}

unsigned char read_cmos(unsigned char registr){
	outb(0x70, registr);
	return inb(0x71);
}

void write_cmos(unsigned char addr, unsigned char value){
	outb(0x70, addr);
	outb(0x71, value);
}

unsigned char is_upd(){
	return read_cmos(0x0A) & 0x80;
}

unsigned char read_cmos_s(unsigned char registr){
	while (is_upd());
	return read_cmos(registr);
}

unsigned char bcd_to_bin(unsigned char bcd){
	return (bcd & 0x0F) + ((bcd >> 4) * 10);
}

unsigned char bin_to_bcd(unsigned char bin){
	return ((bin / 10) << 4) | (bin % 10);
}
