#ifndef COMDUMMY_H
#define COMDUMMY_H

#define MAX_DUMMY 50
#define MAX_DUMMY_NAME 32
#define MAX_DUMMY_SIZE 128

struct ComDummy{
	char name[MAX_DUMMY_NAME];
    char value[MAX_DUMMY_SIZE];
	unsigned char used;
};

static struct ComDummy comdummies[MAX_DUMMY];
static unsigned short comdummy_count;
static unsigned short is_comdummy = 0;

#endif