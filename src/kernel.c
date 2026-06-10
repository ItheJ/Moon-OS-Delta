// Moon OS Delta 1.2.2
// Author - 'Zondrobonie' Ivan, put copyright here ;)

//no more multiboot in C file :(
#include "./modules/multiboot_h/multiboot.h"

#include "./modules/drivers/pc_sp/pcsp.h"
#include "./modules/drivers/io/io.h"
#include "./modules/drivers/vga/vga.h"
#include "./modules/mdstr/mdstr.h"
#include "./modules/comdummy/comdummy.h"
#include "./modules/drivers/ide/ide.h"
#include "./modules/sys/mdfs/mdfs.h"
#include "./modules/sys/cmos/cmos.h"
#include "./modules/time/time.h"
#include "./modules/sys/cpu/cpu.h"
#include "./modules/drivers/keyboard/keyboard.h"
#include "./modules/sys/turn_off/turn_off.h"
#include "./modules/sys/idt/idt.h"
#include "./modules/mdrw/mdrw.h"
#include "./modules/sys/event_queue/event_queue.h"
#include "./modules/lang/mdcode/mdcode.h"
#include "./modules/sys/gdt/gdt.h"
#include "./modules/sys/screen/screen.h"

#include "./modules/drivers/pci/pci.h"
#include "./modules/sys/rand/rand.h"
#include "./modules/sys/mdmath/mdmath.h"

#define KERNEL_STACK_SIZE 8192

//for memory detecting
#define MEM_TYPE_AVAIABLE 1
#define MEM_TYPE_RESERV 2
#define MEM_TYPE_ACPI 3
#define MEM_TYPE_NVS 4

struct memory_map_entry {
	unsigned long long base_addr;
	unsigned long long lenght;
	unsigned int type;
	unsigned int extended_attrs;
} __attribute__((packed));

struct memory_info {
	unsigned int total; //in MB
	unsigned int available; //in MB
	unsigned int entries_count;
	struct memory_map_entry entries[32];
};

struct gdt_entry gdt[6];
struct gdt_ptr gp;

struct IDTent idt[256];
struct tss_entry tss;

unsigned char kernel_stack[KERNEL_STACK_SIZE] __attribute__((aligned(16)));
extern unsigned int mlt_inf;

//init vars...
volatile unsigned short *video_mem = (volatile unsigned short *)0xB8000; //get videomem address
MDFS mdfs;
PCI_bus pci_bus;

KeyBoardState kbs;
EventQueue GlobalEvQ;

char input_buf[BUFFER_SIZE];
char input_buf_exec[BUFFER_SIZE] = {'\0'};
unsigned int buf_id = 0;

int input_mode = 0;
int input_exec_ready = 0;
int hlt_input_mode = 0;

int mdrw_mode = 0;
int mdrw_run = 0;
int mdrw_need_draw = 1;

int debug_mode = 0;

unsigned int cur_x = 0;
unsigned int cur_y = 0;

unsigned int bg_col = 0x0;
unsigned int text_col = 0xF;

unsigned int timer_time = 0;
unsigned char is_timer_started = 0;

Ahci_dev ahci_devs[MAX_AHCI];

unsigned int seed = 122;

void check_comm(const char* comm);

void get_system_info();
void Mdrw_exec();

void krnl_run(void){
	cls();
	
	asm volatile("mov %0, %%esp" : : "r"(kernel_stack + KERNEL_STACK_SIZE));
	
	gdt_ini();
	tss_ini();
	idt_ini();
	
	outb(0x43, 0xB6); //PIT INIT
	
	gdt_set_tss(5, (unsigned int)&tss, sizeof(tss)-1);
	asm volatile("ltr %%ax" : : "a"(0x28));
	
	/*
	 __  __                    ___  ____       
	|  \/  | ___   ___  _ __  / _ \/ ___|   *   
	| |\/| |/ _ \ / _ \| '_ \| | | \___ \      *
	| |  | | (_) | (_) | | | | |_| |___) |     
	|_|  |_|\___/ \___/|_| |_|\___/|____/  *
	 ____       _ _          __  ____    ____    *
	|  _ \  ___| | |_ __ _  / | |___ \  |___ \     *
	| | | |/ _ \ | __/ _` | | |   __) |   __) |
	| |_| |  __/ | || (_| | | |_ / __/ _ / __/   *
	|____/ \___|_|\__\__,_| |_(_)_____(_)_____|     *
	
	*/
	//"=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n        *                *             *                 *            *\n    *           *     MOON OS DELTA             *                      \n         *             (   )              *           *             *  \n *          *                      *                     *        *    \n=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n\n"
	push_text(" __  __                    ___  ____       \n|  \\/  | ___   ___  _ __  / _ \\/ ___|   *   \n| |\\/| |/ _ \\ / _ \\| '_ \\| | | \\___ \\      *\n| |  | | (_) | (_) | | | | |_| |___) |     \n|_|  |_|\\___/ \\___/|_| |_|\\___/|____/  *\n ____       _ _          __  ____    ____    *\n|  _ \\  ___| | |_ __ _  / | |___ \\  |___ \\     *\n| | | |/ _ \\ | __/ _` | | |   __) |   __) |\n| |_| |  __/ | || (_| | | |_ / __/ _ / __/   *\n|____/ \\___|_|\\__\\__,_| |_(_)_____(_)_____|     *\n\n");
	devdetect();

	seed = seeding_rnd(seed, 0xFF);
	
	push_text("MoonOS:>> ");

	pic_remap();
	while (1) asm volatile("hlt");
}

void check_comm(const char* comm){
	
	char* args = strsep(comm, ' ');
	if (args) *args++ = '\0';
	
	for (unsigned short i = 0; i < MAX_DUMMY; i++) {    
		if (streq(comm, comdummies[i].name) == 0) {
			check_comm(comdummies[i].value);
			is_comdummy = 1;
		}
	}
	
	if (streq(comm, "help") == 0){
		if (args && *args){
			if (streq(args, "--standart") == 0) {
				push_text("\nAvailable commands:\n  help <--standart> <--filew> <--advanced> - print this message,\n  cls (clear) - CLear the Screen,\n  echo <text> - print text,\n  logoff - quit from system,\n  rest - reset the system,\n  abt - info the system and authors / credits,\n  mdver - version the system,\n  time <--hms> <--dmy> <--unixt> - print time or date,\n  mdrw - open MD READ&WRITE,\n  setcolor <color(0-F)><color(0-F)> <--default>,\n  timer <--start> <--stop> <--reset> - start, stop and reset user timer");
			}
			else if (streq(args, "--filew") == 0) {
				push_text("\nAvailable commands:\n  touch <filename> - create file,\n  wr <filename> <text> - write text in file,\n  rd <filename> - read file data,\n  del <filename> - delete file,\n  erase <filename> - erase all data in file,\n  ls - list all files,\n  add <filename> <text> - add text in file,\n  rnm <old filename> <new filename> - rename file,\n  copyf <filename1> <filename2> - copy data to <filename2>,\n  movf <filename1> <filename2> - move data to <filename2>,\n  rdhead <filename> <size> - read first <size> symbols,\n  rdtail <filename> <size> - read last <size> symbols,\n  chsymb <filename> <symbol1> <symbol2>  -  change all <symbol1> to <symbol2> in <filename>");
			}
			else if (streq(args, "--advanced") == 0){
				push_text("\nAvailable commands:\n  beep <frequency> <durations (ms)> - sound a signal,\n  ldide <dev> - load data from <dev> to RAM,\n  svide <dev> - save data from RAM,\n  lside - list of available device,\n  formtide <dev> - format <dev> and set MDFS on <dev>,\n  exec <filename> - execute file in MDcode,\n  comdummy <pointer>=<command> - create a custom command,\n  lscomdum - list of created comdummy,\n  chcomdum <pointer>=<command> - change command in comdummy,\n  chdbg - on/off debug mode,\n  timeset<flag (time/date/unixt)> <time (HH:MM:SS)/date (DD:MM:YYYY)/unixt (unix timestamp)> - set new time/date,\n  panic - load Moon Delta fatal kernel stop (advanced), \n  pci <--rescan> <--data> <--ahci> - read and work with PCI");
			}
			else {
				push_text("\nUsage: help <--standart> <--filew> <--advanced>");
			}
		} else {
			push_text("\nFor more commands print help <--standart> or help <--filew> or help <--advanced>");
		}
	}
	else if (streq(comm, "cls") == 0 || streq(comm, "clear") == 0) {
		cls();
	}
	else if (streq(comm, "echo") == 0) {
		push_char('\n');
		if (args && *args){
			while (*args) {
				if (*args == '\\' && *(args+1) == 'n') {
					push_char('\n');
					args += 2;
				}
				else if ((*args == '\\' && *(args+1) == 't') || *args == '\t'){
					push_text("    ");
					args += 2;
				}
				else {
					push_char(*args);
					args++;
				}
			}
			push_char('\n');
		} else {
			push_text("Usage: echo <text>");
		}
	}
	else if (streq(comm, "logoff") == 0) {
		push_text("\nLogging off... ");
		logoff();
	}
	else if (streq(comm, "rest") == 0) {
		push_text("\nHello! If you read this text you:\na - have computer that do not support the restart\nb - want to see this text in source code\nAnyway, just turn off and turn on your computer.");
		check_comm("PASS");
		rest();
	}
	else if (streq(comm, "abt") == 0) {
		push_text("\nABOUT:\n  Moon OS Delta\n  programmer and author - 'Zondrobonie' Ivan (a.k.a ItheJ)\n  for news: t.me/MoonOSDelta \n  Special thanks - BFG (chat), Imancat (for code for comdummy) and others...");
	}
	else if (streq(comm, "mdver") == 0) {
		get_system_info();
	}
	else if (streq(comm, "time") == 0) {
		push_char('\n');
		if (args && *args){
			if (streq(args, "--unixt") == 0) {
				push_unix_t();
			}
			else if (streq(args, "--hms") == 0) {
				push_time();
			}
			else if (streq(args, "--dmy") == 0) {
				push_date();
			}
		} else {
			push_text("\nUsage: time <--hms> <--dmy> <--unixt>");
		}
	}
	else if (streq(comm, "touch") == 0) {
		if (args && *args) {
			file_cr(args);
		}
		else {
			push_text("\nUsage: touch <filename>");
		}
	}
	else if (streq(comm, "wr") == 0) {
		char* args = comm + 3;
        
        char* filename_start = args;
        char* text_start = "";
        
        while (*args == ' ') args++;
        filename_start = args;
		
		char end_char[2];
		
        end_char[0] = filename_start[strsz(filename_start) - 1];
		end_char[1] = '\0';
		
		filename_start[strsz(filename_start) - 1] = '\0';
        
        while (*args != '\0' && *args != ' ') args++;
        
        if (*args == '\0') {
            push_text("\nUsage: wr <filename> <text>");
        }
        else{
			*args = '\0';
			args++;
        
			text_start = args;
        
			if (*text_start == '\'') {
				text_start++;
				char* end_quote = strsep(text_start, '\'');
				if (end_quote) {
					*end_quote = '\0'; 
				}
			}

			if (file_wr(filename_start, text_start, strsz(text_start)) == 0){
				file_add_data(filename_start, end_char, strsz(end_char));
				file_add_data(filename_start, " \n", strsz(" \n"));
				
				push_text("\nData wrote in file.");
			}
		}
    }
	else if (streq(comm, "rd") == 0) {
		if (args && *args) {
			char buffer[MAX_FILE_SIZE + 1];
			int bytes_read = file_read(args, buffer, MAX_FILE_SIZE, 0);
			
			if (bytes_read >= 0){
				buffer[bytes_read] = '\0';
				push_char('\n');
				push_text(buffer);
			}
			else{		
				push_text("\nError: file not found!");
			}
			
		}
		else {
			push_text("\nUsage: rd <filename>");
		}
	}
	else if (streq(comm, "del") == 0) {
		if (args && *args) {
			file_del(args);
		}
		else {
			push_text("\nUsage: del <filename>");
		}
	}
	else if (streq(comm, "erase") == 0) {
		if (args && *args) {
			files_er(args);
		}
		else {
			push_text("\nUsage: erase <filename>");
		}
		
	}
	else if (streq(comm, "ls") == 0) {
		files_list();
	}
	else if (streq(comm, "beep") == 0){
		unsigned int freq = 2000;
		unsigned int duration = 1000;
    
		if (args && *args) {

			char args_copy[64];
			strnumbercopy(args_copy, args, sizeof(args_copy)-1);
			args_copy[sizeof(args_copy)-1] = '\0';
			
			char* freq_str = strtok(args_copy, " ");
			char* dur_str = strtok((void *)0, " ");
        
			if (!strtodig(freq_str, &freq)) {
				push_text("\nError: invalid frequency format!");
			}
			
			else if (dur_str && !strtodig(dur_str, &duration)) {
				push_text("\nError: invalid duration format!");
			}
			
			else if (freq < 20 || freq > 20000 || duration < 10 || duration > 5000) {
				push_text("\nError: invalid parameters! Valid ranges: frequency 20-20000Hz, duration 10-5000ms");
			}
			else {
				char frq[6];
				char dur[5];
				
				beep(freq, duration);
				
				push_text("\nBeep at ");
				
				digtostr(freq, frq);
				digtostr(duration, dur);
				
				push_text(frq);
				push_text("Hz for ");
				push_text(dur);
				push_text(" ms");
			}
		}
		else {
			beep(freq, duration);
			push_text("\nBeep at 2000Hz for 1000 ms");
		}
	}
	else if (streq(comm, "ldide") == 0){
		if (args && *args) {
			if (input_mode){
				push_text("\nRun time warning! Command ldide blocked in execute mode!");
				return;
			}
			else {
				lddsk(args);
			}
		}
		else {
			push_text("\nUsage: ldide <dev>");
		}
	}
	else if (streq(comm, "svide") == 0){
		if (args && *args) {
			svdsk(args);
		}
		else {
			push_text("\nUsage: svide <dev>");
		}
	}
	else if (streq(comm, "lside") == 0){
		lsdevs();
	}
	else if (streq(comm, "formtide") == 0){
		if (args && *args) {
			mdfs_format(args);
		}
		else {
			push_text("\nUsage: formtide <dev>");
		}
	}
	else if (streq(comm, "add") == 0) {
		char* args = comm + 4;
        
        char* filename_start = args;
        char* text_start = "";
        
        while (*args == ' ') args++;
        filename_start = args;
		
		char end_char[2];
		
        end_char[0] = filename_start[strsz(filename_start) - 1];
		end_char[1] = '\0';
		
		filename_start[strsz(filename_start) - 1] = '\0';
        
        while (*args != '\0' && *args != ' ') args++;
        
        if (*args == '\0') {
            push_text("\nUsage: add <filename> <text>");
        }
        else{
			*args = '\0';
			args++;
        
			text_start = args;
        
			if (*text_start == '\'') {
				text_start++;
				char* end_quote = strsep(text_start, '\'');
				if (end_quote) {
					*end_quote = '\0'; 
				}
			}
			
			file_add_data(filename_start, text_start, strsz(text_start));
			file_add_data(filename_start, end_char, strsz(end_char));
			file_add_data(filename_start, " \n", strsz(" \n"));
			
			push_text("\nData added in file.");
		}
    }
	else if (streq(comm, "exec") == 0){
		char* args = comm + 5;
		if (args && *args) {
			if (strsz(args) < 5 || strnumbereq((args + (strsz(args) - 5)), ".mdxt", 5)){
				push_text("\nError: incorrect filename! Usage: <filename>.mdxt");
			}
			else{
				if (input_mode){
					push_text("\nRun time warning! Command exec blocked in execute mode!");
					return;
				}
				else{
					int i = 0;
					int is_find = 0;
				
					char normalized_name[MAX_FILENAME_LEN];
					strnumbercopy(normalized_name, args, MAX_FILENAME_LEN);
    
					char* end = normalized_name + strsz(normalized_name) - 1;
					while (end >= normalized_name && (*end == ' ' || *end == '\r' || *end == '\n')) {
						*end = '\0';
						end--;
					}
				
					for (i; i < MAX_FILES; i++) {
						if (mdfs.files[i].used && streq(mdfs.files[i].name, normalized_name) == 0) {
							is_find = 1;
							push_char('\n');
							interpret_program(mdfs.files[i].data);
							break;
						}
					}
				
					if (!is_find){
						push_text("\nFile not found!");
					}
				}
				
			}
		}
		else {
			push_text("\nUsage: exec <filename>");
		}
	}
	else if (streq(comm, "rnm") == 0){
		char* args = comm + 4;
        
        char* filename_start = args;
        char* fn = "";
        
        while (*args == ' ') args++;
        filename_start = args;
		
		char end_char[2];
		
        end_char[0] = filename_start[strsz(filename_start) - 1];
		end_char[1] = '\0';
		
		filename_start[strsz(filename_start) - 1] = '\0';
        
        while (*args != '\0' && *args != ' ') args++;
        
        if (*args == '\0') {
            push_text("\nUsage: rnm <old filename> <new filename>");
        }
        else{
			*args = '\0';
			args++;
        
			fn = args;
			char new_filename[MAX_FILENAME_LEN];
			int i = 0;
			
			for (i; i < strsz(fn); i++){
				new_filename[i] = fn[i];
			}
			new_filename[i++] = end_char[0];
			new_filename[i++] = end_char[1];
			
			file_rnm(filename_start, new_filename);
		}
	}
	else if (streq(comm, "hltmode") == 0){
		if (input_mode){
			push_text("\nRun time warning! Command hltmode blocked in execute mode!");
			return;
		}
		else {
			hltmode();
		}
	}
	else if (streq(comm, "comdummy") == 0) {  
		char* args = comm + 9;
		if (args && *args) {
			char* equal = strsep(args, '=');
			if (equal) {
				*equal = '\0';
				
				char* name = args;
				char* command = equal + 1;
				
				while (*command == ' ') command++;
				
				if (strsz(name) >= MAX_DUMMY_NAME) {
					push_text("\nError: too long name!");
				} 
				else if (strsz(command) >= MAX_DUMMY_SIZE) {
					push_text("\nError: too long data!");
				}
				else {
					short found = 0;
					for (short i = 0; i < MAX_DUMMY; i++) {
						if (streq(comdummies[i].name, name) == 0 && comdummies[i].used) {
							push_text("\nError: this comdummy was init before!");
							found = 1;
							break;
						}
					}
					if (!found){
						if (comdummy_count < MAX_DUMMY) {
							//list of reserved names (command names)
							if (streq(comdummies[comdummy_count].name, "help") == 0 || streq(comdummies[comdummy_count].name, "cls") == 0 || streq(comdummies[comdummy_count].name, "clear") == 0 || streq(comdummies[comdummy_count].name, "echo") == 0 || streq(comdummies[comdummy_count].name, "logoff") == 0 || streq(comdummies[comdummy_count].name, "rest") == 0 || streq(comdummies[comdummy_count].name, "abt") == 0 || streq(comdummies[comdummy_count].name, "mdver") == 0 || streq(comdummies[comdummy_count].name, "time") == 0 || streq(comdummies[comdummy_count].name, "touch") == 0 || streq(comdummies[comdummy_count].name, "wr") == 0 || streq(comdummies[comdummy_count].name, "rd") == 0 || streq(comdummies[comdummy_count].name, "del") == 0 || streq(comdummies[comdummy_count].name, "erase") == 0 || streq(comdummies[comdummy_count].name, "ls") == 0 || streq(comdummies[comdummy_count].name, "add") == 0 || streq(comdummies[comdummy_count].name, "rnm") == 0 || streq(comdummies[comdummy_count].name, "beep") == 0 || streq(comdummies[comdummy_count].name, "ldide") == 0 || streq(comdummies[comdummy_count].name, "svide") == 0 || streq(comdummies[comdummy_count].name, "lside") == 0 || streq(comdummies[comdummy_count].name, "formtide") == 0 || streq(comdummies[comdummy_count].name, "exec") == 0 || streq(comdummies[comdummy_count].name, "comdummy") == 0 || streq(comdummies[comdummy_count].name, "lscomdum") == 0 ||streq(comdummies[comdummy_count].name, "chcomdum") == 0 || streq(comdummies[comdummy_count].name, "hltmode") == 0 ||streq(comdummies[comdummy_count].name, "chdbg") == 0 || streq(comdummies[comdummy_count].name, "timeset") == 0 || streq(comdummies[comdummy_count].name, "mdrw") == 0 || streq(comdummies[comdummy_count].name, "copyf") == 0 || streq(comdummies[comdummy_count].name, "movf") == 0 || streq(comdummies[comdummy_count].name, "setcolor") == 0 || streq(comdummies[comdummy_count].name, "panic") == 0 || streq(comdummies[comdummy_count].name, "timer") == 0 || streq(comdummies[comdummy_count].name, "PASS") == 0){
								push_text("\nError: Used reserved word in name!");
								return;
							}
							
							strcopy(comdummies[comdummy_count].name, name);
							strcopy(comdummies[comdummy_count].value, command);
							comdummies[comdummy_count].used = 1;
							
							comdummy_count++;
							push_text("\nComdummy was created");
						} else {
							push_text("\nMax comdummy was created before");
						}
					}
				}
			} else {
				push_text("\nUsage: comdummy <pointer>=<command>");
			}
		}
	}
	else if (streq(comm, "lscomdum") == 0){
		if ( comdummy_count == 0) {
			push_text("\nCreated comdummy not found!");
		} else {
			push_text("\nFound comdummy:\n");
			for (int i = 0; i < MAX_DUMMY; i++) {
				if ( comdummies[i].used){
					push_text("  ");
					push_text(comdummies[i].name);
					push_text(" = ");
					push_text(comdummies[i].value);
					push_text("   ");
				}
			}
		}
	}
	else if (streq(comm, "chcomdum") == 0){
		char* args = comm + 9;
		if (args && *args) {
			char* equal = strsep(args, '=');
			if (equal) {
				*equal = '\0';
				
				char* name = args;
				char* command = equal + 1;
				
				while (*command == ' ') command++;
				
				if (strsz(name) >= MAX_DUMMY_NAME) {
					push_text("\nError: too long name!");
				} 
				else if (strsz(command) >= MAX_DUMMY_SIZE) {
					push_text("\nError: too long data!");
				}
				else {
					short found = 0;
					for (short i = 0; i < MAX_DUMMY; i++) {
						if (streq(comdummies[i].name, name) == 0 && comdummies[i].used) {
							strcopy(comdummies[i].value, command);
							push_text("\nComdummy data was updated!");
							found = 1;
							break;
						}
					}
					if (found == 0) {
						push_text("\nComdummy not found!");
					}
				}
			}
		}
	}
	else if (streq(comm, "chdbg") == 0){
		debug_mode = (!debug_mode);
		char db[2];
		digtostr(debug_mode, db);
		
		push_text("\nChanged [DEBUG MODE] to '");
		push_text(db);
		push_text("'!\n");
		
	}
	else if (streq(comm, "timeset") == 0){
		if (args && *args) {

			char args_copy[64];
			strnumbercopy(args_copy, args, sizeof(args_copy)-1);
			args_copy[sizeof(args_copy)-1] = '\0';
			
			char* flag_str = strtok(args_copy, " ");
			char* time_str = strtok((void *)0, " ");

			if (streq(flag_str, "time") == 0){
				char * hour = strtok(time_str, ":");
				char * minute = strtok((void *)0, ":");
				char * second = strtok((void *)0, ":");
				
				RTC_Time *rtc;
				Time prev_time = get_t();
				
				strtodig(second, (int *)&rtc->second);
				strtodig(minute, (int *)&rtc->minute);
				strtodig(hour, (int *)&rtc->hour);
				
				rtc->day = prev_time.day;
				rtc->month = prev_time.month;
				rtc->year = (prev_time.year - 2000);
				
				if (rtc->second > 59 || rtc->minute > 59 || rtc->hour >23){
					push_text("\nError! Invalid time!");
				}
				else {
					set_rtc_time((RTC_Time*)rtc);
					push_text("\nNew time setted!");
				}
			}
			else if (streq(flag_str, "date") == 0){
				char * day = strtok(time_str, "|");
				char * month = strtok((void *)0, "|");
				char * year = strtok((void *)0, "|");
				
				unsigned int year_n = 0;
				
				RTC_Time *rtc;
				Time prev_time = get_t();
				
				strtodig(day, (int *)&rtc->day);
				strtodig(month, (int *)&rtc->month);
				strtodig(year, &year_n);
				
				rtc->second = prev_time.second;
				rtc->minute = prev_time.minute;
				rtc->hour = prev_time.hour;
				
				if (rtc->day < 1 || rtc->day > 31 || rtc->month < 1 || rtc->month > 12 || year_n < 2000 || year_n > 2099){
					push_text("\nError! Invalid date!");
				}
				else {
					rtc->year = (year_n - 2000);
					
					set_rtc_time((RTC_Time*)rtc);
					push_text("\nNew date setted!");
				}
			}
			else if (streq(flag_str, "unixt") == 0) {
				unsigned int timestamp;
				strtoudig(time_str, &timestamp);
				
				int second, minute, hour, day, month, year;
				unixt_to_date(timestamp, &second, &minute, &hour, &day, &month, &year);
				
				RTC_Time *rtc;
				
				rtc->second = second;
				rtc->minute = minute;
				rtc->hour = hour;
				rtc->day = day;
				rtc->month = month;
				rtc->year = (year - 2000);
				
				set_rtc_time((RTC_Time*)rtc);
				push_text("\nUnixt timestamp loaded!");
			}
			
		}
		else {
			push_text("\nUsage: timeset <flag (time/date/unixt)> <time (HH:MM:SS)/date (DD:MM:YYYY)/unixt (unix timestamp)>");
		}
	}
	else if (streq(comm, "mdrw") == 0){
		if (input_mode){
			push_text("\nRun time warning! Command mdrw blocked in execute mode!");
			return;
		}
		else {
			Mdrw_exec();
			//cls();
		}
	}
	else if (streq(comm, "copyf") == 0){
		char* args = comm + 6;
        
        char* filename_start = args;
        char* fn = "";
        
        while (*args == ' ') args++;
        filename_start = args;
		
		char end_char[2];
		
		end_char[0] = filename_start[strsz(filename_start) - 1];
		end_char[1] = '\0';
		
		filename_start[strsz(filename_start) - 1] = '\0';
        
        while (*args != '\0' && *args != ' ') args++;
        
        if (*args == '\0') {
            push_text("\nUsage: copyf <filename1> <filename2>");
        }
        else{
			*args = '\0';
			args++;
			
			fn = args;
			char filename2[MAX_FILENAME_LEN];
			int i = 0;
			
			for (i; i < strsz(fn); i++){
				filename2[i] = fn[i];
			}
			
			filename2[i++] = end_char[0];
			filename2[i++] = end_char[1];
			
			char buffer[MAX_FILE_SIZE + 1];
			int bytes_read = file_read(filename_start, buffer, MAX_FILE_SIZE, 0);
			
			char ech[2];
			
			ech[0] = buffer[MAX_FILE_SIZE];
			ech[1] = '\0';
			
			if( bytes_read >= 0){
				if (file_wr(filename2, buffer, strsz(buffer)) == 0){
					file_add_data(filename2, ech, strsz(ech));
					
					push_text("\nData copied and moved to other file.");
				}
			}
			else{		
				push_text("\nError: file not found!");
			}
			
		}
	}
	else if (streq(comm, "movf") == 0){
		char* args = comm + 5;
        
        char* filename_start = args;
        char* fn = "";
        
        while (*args == ' ') args++;
        filename_start = args;
		
		char end_char[2];
		
		end_char[0] = filename_start[strsz(filename_start) - 1];
		end_char[1] = '\0';
		
		filename_start[strsz(filename_start) - 1] = '\0';
        
        while (*args != '\0' && *args != ' ') args++;
        
        if (*args == '\0') {
            push_text("\nUsage: movf <filename1> <filename2>");
        }
        else{
			*args = '\0';
			args++;
			
			fn = args;
			char filename2[MAX_FILENAME_LEN];
			int i = 0;
			
			for (i; i < strsz(fn); i++){
				filename2[i] = fn[i];
			}
			
			filename2[i++] = end_char[0];
			filename2[i++] = end_char[1];
			
			char buffer[MAX_FILE_SIZE + 1];
			int bytes_read = file_read(filename_start, buffer, MAX_FILE_SIZE, 0);
			
			char ech[2];
			
			ech[0] = buffer[MAX_FILE_SIZE];
			ech[1] = '\0';
			
			if( bytes_read >= 0){
				if (file_wr(filename2, buffer, strsz(buffer)) == 0){
					file_add_data(filename2, ech, strsz(ech));
					
					files_er(filename_start);
					
					push_text("\nData moved to other file.");
				}
			}
			else{		
				push_text("\nError: file not found!");
			}
			
		}
	}
	else if (streq(comm, "setcolor") == 0) {
		if (args && *args) {
			if (streq(args, "--default") == 0){
				bg_col = 0;
				text_col = 0xF;
				csc();
			}
			else {
				char hex[2];
				hex[0] = args[0];
				hex[1] = '\0';
				
				if (hextodig(hex) > 7){
					push_text("\nError: unavaible parameter ");
					push_text(hex);
					push_char('!');
				}
				else {
			
					bg_col = hextodig(hex);
					hex[0] = args[1];
					text_col = hextodig(hex);
				
					csc();
				}
			}
		}
		else {
			push_text("\nUsage: setcolor <color(0-F)><color(0-F)> or setcolor --default\nExample: setcolor 0F");
		}
	}
	else if (streq(comm, "themes") == 0) {
		themes();
	}
	else if (streq(comm, "panic") == 0){
		
		if (input_mode){
			push_text("\nRun time warning! Command panic blocked in execute mode!");
			return;
		}
		
		setmemory(input_buf, 0, sizeof(input_buf));
		setmemory(input_buf_exec, 0, sizeof(input_buf_exec));
				
		input_mode = 1;
		input_exec_ready = 0;
		buf_id = 0;
		input_buf[0] = '\0';
		
		int i = 0;
		unsigned short reserved;
		push_text("\nARE YOU SURE? [y/n]");
		
		YN_INPUT:
			idt_ini();
			push_text("\n> ");
				
			pic_remap();
				
			while (!input_exec_ready){
				asm volatile("hlt");
			}
			input_exec_ready = 0;
			
			for (i; input_buf_exec[i] != 'y' || input_buf_exec[i] != 'n'; i++){
				if (input_buf_exec[i] == 'y'){
					input_mode = 0;
					slp(2500);
					asm volatile (
						"divl %2" 
						: "=a" (reserved)
						: "a" (100), "r" (0) 
						: "edx"
					);
				}
				else if (input_buf_exec[i] == 'n'){
					push_text("\nCanceled.");
					input_mode = 0;
					goto END;
				}
			}
			
			goto YN_INPUT;
		
		END:
	}
	else if (streq(comm, "timer") == 0){
		if (args && *args) {
			if (streq(args, "--start") == 0){
				timer_time = get_unix_t();
				push_text("\nTimer started");
				is_timer_started = 1;
			}
			else if (streq(args, "--stop") == 0){
				char total_time_str[12];
				
				unsigned int total_time = get_unix_t() - timer_time;
				push_text("\nTimer stopped, total time: ");
				digtostr(total_time, total_time_str);
				push_text(total_time_str);
				timer_time = 0;
				is_timer_started = 0;
			}
			else if (streq(args, "--reset") == 0){
				push_text("\nTimer reset");
				timer_time = get_unix_t();
				is_timer_started = 1;
			}
		}
		else {
			if (is_timer_started == 0){
				push_text("\nTimer not started! For start: timer --start");
			}
			else {
				char cur_time[12];
				
				unsigned int current_time = get_unix_t() - timer_time;
				push_text("\nCurrent time: ");
				digtostr(current_time, cur_time);
				push_text(cur_time);
			}
		}
	}
	else if (streq(comm, "pci") == 0){
		if (args && *args) {
			if (streq(args, "--rescan") == 0){
				scan_pci();
			}
			else if (streq(args, "--data") == 0){
				
				for (int i = 0; i < 256; i++){
					if (pci_bus.pci_ch[i].used){
						push_char('\n');
						push_format("PCI (Bus=%u, Dev=%u, Func=%u):\nVendor=0x%x02, Device=0x%x02, Class=0x%x01, Subclass=0x%x01, ProgIF=0x%x01", pci_bus.pci_ch[i].bus, pci_bus.pci_ch[i].dev, pci_bus.pci_ch[i].func, pci_bus.pci_ch[i].vend_id, pci_bus.pci_ch[i].device_id, pci_bus.pci_ch[i].class_code, pci_bus.pci_ch[i].subclass, pci_bus.pci_ch[i].prog_if);
					}
				}
			}
			else if (streq(args, "--ahci") == 0){
				int j = 0;
				for (int i = 0; i < 256; i++){
					if (pci_bus.pci_ch[i].class_code == 0x01 && pci_bus.pci_ch[i].subclass == 0x06 && pci_bus.pci_ch[i].used){
						
						unsigned int bar5 = read_pci_conf(pci_bus.pci_ch[i].bus, pci_bus.pci_ch[i].dev, pci_bus.pci_ch[i].func, 0x24);
						
						push_char('\n');
						push_format("AHCI (dev ahci%i) (Bus=%u, Dev=%u, Func=%u):\nVendor=0x%x02, Device=0x%x02, Class=0x%x01, Subclass=0x%x01, ProgIF=0x%x01, BAR5=0x%X", j, pci_bus.pci_ch[i].bus, pci_bus.pci_ch[i].dev, pci_bus.pci_ch[i].func, pci_bus.pci_ch[i].vend_id, pci_bus.pci_ch[i].device_id, pci_bus.pci_ch[i].class_code, pci_bus.pci_ch[i].subclass, pci_bus.pci_ch[i].prog_if, bar5);
						
						if (j < MAX_AHCI){
							ahci_devs[j] = (Ahci_dev){
								.ahci = pci_bus.pci_ch[i],
								.bar5 = bar5,
								.number = j
							};
							j++;
						}
						
						check_ahci_contr(pci_bus.pci_ch[i]);
						ls_ahci_ports(pci_bus.pci_ch[i]);
						chk_ahci_ports(pci_bus.pci_ch[i]);
					}
				}
			}
		}
		else {
			push_text("\nUsage: pci <--rescan> <--data> <--ahci>");
		}
	}
	else if (streq(comm, "rdhead") == 0) {
		if (args && *args) {
			int i = 0;
			
			char *arg1 = (void *)0;
			for (i; args[i] != ' '; i++){
				*(arg1 + i) = args[i];
			}
			
			*(arg1 + i) = 0;
			char *arg2 = args + i + 1;
			
			unsigned int size = 0;
			
			if (!strtodig(arg2, &size)) {
				push_text("\nError: Invalid size parameter!");
			}
			else if (size > MAX_FILE_SIZE){
				push_text("\nError: Invalid size parameter!");
			}
			else {
			
				char buffer[MAX_FILE_SIZE + 1];
				int bytes_read = file_read(arg1, buffer, size, 0);
				
				if (bytes_read >= 0 && size > 0){
					buffer[bytes_read] = '\0';
					push_char('\n');
					push_text(buffer);
				}
				else{		
					push_text("\nError: file not found!");
				}
				
			}
			
		}
		else {
			push_text("\nUsage: rdhead <filename> <size>");
		}
	}
	else if (streq(comm, "rdtail") == 0) {
		if (args && *args) {
			int i = 0;
			
			char *arg1 = (void *)0;
			for (i; args[i] != ' '; i++){
				*(arg1 + i) = args[i];
			}
			
			*(arg1 + i) = 0;
			char *arg2 = args + i + 1;
			
			unsigned int size = 0;
			unsigned char done = 0;
			
			if (!strtodig(arg2, &size)) {
				push_text("\nError: Invalid size parameter!");
			}
			else if (size > MAX_FILE_SIZE){
				push_text("\nError: Invalid size parameter!");
			}
			else {
				for (int i = 0; i < MAX_FILES; i++) {
					if (mdfs.files[i].used && streq(mdfs.files[i].name, arg1) == 0) {
						char buffer[MAX_FILE_SIZE + 1];
						int bytes_read = file_read(arg1, buffer, size, (mdfs.files[i].size - size));
						
						if (bytes_read >= 0 && size > 0){
							buffer[bytes_read] = '\0';
							push_char('\n');
							push_text(buffer);
						}
						
						done = 1;
					}
				}
				if (!done){
					push_text("\nError: file not found!");
				}
			}
			
		}
		else {
			push_text("\nUsage: rdtail <filename> <size>");
		}
	}
	else if (streq(comm, "chsymb") == 0) {
		if (args && *args) {
			int i = 0;

			char *arg1 = (void *)0;
			while (args[i] != ' ' && args[i] != '\0') {
				*(arg1 + i) = args[i];
				i++;
			}
			*(arg1 + i) = '\0';

			if (args[i] == '\0' || args[i+1] == ' ' || args[i+2] == '\0' || args[i+3] == ' ') {
				push_text("\nUsage: chsymb <filename> <symbol1> <symbol2>");
			}
			else {
				char *arg2 = args + i + 1;
				char *arg3 = args + i + 3;

				if (arg2[1] != ' ' || arg3[0] == '\0') {
					push_text("\nError: symbol1 and symbol2 must be single characters!");
				}
				else {
					char buffer[MAX_FILE_SIZE + 1];
					int bytes_read = file_read(arg1, buffer, MAX_FILE_SIZE, 0);

					if (bytes_read >= 0) {
						for (int j = 0; j < bytes_read; j++) {
							if (buffer[j] == arg2[0]) {
								buffer[j] = arg3[0];
							}
						}
						file_wr(arg1, buffer, bytes_read);
					} else {
						push_text("\nError: file not found!");
					}
				}
			}
		} else {
			push_text("\nUsage: chsymb <filename> <symbol1> <symbol2>");
		}
	}
	else if (streq(comm, "PASS") == 0){
		push_char('\0');
	}
	else {
		if (!is_comdummy){
			push_text("\nUnknown command > ");
			push_text(comm);
		}
		else {
			is_comdummy = 0;
			return;
		}
	}
	
	if (!input_mode){
		push_text("\n\nMoonOS:>> ");
	}
}

void get_system_info(){
	push_text("\nMoon OS Delta\nversion - 1.2.2 Disk update Vol.3, Standart\nNewest version see on github: github.com/ItheJ/Moon-OS-Delta");
		
	if (!cpuid_supported()) {
		push_text("\nWarning: CPU not supported \"cpuid\"");
	}
	else {
		char arr[13];
		char brand[49];
			
		unsigned char family;
		unsigned char model;
			
		char *family_s = "";
		char *model_s = "";
			
		unsigned int ecx, edx;
			
		get_cpuid_info(arr, 0);
		push_text("\n\nCPU vendor: ");
		push_text(arr);
		get_cpuid_brand(brand);
		push_text("\nCPU brand: ");
		push_text(brand);
			
		get_cpuid_features(&family, &model, &ecx, &edx);
			
		digtostr(family, family_s);
		digtostr(model, model_s);
			
		push_text("\nCPU Family: ");
		push_text(family_s);
		push_text("  Model: ");
		push_text(model_s);
			
		push_text("\nAvailable features: ");
			
		if (edx & (1 << 23)) push_text("MMX ");
			
		if (edx & (1 << 25)) push_text("SSE ");
		if (edx & (1 << 26)) push_text("SSE2 ");
			
		if (ecx & (1 << 0)) push_text("SSE3 ");
		if (ecx & (1 << 9)) push_text("SSSE3 ");
		if (ecx & (1 << 19)) push_text("SSE4.1 ");
		if (ecx & (1 << 20)) push_text("SSE4.2 ");
			
		if (ecx & (1 << 28)) push_text("AVX ");
	}
	multiboot_info_t *mbi = (multiboot_info_t *)mlt_inf;
	
	char mem_cmos[21];
	char mem_mbi[21];
	
	if (!mbi) {
		push_text("\n\nMultiboot info not available [total memory will not write]\n");
	}
	else {
		if (mbi->flags & 0x001) {
			unsigned int total_kb = mbi->mem_lower + mbi->mem_upper;
			unsigned int total_mb = (total_kb + 1024) / 1024;
			
			push_text("\n\nMemory: ");
			
			digtostr(total_mb, mem_mbi);
			push_text(mem_mbi);
			
			push_text(" MB total\n");
			push_text(" - Conventional: ");
			
			digtostr(mbi->mem_lower, mem_mbi);
			push_text(mem_mbi);
			push_text(" KB (0-640 KB)\n");
			
			push_text(" - Extended: ");
			
			digtostr(mbi->mem_upper, mem_mbi);
			push_text(mem_mbi);
			
			push_text(" KB (1 MB+)\n");		
		}
	}
	digtostr(get_memory_size(), mem_cmos);
	
	push_text("\nMemory (by CMOS): ");
	push_text(mem_cmos);
	push_text(" KB");
	
	char vde[6];
	char hde[6];
		
	push_text("\nScreen size: ");
	digtostr(get_vde(), vde);
	digtostr(get_hde(), hde);
	push_text(vde);
	push_char('x');
	push_text(hde);
	
	seed = seeding_rnd(seed, 0xFF);
	
	push_format("\nRandom seed: %u", seed);
}

void Mdrw_exec(){
	Mdrw *text_ed;
	
	strcopy(text_ed->filename, "unnamed");
	
	check_comm("touch unnamed");
	cls();
	
	text_ed->cur_xe = 0;
	text_ed->cur_ye = 0;
	text_ed->scroll_offset = 0;
	text_ed->modified = 0;

	setmemory(text_ed->lines, 0, sizeof(text_ed->lines));

	text_ed->total_lines = 0;
	
	mdrw_load_file(text_ed->filename, text_ed);
	mdrw_run = 1;
	
	Event event;
	
	while (mdrw_run) {
		
		if (ev_q_get(&event)){
			if(event.type == EVENT_KEY_PRESS){
				unsigned char scancode = event.data;
				
				switch(scancode) {
					case 0x48: { //Key up
						if (text_ed->cur_ye > 0) text_ed->cur_ye--; text_ed->cur_xe = 0;
						mdrw_need_draw = 1;
						break;
					}
					case 0x50: { //Key down
						if (text_ed->cur_ye < text_ed->total_lines - 1) text_ed->cur_ye++; text_ed->cur_xe = 0;
						mdrw_need_draw = 1;
						break;
					}
					case 0x4B: { //Key left
						if (text_ed->cur_xe > 0) text_ed->cur_xe--;
						mdrw_need_draw = 1;
						break;
					}
					case 0x4D: { //Key right
						if (text_ed->cur_xe < 80){
							if (text_ed->lines[text_ed->cur_ye][text_ed->cur_xe] == '\0'){
								text_ed->lines[text_ed->cur_ye][text_ed->cur_xe] = ' ';
							}
							text_ed->cur_xe++;
						}
						mdrw_need_draw = 1;
						break;
					}
					case 0x47: { //home
						text_ed->cur_xe = 0;
						mdrw_need_draw = 1;
						break;
					}
					case 0x4F: { //end (key)
						text_ed->cur_xe = strsz(text_ed->lines[text_ed->cur_ye]);
						mdrw_need_draw = 1;
						break;
					}
					case F1: {
						mdrw_save_file(text_ed->filename, text_ed);
						mdrw_need_draw = 1;
						break;
					}
					case F2: {
				
						char filename[MAX_FILENAME_LEN];
						mdrw_input_dialog(filename, text_ed, "\nFilename: ");
				
						strcopy(text_ed->filename, filename);
				
						text_ed->cur_xe = 0;
						text_ed->cur_ye = 0;
						text_ed->scroll_offset = 0;
						text_ed->modified = 0;

						setmemory(text_ed->lines, 0, sizeof(text_ed->lines));

						text_ed->total_lines = 0;
	
						mdrw_load_file(text_ed->filename, text_ed);
						mdrw_need_draw = 1;
						break;
					}
					case F5: {
						mdrw_run = 0;
						cls();
						goto END;
						break;
					}
					case 0x1C: {
						if (text_ed->total_lines < 50) {
							text_ed->cur_ye++;
							text_ed->cur_xe = 0;
					
							text_ed->lines[text_ed->total_lines][0] = '\0';
							text_ed->total_lines++;
							text_ed->modified = 1;
						}
						mdrw_need_draw = 1;
						break;
					}
					case 0x0E: {
						if (text_ed->cur_xe > 0) {
							int line_len = strsz(text_ed->lines[text_ed->cur_ye]);
							for (int i = text_ed->cur_xe - 1; i < line_len; i++) {
								text_ed->lines[text_ed->cur_ye][i] = text_ed->lines[text_ed->cur_ye][i+1];
							}
       
							text_ed->cur_xe--;
							text_ed->modified = 1;
						}
						mdrw_need_draw = 1;
						break;
					}
					default: {
						
						unsigned char sh_active = kbs.lshift || kbs.rshift;
						unsigned char use_sh_tabl = (sh_active || kbs.capslc);
						
						unsigned char key_code = scancode & 0x7F;
						
						char base_ascii = (key_code < 128) ? scancodes[key_code] : 0;
						int is_letter = (base_ascii >= 'a' && base_ascii <= 'z');
    
						char ascii = 0;
    
						if (is_letter) {
							if (sh_active != kbs.capslc) {
								ascii = scancodes_sh[key_code];
							} else {
								ascii = scancodes[key_code];
							}
						} else {
							if (sh_active) {
								ascii = scancodes_sh[key_code];
							} else {
								ascii = scancodes[key_code];
							}
						}
						if (ascii != 0 && ascii >= 32 && ascii <= 126) {
							int line_len = strsz(text_ed->lines[text_ed->cur_ye]);
        
							if (line_len < 79) {
								for (int i = line_len; i > text_ed->cur_xe; i--) {
									text_ed->lines[text_ed->cur_ye][i] = text_ed->lines[text_ed->cur_ye][i-1];
								}
            
								text_ed->lines[text_ed->cur_ye][text_ed->cur_xe] = ascii;
								text_ed->lines[text_ed->cur_ye][line_len + 1] = '\0';
								text_ed->cur_xe++;
								text_ed->modified = 1;
							}
						}
						mdrw_need_draw = 1;
						break;
					}
				}
			}
		}
		
		if (mdrw_need_draw) {
			cls();
    
			draw_ed(text_ed);
			mdrw_need_draw = 0;
		}
		mdrw_handler();
		
		asm("hlt");
	}
	
	END:
		setmemory(input_buf, 0, sizeof(input_buf));
		while (GlobalEvQ.count != 0) ev_q_del();
		emul_key_press(0x1C);
}