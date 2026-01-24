#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "../../drivers/io/io.h"
#include "../../drivers/vga/vga.h"
#include "../../mdstr/mdstr.h"
#include "../../time/time.h"
#include "../../drivers/ide/ide.h"
#include "../../sys/mdfs/mdfs.h"
#include "../../sys/event_queue/event_queue.h"

//define for keyboard port
#define KEYBOARD_PORT 0x60

//max size input buffer
#define BUFFER_SIZE 256

// F symbols ☻
#define F1 0x3B
#define F2 0x3C
#define F3 0x3D
#define F4 0x3E
#define F5 0x3F
#define F6 0x40
#define F7 0x41
#define F8 0x42
#define F9 0x43
#define F10 0x44
#define F11 0x57
#define F12 0x58

// Struct for easily work with keyboard states
typedef struct {
	unsigned char lshift : 1;
	unsigned char rshift : 1;
	unsigned char lctrl : 1;
	unsigned char rctrl : 1;
	unsigned char lalt : 1;
	unsigned char ralt : 1;
	unsigned char capslc : 1;
} KeyBoardState;

//Keyboard scancodes for work with keyboard
static const char scancodes[128] = {
	0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*',
    0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-',
    '4', '5', '6', '+', '1', '2', '3', '0', '.', 0, 0, 0, 0, 0
};
//And scancodes for pressed 'shift'
static const char scancodes_sh[128] = {
	0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
	'\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
	0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~',
	0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*',
	0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-',
	'4', '5', '6', '+', '1', '2', '3', '0', '.', 0, 0, 0, 0, 0
};

void pic_remap();
void keyboard_handler();
void emul_key_press(unsigned char key);

#endif