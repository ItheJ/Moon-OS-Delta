#ifndef TURNOFF_H
#define TURNOFF_H

#include "../../drivers/io/io.h"
#include "../../drivers/vga/vga.h"
#include "../../drivers/keyboard/keyboard.h"
#include "../../sys/idt/idt.h"

void logoff();
void rest();
void hltmode();

#endif