#ifndef CMOS_H
#define CMOS_H

#include "../../drivers/io/io.h"

unsigned char read_cmos(unsigned char registr);
void write_cmos(unsigned char addr, unsigned char value);
unsigned char is_upd();
unsigned char read_cmos_s(unsigned char registr);
unsigned char bcd_to_bin(unsigned char bcd);
unsigned char bin_to_bcd(unsigned char bin);

unsigned long long get_memory_size();

#endif