#include "rand.h"

unsigned int get_val(void *addr){
	return *(unsigned int*)addr;
}

unsigned int seeding_rnd(unsigned int seed, unsigned int limit){
	
	seed ^= get_val(ADDR1) ^ get_val(ADDR2);
    seed = (seed << 13) | (seed >> 19);
    
    seed ^= get_unix_t();
    seed ^= get_val(ADDR3);

    seed *= 1409;
    seed += 50331;
    seed ^= seed >> 7;
    seed ^= seed << 3;
	
	if (limit > 0){
		seed = seed % (limit + 1);
	}
	else {
		seed % 0xFFFFFFFF;
	}
	
    return seed;
}