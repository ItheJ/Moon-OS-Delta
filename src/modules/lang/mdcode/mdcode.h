#ifndef MDCODE_H
#define MDCODE_H

#include "../../drivers/vga/vga.h"
#include "../../mdstr/mdstr.h"
#include "../../sys/mdfs/mdfs.h"
#include "../../drivers/keyboard/keyboard.h"
#include "../../sys/idt/idt.h"
#include "../../drivers/pc_sp/pcsp.h"
#include "../../time/time.h"
#include "../../sys/rand/rand.h"

// operations codes (for MDcode)
#define OP_PUSH_CHAR 0x00
#define OP_PUSH_TEXT 0x01
#define OP_INPUT 0x02
#define OP_PRINT 0x03
#define OP_EXIT 0x04
#define OP_SYS_EXEC 0x05
#define OP_CLEAR_BUF 0x06
#define OP_CALC 0x07
#define OP_GOTO 0x08
#define OP_EQ 0x09
#define OP_ABS 0x0A
#define OP_ABS_GOTO 0x0B
#define OP_JMP_IF_TRUE 0x0C
#define OP_JMP_IF_FALSE 0x0D
#define OP_DELAY 0x0E
#define OP_RAW_TIME 0x0F
#define OP_SET_REG 0x10
#define OP_LOAD_REG 0x11
#define OP_STR_EQ 0x12
#define OP_RANDOM 0x13

//struct "Executor" for execute code in *Execute mode*

typedef struct {
	unsigned char* program;
	unsigned int program_c;
	char work_stack[256];
	unsigned int stack_pointer;
	unsigned char registers[8][16];
	unsigned char flags;
} Executor;

// func for MDcode and command "exec"
void interpret_program(unsigned char* filename);

#endif