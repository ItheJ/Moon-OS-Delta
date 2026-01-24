#ifndef CPU_H
#define CPU_H

int cpuid_supported();
void get_cpuid_info(char vendor[13], unsigned int byte);
void get_cpuid_brand(char brand[49]);
void get_cpuid_features(unsigned char *family, unsigned char *model, unsigned int *ecxf, unsigned int *edxf);

#endif