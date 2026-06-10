#ifndef MDFS_H
#define MDFS_H

//defines for RAM MDFS
#define MAX_FILES 256
#define MAX_FILENAME_LEN 32
#define MAX_FILE_SIZE 4096

#include "../../drivers/vga/vga.h"
#include "../../mdstr/mdstr.h"

// File Entry - name, size, char and used - inited in system or not

typedef struct {
	char name[MAX_FILENAME_LEN];
	unsigned int size;
	unsigned char data[MAX_FILE_SIZE];
	unsigned char used;
} FileEntry;

//RAM MDFS - struct for init the ram system

typedef struct {
	FileEntry files[MAX_FILES];
} MDFS;

void mdfs_ini();

//function prototypes for work with files in RAM

int file_cr(const char* filename);
int file_wr(const char* filename, const char * data, unsigned int size);
int file_add_data(const char* filename, const char * data, unsigned int size);
int file_read(const char *filename, char * buffer, unsigned int buffer_size, unsigned int offset);
int file_del(const char *filename);
int file_rnm(const char* old_filename, const char* new_filename);
int files_er();
void files_list();

void *setmemory(void *ptr, int value, unsigned int number);
void *memset(void *s, int c, unsigned int n);
void *copymemory(void *dest, const void *src, unsigned int number);
int memoryeq(const void *s1, const void *s2, unsigned int number);

#endif