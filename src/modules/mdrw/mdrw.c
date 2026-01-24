#include "mdrw.h"

extern unsigned int cur_x;
extern unsigned int cur_y;

extern int input_mode;
extern int input_exec_ready;
extern int hlt_input_mode;

extern int mdrw_mode;
extern int mdrw_run;
extern int mdrw_need_draw;

extern char input_buf[BUFFER_SIZE];
extern char input_buf_exec[BUFFER_SIZE];
extern unsigned int buf_id;

extern void check_comm(const char* comm);

extern KeyBoardState kbs;
extern MDFS mdfs;

extern EventQueue GlobalEvQ;

void mdrw_handler(){
	asm("cli");
	
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
		if (ev_q_add(EVENT_KEY_PRESS, scancode)){
			outb(0x20, 0x20);
			outb(0xA0, 0x20);
			asm volatile("sti");
			return;
		}
	}
	
	outb(0x20, 0x20);
	outb(0xA0, 0x20);
	asm volatile("sti");
}

void draw_ed(Mdrw *text_ed){
	
	cur_x = 0; cur_y = 0;
	cur_move();
	push_text("MD READ & WRITE -> [");
	push_text(text_ed->filename);
	push_text("]");
	
	while (text_ed->cur_ye >= text_ed->scroll_offset + 20){
		text_ed->scroll_offset++;
	}
	while (text_ed->cur_ye < text_ed->scroll_offset) {
		text_ed->scroll_offset--;
	}
	
	if (text_ed->scroll_offset > text_ed->total_lines - 20){
		text_ed->scroll_offset = text_ed->total_lines - 20;
	}
	if (text_ed->scroll_offset < 0){
		text_ed->scroll_offset = 0;
	}
	
	if (text_ed->modified) push_text(" *");
    
	for (int i = 0; i < 20 && (i + text_ed->scroll_offset) < text_ed->total_lines; i++) {
		
		cur_x = 0;
		cur_y = i + 1;
		cur_move();
		
		if ((i + text_ed->scroll_offset) == text_ed->cur_ye) {
			push_text(">");
		} else {
			push_text(" ");
		}
				
		push_text(text_ed->lines[i + text_ed->scroll_offset]);
	}
    
	cur_x = 0; 
	cur_y = 21;
	cur_move();
	push_text("F1:Save  F2:Open  F5:Quit");

	cur_x = 0; cur_y = 22;
	cur_move();
		
	char x[3];
	char y[3];
	char t[3];
		
	digtostr(text_ed->cur_ye + 1, y);
	digtostr(text_ed->cur_xe + 1, x);
	digtostr(text_ed->total_lines, t);
		
	push_text("Line:");
	push_text(y);
	push_text(" ; Col:");
	push_text(x);
	push_text(" ; Total:");
	push_text(t);
}

void mdrw_load_file(const char* filename, Mdrw *text_ed){
    for (int i = 0; i < 51; i++) {
        text_ed->lines[i][0] = '\0';
    }
    text_ed->total_lines = 0;
    
    char file_buffer[4080];
    setmemory(file_buffer, 0, sizeof(file_buffer));
    
    for (int i = 0; i < MAX_FILES; i++) {
        if (mdfs.files[i].used && streq(mdfs.files[i].name, filename) == 0) {
            int file_size = file_read(filename, file_buffer, 4080 - 1, 0);
            
            if (file_size <= 0) {
                text_ed->lines[0][0] = '\0';
                text_ed->total_lines = 1;
                return;
            }
            
            file_buffer[file_size] = '\0';
            
            int line_num = 0;
            int line_start = 0;
            
            for (int pos = 0; pos < file_size && line_num < 51; pos++) {
                if (file_buffer[pos] == '\n' || pos == file_size - 1) {
                    int line_end = (file_buffer[pos] == '\n') ? pos : pos + 1;
                    int line_len = line_end - line_start;
                    
                    if (line_len > 79) line_len = 79;
                    
                    if (line_len > 0) {
                        copymemory(text_ed->lines[line_num], 
                                  file_buffer + line_start, 
                                  line_len);
                    }
                    text_ed->lines[line_num][line_len] = '\0';
                    
                    line_num++;
                    line_start = pos + 1;
                }
            }
            
            text_ed->total_lines = (line_num > 0) ? line_num : 1;
            
            if (file_size > 0 && file_buffer[file_size - 1] == '\n') {
                text_ed->lines[text_ed->total_lines][0] = '\0';
                text_ed->total_lines++;
            }
            
            return;
        }
    }
    
    text_ed->lines[0][0] = '\0';
    text_ed->total_lines = 1;
    return;
}

void mdrw_save_file(const char* filename, Mdrw *text_ed){
	char file_buffer[4080];
	setmemory(file_buffer, 0, sizeof(file_buffer));
	
    int buffer_pos = 0;
	
	char end_char[2];
	
	end_char[0] = file_buffer[strsz(file_buffer) - 1];
	end_char[1] = '\0';
    
    for (int i = 0; i < text_ed->total_lines; i++) {
        int line_len = strsz(text_ed->lines[i]);
		
        for (int j = 0; j < line_len; j++) {
            file_buffer[buffer_pos++] = text_ed->lines[i][j];
        }
		
        if (i < text_ed->total_lines - 1) {
            file_buffer[buffer_pos++] = '\n';
        }
    }
    file_buffer[buffer_pos] = '\0';

    file_wr(filename, file_buffer, buffer_pos);
	file_add_data(filename, end_char, strsz(end_char));
	
	push_text("\nData wrote in file.");
	text_ed->modified = 0;
	mdrw_need_draw = 1;
}

void mdrw_input_dialog(char *new_filename, Mdrw *text_ed, const char *text){
	INPUT:
		input_mode = 1; //kostyl for easily work ;)
		input_exec_ready = 0;
		buf_id = 0;
		int i = 0;
				
		char filename[MAX_FILENAME_LEN];
		input_buf[0] = '\0';
				
		setmemory(input_buf, 0, sizeof(input_buf));
		setmemory(input_buf_exec, 0, sizeof(input_buf));
				
		idt_ini();
		push_text(text);
		pic_remap();
				
		while (!input_exec_ready){
			asm volatile("hlt");
		}
		input_exec_ready = 0;
		input_mode = 0;
				
		for (char *p = input_buf_exec; *p != '\0'; p++){
			filename[i++] = *p;
		}
		
		filename[i] = '\0';
		push_char('\n');
		
		for (int i = 0; i < MAX_FILES; i++){
			if (mdfs.files[i].used && (streq(mdfs.files[i].name, filename) == 0)) {
				strcopy(new_filename, filename);
				return;
			}
		}
		
	push_text("Error: file not found!\n");
	goto INPUT;
}