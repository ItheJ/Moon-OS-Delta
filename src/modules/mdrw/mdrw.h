#ifndef MDRW_H
#define MDRW_H

#include "../drivers/io/io.h"
#include "../drivers/vga/vga.h"
#include "../mdstr/mdstr.h"
#include "../sys/mdfs/mdfs.h"
#include "../drivers/keyboard/keyboard.h"
#include "../sys/idt/idt.h"
#include "../sys/event_queue/event_queue.h"

// struct for text editor "MD READ & WRITE" (mdrw)
typedef struct {
	char lines[51][80];
	unsigned int total_lines;
	int cur_xe, cur_ye;
	int scroll_offset;
	char filename[MAX_FILENAME_LEN];
	unsigned short modified;
} Mdrw;

void draw_ed(Mdrw *text_ed);
void mdrw_load_file(const char* filename, Mdrw *editor);
void mdrw_save_file(const char* filename, Mdrw *editor);
void mdrw_input_dialog(char *new_filename, Mdrw *editor, const char *text);
void mdrw_handler();

#endif