#include "keyboard.h"

extern int input_mode;
extern int input_exec_ready;
extern int hlt_input_mode;

extern char input_buf[BUFFER_SIZE];
extern char input_buf_exec[BUFFER_SIZE];
extern unsigned int buf_id;

extern int mdrw_run;

extern KeyBoardState kbs;

extern void check_comm(const char* comm);

void pic_remap(){
	outb(0x20, 0x11);
	outb(0x21, 0x20);
	outb(0x21, 0x04);
	outb(0x21, 0x01);
	
	outb(0xA0, 0x11);
	outb(0xA1, 0x28);
	outb(0xA1, 0x02);
	outb(0xA1, 0x01);

	outb(0x21, 0xFD);
	outb(0xA1, 0xFF);
}

void keyboard_handler(){
	asm volatile("cli");
	
	unsigned char scancode = inb(KEYBOARD_PORT);
	
	unsigned char relflag = scancode & 0x80;
	unsigned char key_code = scancode & 0x7F;
	
	if (!relflag) {
		switch(key_code) {
			case 0x2A:
				kbs.lshift = 1;
				break;
			case 0x36:
				kbs.rshift = 1;
				break;
			case 0x1D:
				kbs.lctrl = 1;
				break;
			case 0x38:
				kbs.lalt = 1;
				break;
			case 0x3A:
				kbs.capslc = !kbs.capslc;
				break;
		}
	}
	else {
		switch (key_code) {
			case 0x2A:
				kbs.lshift = 0;
				break;
			case 0x36:
				kbs.rshift = 0;
				break;
			case 0x1D:
				kbs.lctrl = 0;
				break;
			case 0x38:
				kbs.lalt = 0;
				break;
		}
	}
	
	if (!relflag){
		unsigned char sh_active = kbs.lshift || kbs.rshift;
		unsigned char use_sh_tabl = (sh_active || kbs.capslc);
	
		char ascii = 0;
		if (key_code < 128){
			ascii = use_sh_tabl ?
				scancodes_sh[key_code] :
				scancodes[key_code];
		}
	
		switch(key_code){
			case 0x0E:
				if(buf_id > 0){
					buf_id--;
					del_back();
				}
				break;
			case 0x1C:
				if (!hlt_input_mode){
					
					if (input_mode){
						input_buf[buf_id] = '\0';
						int i = 0;
						
						while (input_buf[i] == ' '){
							input_buf[i] = '\0';
							i++;
						}
						if (input_buf[0] == '\0' || buf_id == 0){
							input_buf[0] = '`';
						}
						buf_id = 0;
					
						strcopy(input_buf_exec, input_buf);
					
						input_exec_ready = 1;
					}
					else {
						input_buf[buf_id] = '\0';
						int i = 0;
						
						while (input_buf[i] == ' '){
							input_buf[i] = '\0';
							i++;
						}
						if (input_buf[0] == '\0' || buf_id == 0){
							input_buf[0] = '`';
							check_comm("PASS");
						}
						else{
							check_comm(input_buf);
						}
						buf_id = 0;
					}
				}
				break;
				
			case F11:
				if (!input_mode){
					input_buf[buf_id] = '\0';
					
					buf_id = 0;
					
					push_text("\n ");
					push_char(2); //☻ lip biting emoticon
					push_text(" lip biting emoticon");
					
					check_comm("PASS");
					
				}
				break;
			case F12:
				if (!input_mode){
					input_buf[buf_id] = '\0';
					
					buf_id = 0;
					
					push_text("\n ");
					push_char(3); //♥
					push_text(" I LOVE MOON OS DELTA!");
					
					check_comm("PASS");
					
				}
				break;
			case F5:
				if (!input_mode){
					input_buf[buf_id] = '\0';
					buf_id = 0;
					
					push_char('\n');
					push_date();
					push_char(' ');
					push_time();
					
					check_comm("PASS");
					
				}
				break;
			
			case F6:
				if (!input_mode){
					input_buf[buf_id] = '\0';
					buf_id = 0;
					
					lsdevs();
					
					check_comm("PASS");
					
				}
				break;
				
			case F7:
				if (!input_mode){
					input_buf[buf_id] = '\0';
					buf_id = 0;
					
					files_list();
					
					check_comm("PASS");
					
				}
				break;
			
			case F8:
				if (!input_mode){
					input_buf[buf_id] = '\0';
					buf_id = 0;
					
					themes();
					
					check_comm("PASS");
					
				}
				break;
			
			case 0x7D:
				if (!input_mode){
					input_buf[buf_id] = '\0';
					buf_id = 0;
					
					push_text("\n88");
					
					check_comm("PASS");
					
				}
				break;
			
			default:
				if (ascii == 27 && hlt_input_mode){
					hlt_input_mode = 0;
					buf_id = 0;
				}
				else{
					if (!hlt_input_mode){
						
						if (ascii != 0) {
						
							input_buf[buf_id++] = ascii;
							push_char(ascii);
						
							if (buf_id >= BUFFER_SIZE - 1){
								buf_id = BUFFER_SIZE - 1;
							}
					
						}
					}
				}
				break;
		}
		
	}
	
	outb(0x20, 0x20);
	outb(0xA0, 0x20);
	asm volatile("sti");
}

void emul_key_press(unsigned char key){
	outb(0x60, key); //push key code in keyboard data register
	outb(0x20, 0x21); // send a signal
	
	asm volatile (
		"mov $0x02, %%ah\n\t"
		"mov %0, %%dl\n\t"
		"int $0x21"
		:
		: "r"(key)
		: "ax", "dx"
	);	
}