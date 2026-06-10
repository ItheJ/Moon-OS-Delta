#ifndef VGA_H
#define VGA_H

#include "../../drivers/io/io.h"
#include "../../mdstr/mdstr.h"

typedef unsigned int uintptr;
typedef int intptr;

void cur_move();
void scroll_down();

void del_back();

void push_char(char ch);
void push_text(const char * text);

void cls();
void csc(); //change screen color ;)

void push_hchar(unsigned char ch);
void push_h32(unsigned int num);

void themes();

void push_number(int number);
void push_format(const char *format, ...);

void push_hbytes(unsigned int value, int byte_len);

void push_float(float x);

#endif