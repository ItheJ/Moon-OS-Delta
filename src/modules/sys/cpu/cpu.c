#include "cpu.h"

int cpuid_supported(){
	unsigned int eflags;
	asm volatile(
		"pushfl\n\t"
		"popl %0\n\t"
		"movl %0, %%ecx\n\t"
		"xorl $0x200000, %0\n\t"
		"pushl %0\n\t"
		"popfl\n\t"
		"pushfl\n\t"
		"popl %0\n\t"
		: "=r" (eflags)
		:
		: "ecx"
	);
	return (eflags & 0x200000) != 0;
}

void get_cpuid_info(char vendor[13], unsigned int byte){
	unsigned int eax, ebx, ecx, edx;
	
	asm volatile(
		"cpuid"
		: "=b" (ebx), "=c"(ecx), "=d" (edx)
		: "a" (byte)
	);
	
	*(unsigned int*)&vendor[0] = ebx;
	*(unsigned int*)&vendor[4] = edx;
	*(unsigned int*)&vendor[8] = ecx;
	vendor[12] = '\0';
}

void get_cpuid_brand(char brand[49]){
	unsigned int eax, ebx, ecx, edx;
    unsigned int *brand_ptr = (unsigned int*)brand;
    
    eax = 0x80000002;
    asm volatile("cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(eax)
	);
	
    *brand_ptr++ = eax;
    *brand_ptr++ = ebx;
    *brand_ptr++ = ecx;
    *brand_ptr++ = edx;

    eax = 0x80000003;
    asm volatile("cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(eax)
	);
	
    *brand_ptr++ = eax;
    *brand_ptr++ = ebx;
    *brand_ptr++ = ecx;
    *brand_ptr++ = edx;

    eax = 0x80000004;
    asm volatile("cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(eax)
	);
	
    *brand_ptr++ = eax;
    *brand_ptr++ = ebx;
    *brand_ptr++ = ecx;
    *brand_ptr++ = edx;
}

void get_cpuid_features(unsigned char *family, unsigned char *model, unsigned int *ecxf, unsigned int *edxf){
	unsigned int eax, ebx, ecx, edx;
	asm volatile (
		"cpuid"
		: "=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx)
		: "a" (1)
	);
	
	*family = ((eax >> 8) & 0xF);
	*model = ((eax >> 4) & 0xF);
	*ecxf = ecx;
	*edxf = edx;
}
