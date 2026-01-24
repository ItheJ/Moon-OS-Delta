#include "screen.h"

unsigned short get_vde() {
	outb(0x3D4, 0x11);
	return (inb(0x3D5) + 1);
}

unsigned short get_hde() {
	outb(0x3D4, 0x01);
	return (inb(0x3D5) + 1);
}