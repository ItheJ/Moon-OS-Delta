#include "vga.h"

extern volatile unsigned short *video_mem;
extern unsigned int cur_x;
extern unsigned int cur_y;
extern unsigned int bg_col;
extern unsigned int text_col;

void cur_move(){
	unsigned short pos = cur_y * 80 + cur_x;
	
	outb(0x3D4, 0x0F);
	outb(0x3D5, (unsigned char)(pos & 0xFF));
	
	outb(0x3D4, 0x0E);
	outb(0x3D5, (unsigned char)((pos >> 8) & 0xFF));
}

void scroll_down(){
	for (int y = 1; y < 25; y++){
		for (int x = 0; x < 80; x++){
			video_mem[(y - 1) * 80 + x] = video_mem[y * 80 + x];
		}
	}
	
	for ( int x = 0; x < 80; x++){
		video_mem[24 * 80 + x] = ((bg_col * 16 + text_col) << 8) | ' ';
	}
}

void del_back(){
	if (cur_x > 0){
		cur_x--;
	} else if (cur_y > 0){
		cur_y--;
		cur_x = 79;
	}
	
	video_mem[cur_y * 80 + cur_x] = ((bg_col * 16 + text_col) << 8) | ' ';
	cur_move();
}

void push_char(char ch){
	if (ch == '\n'){
		cur_x = 0;
		cur_y++;
	} else {
		video_mem[cur_y * 80 + cur_x] = ((bg_col * 16 + text_col) << 8) | ch;
		cur_x++;
		if (cur_x > 80){
			cur_x = 0;
			cur_y++;
		}
	}
	
	if (cur_y >= 25){
		scroll_down();
		cur_y = 24;
	}
	cur_move();
}

void push_text(const char *text){
	while (*text){
		push_char(*text++);
	}
}

void cls(){
	volatile char *video = (volatile char *)0xB8000;
	for (int i = 0; i < 80 * 25 * 2; i += 2){
		video[i] = ' ';
		video[i+1] = (bg_col * 16 + text_col);
	}
	cur_x = 0;
	cur_y = 0;
	cur_move();
}

void csc(){
	volatile char *video = (volatile char *)0xB8000;
	for (int i = 0; i < 80 * 25 * 2; i += 2){
		video[i+1] = (bg_col * 16 + text_col);
	}
}

void push_hchar(unsigned char ch){
	if (ch < 10) {
        ch += '0';
    } else {
        ch += 'A' - 10;
    }
	
	push_char(ch);
}

void push_h32(unsigned int num) {
    push_hchar((num >> 28) & 0xF);
    push_hchar((num >> 24) & 0xF);
    push_hchar((num >> 20) & 0xF);
    push_hchar((num >> 16) & 0xF);

    push_hchar((num >> 12) & 0xF);
    push_hchar((num >> 8) & 0xF);
    push_hchar((num >> 4) & 0xF);
    push_hchar(num & 0xF);
}

void themes(){
		
	push_text("\nThemes:\n  \"default [0F]\"    \"snowy [70]\"    \"azure lake [1B]\"\n  \"Something disturbing [4F]\"    \"Glade [2E]\"    \"Current [");
	
	push_hchar(((bg_col * 16 + text_col) >> 4) & 0xF);
	push_hchar((bg_col * 16 + text_col) & 0xF);
	
	push_text("]\"");
}