#ifndef RAND_H
#define RAND_H

#include "../../time/time.h"

#define ADDR1 (unsigned int *)0x4E18 // random addr btw
#define ADDR2 (unsigned int *)0x8EB4 // random addr btw x2
#define ADDR3 (unsigned int *)0xA4EE // random addr btw x3

unsigned int get_val(void *addr);
unsigned int seeding_rnd(unsigned int seed, unsigned int limit);

#endif