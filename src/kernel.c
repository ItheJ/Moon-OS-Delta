// Moon OS Delta 1.0
// Author - 'Zondrobonie' Ivan, put copyright here ;)

//put multiboot to first file's bytes
__attribute__((section(".multiboot"), used))
const unsigned int multiboot_header[4] = {
	0x1BADB002,
	0x00000000,
	0xE4524FFE,
	0x00000000
};
//set standart cursor position
int cur_x = 0;
int cur_y = 0;

//define for keyboard port
#define KEYBOARD_PORT 0x60
//max size input buffer
#define BUFFER_SIZE 256
//defines for RAM MDFS
#define MAX_FILES 256
#define MAX_FILENAME_LEN 32
#define MAX_FILE_SIZE 4096
//defines for ATA/IDE and other drivers
#define IDE_DATA 0x1F0
#define IDE_ERR 0x1F1
#define IDE_SCTR_CNT 0x1F2
#define IDE_LBA_LOW 0x1F3
#define IDE_LBA_MID 0x1F4
#define IDE_LBA_HIGH 0x1F5
#define IDE_DRIVE_SEL 0x1F6
#define IDE_COM 0x1F7
#define IDE_STATUS 0x1F7

#define COMM_RD 0x20
#define COMM_WR 0x30
#define ST_BSY 0x80
#define ST_DRDY 0x40
#define ST_DRQ 0x08

#define SECTOR_SIZE 512
#define SUPERBLOCK_SECTOR 0
#define ROOT_DIR_SECTOR 1
#define DATA_START_SECTOR 2

#define DIR_SECTORS ((MAX_FILES * sizeof(struct DirEntry)) / SECTOR_SIZE + 1)

#define MAX_DEVICES 8
#define KERNEL_STACK_SIZE 8192

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

//for comdummy
#define MAX_DUMMY 50
#define MAX_DUMMY_NAME 32
#define MAX_DUMMY_SIZE 128

struct ComDummy{
	char name[MAX_DUMMY_NAME];
    char value[MAX_DUMMY_SIZE];
	unsigned char used;
};

// reader keyboard inputs

struct IDTent {
	unsigned short offset_low;
	unsigned short selector;
	unsigned char zero;
	unsigned char type_attr;
	unsigned short offset_high;
} __attribute__((packed));

//struct for work with time

typedef struct {
	unsigned char second;
	unsigned char minute;
	unsigned char hour;
	unsigned char day;
	unsigned char month;
	unsigned short year;
} Time;

typedef struct {
	unsigned char second;
	unsigned char minute;
	unsigned char hour;
	unsigned char day;
	unsigned char month;
	unsigned char year;
} RTC_Time;

// Superblock - magic fs name (MDFS), version MDFS, sector size, count inodes, bit card (free blocks) and reserved bytes for size Superblock is 512 bytes
// File Entry - name, size, char and used - inited in system or not
// Dir Entry - name, start sector, size and reserved size

struct Superblock {
	char magic[4];
	unsigned short version;
	unsigned short sector_sz;
	unsigned int inode_count;
	unsigned int free_blocks[64];
	unsigned int reserved[61];
};

typedef struct {
	char name[MAX_FILENAME_LEN];
	unsigned int size;
	unsigned char data[MAX_FILE_SIZE];
	unsigned char used;
} FileEntry;

struct DirEntry {
	char name[28];
	unsigned int start_sctr;
	unsigned int size;
	unsigned int reserved;
};

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

//RAM MDFS - struct for init the ram system

typedef struct {
	FileEntry files[MAX_FILES];
	unsigned int file_count;
} MDFS;

// gdt - init and need structs

struct gdt_entry{
	unsigned short limit_low;
	unsigned short base_low;
	unsigned char base_mid;
	unsigned char access;
	unsigned char granularity;
	unsigned char base_high;
} __attribute__((packed));

struct gdt_ptr {
	unsigned short limit;
	unsigned int base;
} __attribute__((packed));

// tss entry for work system in protected mode
struct tss_entry {
	unsigned int prev_tss;
	unsigned int esp0;
	unsigned int ss0;
	unsigned int esp1;
	unsigned int ss1;
	unsigned int esp2;
	unsigned int ss2;
	unsigned int cr3;
	unsigned int eip;
	unsigned int eflags;
	unsigned int eax;
	unsigned int ebx;
	unsigned int edx;
	unsigned int ecx;
	unsigned int esp;
	unsigned int ebp;
	unsigned int esi;
	unsigned int edi;
	unsigned int es;
	unsigned int cs;
	unsigned int ss;
	unsigned int ds;
	unsigned int fs;
	unsigned int gs;
	unsigned int ldt;
	unsigned short trap;
	unsigned short iomap_base;
} __attribute__((packed));

typedef enum {
	DEV_TYPE_NONE,
	DEV_TYPE_IDE,
	DEV_TYPE_RAMDSK,
	DEV_TYPE_VIRTUAL
} DevType;

typedef struct {
	char name [12];
	DevType type;
	unsigned char available;
	unsigned char readonly;
} StDev;

//struct executor for execute code in *Execute mode*

typedef struct {
	unsigned char* program;
	unsigned int program_c;
	char work_stack[256];
	unsigned int stack_pointer;
	unsigned char registers[8][16];
	unsigned char flags;
} Executor;

//init vars

StDev storage_devices[MAX_DEVICES];
unsigned char dev_count = 0;

MDFS mdfs;
KeyBoardState kbs;

struct gdt_entry gdt[6];
struct gdt_ptr gp;

struct IDTent idt[256];
struct tss_entry tss;

unsigned char kernel_stack[KERNEL_STACK_SIZE] __attribute__((aligned(16)));

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
//init vars...
static char input_buf[BUFFER_SIZE] = {'\0'};
static char input_buf_exec[BUFFER_SIZE] = {'\0'};
static unsigned int buf_id = 0;

static int input_mode = 0;
static int hlt_input_mode = 0;
static int input_exec_ready = 0;

static int in_format = 0;

static int debug_mode = 0;

// for lddsk correctly work
static struct DirEntry dir_entries[MAX_FILES] __attribute__((aligned(4)));

static struct ComDummy comdummies[MAX_DUMMY];
static unsigned short comdummy_count;
static unsigned short is_comdummy = 0;

// get address video memory
volatile unsigned short *video_mem = (volatile unsigned short *)0xB8000;
//function prototypes...
static inline void outb(unsigned short port, unsigned char value);
static inline unsigned char inb(unsigned short port);

static inline void outw(unsigned short port, unsigned short value);
static inline unsigned short inw(unsigned short port);

void pic_remap();

void set_idt_ent(unsigned char num, unsigned int handler);
void idt_ini();

void cur_move();
void scroll_down();

void del_back();

void push_char(char ch);
void push_text(const char * text);

void cls();
//Function prototypes for work with strings
int streq(const char* str1, const char* str2);
int strnumbereq(const char* str1, const char* str2, int number);
char* strsep(const char *str, int c);
void strcopy(char * dest, const char * src);
void strcat(char * dest, const char * src);
void strnumbercopy(char *dest, const char *src, unsigned int n);
unsigned int strlen(const char *str);
char *strtok(char *str, const char *delim);
char strchr(const char* str, char c);

int isspace(char symbol);
int isnumber(char symbol);
//function prototypes for translate x to y
void digrostr(unsigned number, char * buffer);
void lglg_to_str(unsigned long long number, char * buffer);
int strtodig(const char *str, int *result);
int strtoudig(const char *str, unsigned int *result);

void keyboard_handler();
extern int keyboard_handler_entry();

void check_comm(const char* comm);

void logoff();
void rest();

void push_time();
void push_date();
void push_unix_t();
void set_rtc_time(RTC_Time* time);
int is_vis_year(int year);
void unixt_to_date(unsigned long unixt, int *second, int *minute, int *hour, int *day, int *month, int *year);

unsigned char read_cmos(unsigned char registr);
void write_cmos(unsigned char addr, unsigned char value);
unsigned char is_upd();
unsigned char read_cmos_s(unsigned char registr);
unsigned char bcd_to_bin(unsigned char bcd);
unsigned char bin_to_bcd(unsigned char bin);


Time get_t();
unsigned int get_unix_t();

void mdfs_ini();
//function prototypes for work with files in RAM
int file_cr(const char* filename);
int file_wr(const char* filename, const char * data, unsigned int size);
int file_read(const char *filename, char * buffer, unsigned int buffer_size);
int file_del(const char *filename);
int file_rnm(const char* old_filename, const char* new_filename);
int files_er();
void files_list();

int file_add_data(const char* filename, const char * data, unsigned int size);

unsigned long long get_memory_size();

void *setmemory(void *ptr, int value, unsigned int number);
void *memset(void *s, int c, unsigned int n);
void *copymemory(void *dest, const void *src, unsigned int number);
//function prototypes for get size of screen (vertical and horisontal)
unsigned short get_vde();
unsigned short get_hde();

void get_cpu_vendor(char vendor[13]);

void beep(unsigned int freq, unsigned int durations);
void slp(unsigned int ms);

int power(int x, int y);

void gdt_ini();
void gdt_set_gate(int number, unsigned long base, unsigned long limit, unsigned char access, unsigned char gran);
extern void gdt_flush(unsigned int);

void tss_ini();
void gdt_set_tss(int number, unsigned int base, unsigned int limit);

void wait_dsk_ready();
int wait_dsk_drq();

void mdfs_format(const char * dev_name);

void svdsk();
void lddsk(const char * dev_name);

int chkdsk();

void devdetect();
int get_dev(const char* name);
void lsdevs();

int chkide();
void ide_reset();
void paging_ini();

// func for MDcode and command "exec"
void interpret_program(unsigned char *program);

void hltmode();

void krnl_run (void){
	cls();
	asm volatile("mov %0, %%esp" : : "r"(kernel_stack + KERNEL_STACK_SIZE));
	
	gdt_ini();
	tss_ini();
	idt_ini();
	gdt_set_tss(5, (unsigned int)&tss, sizeof(tss)-1);
	asm volatile("ltr %%ax" : : "a"(0x28));
	
	push_text("=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n        *                *             *                 *            *\n    *           *     MOON OS DELTA             *\n         *             (   )              *           *             *\n *          *                      *                     *        *\n=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n\n");
	
	devdetect();
	
	push_text("MoonOS:>> ");

	pic_remap();
	while (1);
	
}

static inline void outb(unsigned short port, unsigned char value){
	asm volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}
static inline unsigned char inb(unsigned short port){
	unsigned char ret;
	asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
	return ret;
}

static inline void outw(unsigned short port, unsigned short value){
	asm volatile ("outw %0, %1" : : "a"(value), "Nd"(port));
}
static inline unsigned short inw(unsigned short port){
	unsigned short ret;
	asm volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port));
	return ret;
}

void pic_remap() {
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

void set_idt_ent(unsigned char num, unsigned int handler){
	idt[num].offset_low = handler & 0xFFFF;
	idt[num].selector = 0x08;
	idt[num].zero = 0;
	idt[num].type_attr = 0x8E;
	idt[num].offset_high = (handler >> 16) & 0xFFFF;
}
void idt_ini() {
	set_idt_ent(0x21, (unsigned int)keyboard_handler_entry);
	
	
	struct {
		unsigned short limit;
		unsigned int base;
	} __attribute__((packed)) idtr = {sizeof(idt) -1, (unsigned int)idt};
	
	asm volatile("lidt %0" : : "m"(idtr));
	
	asm volatile("sti");
}

void gdt_set_gate(int number, unsigned long base, unsigned long limit, unsigned char access, unsigned char gran){
	gdt[number].base_low = base & 0xFFFF;
	gdt[number].base_mid = (base >> 16) & 0xFF;
	gdt[number].base_high = (base >> 24) & 0xFF;
	gdt[number].limit_low = (limit & 0xFFFF);
	gdt[number].granularity = ((limit >> 16) & 0x0F);
	gdt[number].granularity |= (gran & 0xF0);
	gdt[number].access = access;
}

void gdt_ini(){
	gp.limit = (sizeof(struct gdt_entry) * 6) - 1;
	gp.base = (unsigned int)&gdt;
	
	gdt_set_gate(0, 0, 0, 0, 0);
	
	gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
	
	gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);
	
	//gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);
	//gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);
	
	gdt_set_gate(5, (unsigned int)&tss, sizeof(tss), 0x89, 0x40);
	
	gdt_flush((unsigned int)&gp);
}

void tss_ini(){
	tss.esp0 = 0x00100000;
	tss.ss0 = 0x10;
	setmemory(&tss, 0, sizeof(tss));
	tss.iomap_base = sizeof(tss);
}

void gdt_set_tss(int number, unsigned int base, unsigned int limit){
	gdt[number].base_low = base & 0xFFFF;
	gdt[number].base_mid = (base >> 16) & 0xFF;
	gdt[number].base_high = (base >> 24) & 0xFF;
	gdt[number].limit_low = limit & 0xFFFF;
	gdt[number].granularity = ((limit >> 16) & 0x0F) | 0x40;
	gdt[number].access = 0x89;
}

void paging_ini(){
	unsigned int *page_dir = (unsigned int *)0x10000;
	setmemory(page_dir, 0, 4096);
	
	unsigned int *page_table = (unsigned int *)0x11000;
	for (int i = 0; i < 1024; i++){
		page_table[i] = (i * 0x1000) | 0x03;
	}
	
	page_dir[0] = (unsigned int)page_table | 0x03;
	page_dir[768] = (unsigned int)page_table | 0x03;
	
	asm volatile("mov %%cr3, %0" : : "r"(page_dir));
	
	unsigned int cr0;
	asm volatile("mov %%cr0, %0" : "=r" (cr0));
	cr0 |= 0x80000000;
	asm volatile("mov %0, %%cr0" : : "r"(cr0));
}

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
		video_mem[24 * 80 + x] = (0x0F << 8) | ' ';
	}
}

void del_back(){
	if (cur_x > 0){
		cur_x--;
	} else if (cur_y > 0){
		cur_y--;
		cur_x = 79;
	}
	
	video_mem[cur_y * 80 + cur_x] = (0x0F << 8) | ' ';
	cur_move();
}

void push_char (char ch){
	if (ch == '\n'){
		cur_x = 0;
		cur_y++;
	} else {
		video_mem[cur_y * 80 + cur_x] = (0x0F << 8) | ch;
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
		video[i+1] = 0x07;
	}
	cur_x = 0;
	cur_y = 0;
	cur_move();
}

int streq(const char* str1, const char* str2){
	while(*str1 && (*str1 == *str2)) {
		str1++;
		str2++;
	}
	return *(const unsigned char*)str1 - *(const unsigned char*)str2;
}

int strnumbereq(const char* str1, const char* str2, int number){
	while(number && *str1 && (*str1 == *str2)) {
		str1++;
		str2++;
	}
	if (number == 0) {
		return 0;
	} else {
		return (*(unsigned char *)str1 - *(unsigned char *)str2);
	}
}
char* strsep(const char *str, int c) {
	while (*str != '\0') {
		if (*str == c) {
			return (char *)str;
		}
		str++;
	}
	if (c == '\0') {
		return (char *)str;
	}
	return (char *)'\0';
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
				push_text("\nAvailable commands:\n  help <--standart> <--filew> <--advanced> - print this message,\n  cls - CLear the Screen,\n  echo <text> - print text,\n  logoff - quit from system\n  rest - reset the system\n  abt - info the system and authors / credits\n  mdver - version the system and devices\n  time <--hms> <--dmy> <--unixt> - print time or date");
			}
			else if (streq(args, "--filew") == 0) {
				push_text("\nAvailable commands:\n  touch <filename> - create file,\n  wr <filename> <text> - write text in file,\n  rd <filename> - read file data,\n  del <filename> - delete file\n  erase - delete all files\n  ls - list all files,\n  add <filename> <text> - add text in file\n  rnm <old filename> <new filename> - rename file");
			}
			else if (streq(args, "--advanced") == 0){
				push_text("\nAvailable commands:\n  beep <frequency> <durations (ms)> - sound a signal\n  lddsk <dev> - load data from <dev> to RAM\n  svdsk - save data from RAM\n  lsdevs - list of available device\n  formt <dev> - format <dev> and set MDFS on <dev>\n  exec <filename> - execute file in MDcode\n  comdummy <pointer>=<command> - create a custom command\n  lscomdum - list of created comdummy\n  chcomdum <pointer>=<command> - change command in comdummy\n  chdbg - on/off debug mode\n  timeset <flag (time/date/unixt)> <time (HH:MM:SS)/date (DD:MM:YYYY)/unixt (unix timestamp)> - set new time/date\n ");
			}
		} else {
			push_text("\nFor more commands print help <--standart> or help <--filew> or help <--advanced>");
		}
	}
	else if (streq(comm, "cls") == 0) {
		cls();
	}
	else if (streq(comm, "echo") == 0) {
		push_char('\n');
		if (args && *args){
			push_text(args);
		} else {
			push_text("Usage: echo <text>");
		}
	}
	else if (streq(comm, "logoff") == 0) {
		push_text("\nLogging off... ");
		logoff();
	}
	else if (streq(comm, "rest") == 0) {
		rest();
	}
	else if (streq(comm, "abt") == 0) {
		push_text("\nABOUT:\n  Moon OS Delta\n  programmer and author - 'Zondrobonie' Ivan\n  for news: t.me/MoonOSDelta \n  Special thanks - BFG (chat), Imancat (for code for comdummy) and others...");
	}
	else if (streq(comm, "mdver") == 0) {
		
		char mem[21];
		char vend[13];
		
		char vde[6];
		char hde[6];
		
		push_text("\nMoon OS Delta\nversion - 1.1 November update Vol.1, Standart\nNewest version see on github: github.com/ItheJ/Moon-OS-Delta \n\n");
		
		push_text("Available memory - ");
		digrostr(get_memory_size(), mem);
		push_text(mem);
		push_text(" KB\n");
		
		get_cpu_vendor(vend);
		push_text("CPU vendor: ");
		push_text(vend);
		
		
		push_text("\nScreen size: ");
		digrostr(get_vde(), vde);
		digrostr(get_hde(), hde);
		push_text(vde);
		push_char('x');
		push_text(hde);
		
		
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
		
        end_char[0] = filename_start[strlen(filename_start) - 1];
		end_char[1] = '\0';
		
		filename_start[strlen(filename_start) - 1] = '\0';
        
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

			if (file_wr(filename_start, text_start, strlen(text_start)) == 0){
				file_add_data(filename_start, end_char, strlen(end_char));
				file_add_data(filename_start, " \n", strlen(" \n"));
			}
		}
    }
	else if (streq(comm, "rd") == 0) {
		if (args && *args) {
			char buffer[MAX_FILE_SIZE + 1];
			unsigned int bytes_read = file_read(args, buffer, MAX_FILE_SIZE);
			
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
		files_er();
	}
	else if (streq(comm, "ls") == 0) {
		files_list();
	}
	else if (streq(comm, "beep") == 0){
		unsigned int freq = 1000;
		unsigned int duration = 200;
    
		if (args && *args) {

			char args_copy[64];
			strnumbercopy(args_copy, args, sizeof(args_copy)-1);
			args_copy[sizeof(args_copy)-1] = '\0';
			
			char* freq_str = strtok(args_copy, " ");
			char* dur_str = strtok((void *)0, " ");
        
			if (!strtodig(freq_str, &freq)) {
				push_text("\nInvalid frequency format!");
				return;
			}
        
			if (dur_str && !strtodig(dur_str, &duration)) {
				push_text("\nInvalid duration format!");
				return;
			}

			if (freq < 20 || freq > 20000 || duration < 10 || duration > 5000) {
				push_text("\nInvalid parameters! Valid ranges: frequency 20-20000Hz, duration 10-5000ms");
				return;
			}
		}
		
		char frq[6];
		char dur[5];
		
		beep(freq, duration);
		
		push_text("\nBeep at ");
		
		digrostr(freq, frq);
		digrostr(duration, dur);
		
		push_text(frq);
		push_text("Hz for ");
		push_text(dur);
		push_text(" ms");

	}
	else if (streq(comm, "lddsk") == 0){
		if (args && *args) {
			if (input_mode){
				push_text("\nRun time warning! Command lddsk blocked in execute mode!");
				return;
			}
			else {
				lddsk(args);
			}
		}
		else {
			push_text("\nUsage: lddsk <dev>");
		}
	}
	else if (streq(comm, "svdsk") == 0){
		svdsk();
	}
	else if (streq(comm, "lsdevs") == 0){
		lsdevs();
	}
	else if (streq(comm, "formt") == 0){
		if (args && *args) {
			mdfs_format(args);
		}
		else {
			push_text("\nUsage: formt <dev>");
		}
	}
	else if (streq(comm, "add") == 0) {
		char* args = comm + 4;
        
        char* filename_start = args;
        char* text_start = "";
        
        while (*args == ' ') args++;
        filename_start = args;
		
		char end_char[2];
		
        end_char[0] = filename_start[strlen(filename_start) - 1];
		end_char[1] = '\0';
		
		filename_start[strlen(filename_start) - 1] = '\0';
        
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
			
			file_add_data(filename_start, text_start, strlen(text_start));
			file_add_data(filename_start, end_char, strlen(end_char));
			file_add_data(filename_start, " \n", strlen(" \n"));
			
			push_text("\nData added in file.\n");
		}
    }
	else if (streq(comm, "exec") == 0){
		char* args = comm + 5;
		if (args && *args) {
			if (strlen(args) < 5 || strnumbereq((args + (strlen(args) - 5)), ".mdxt", 5)){
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
    
					char* end = normalized_name + strlen(normalized_name) - 1;
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
		
        end_char[0] = filename_start[strlen(filename_start) - 1];
		end_char[1] = '\0';
		
		filename_start[strlen(filename_start) - 1] = '\0';
        
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
			
			for (i; i < strlen(fn); i++){
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
				
				if (strlen(name) >= MAX_DUMMY_NAME) {
					push_text("\nError: too long name!");
				} 
				else if (strlen(command) >= MAX_DUMMY_SIZE) {
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
							if ( comdummies[comdummy_count].name == "help" || comdummies[comdummy_count].name == "cls" || comdummies[comdummy_count].name == "echo" || comdummies[comdummy_count].name == "logoff" || comdummies[comdummy_count].name == "rest" || comdummies[comdummy_count].name == "abt" || comdummies[comdummy_count].name == "mdver" || comdummies[comdummy_count].name == "time" || comdummies[comdummy_count].name == "touch" || comdummies[comdummy_count].name == "wr" || comdummies[comdummy_count].name == "rd" || comdummies[comdummy_count].name == "del" || comdummies[comdummy_count].name == "erase" || comdummies[comdummy_count].name == "ls" || comdummies[comdummy_count].name == "add" || comdummies[comdummy_count].name == "rnm" || comdummies[comdummy_count].name == "beep" || comdummies[comdummy_count].name == "lddsk" || comdummies[comdummy_count].name == "svdsk" || comdummies[comdummy_count].name == "lsdevs" || comdummies[comdummy_count].name == "formt" || comdummies[comdummy_count].name == "exec" || comdummies[comdummy_count].name == "comdummy" || comdummies[comdummy_count].name == "lscomdum" || comdummies[comdummy_count].name == "chcomdum" || comdummies[comdummy_count].name == "hltmode" || comdummies[comdummy_count].name == "chdbg" || comdummies[comdummy_count].name == "timeset"){
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
				
				if (strlen(name) >= MAX_DUMMY_NAME) {
					push_text("\nError: too long name!");
				} 
				else if (strlen(command) >= MAX_DUMMY_SIZE) {
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
		digrostr(debug_mode, db);
		
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
			push_text("Usage: timeset <flag (time/date/unixt)> <time (HH:MM:SS)/date (DD:MM:YYYY)/unixt (unix timestamp)>");
		}
	}
	else if (streq(comm, "PASS COMMAND") == 0){
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
							check_comm("PASS COMMAND");
						}
						else{
							check_comm(input_buf);
						}
						buf_id = 0;
					}
				}
				break;
				
			case F1:
				if (!input_mode){
					input_buf[buf_id] = '\0';
					
					buf_id = 0;
					
					push_text("\n ");
					push_char(2); //☻ lip biting emoticon
					push_text(" lip biting emoticon");
					
					check_comm("PASS COMMAND");
					
				}
				break;
			case F2:
				if (!input_mode){
					input_buf[buf_id] = '\0';
					
					buf_id = 0;
					
					push_text("\n ");
					push_char(3); //♥
					push_text(" I LOVE MOON OS DELTA!");
					
					check_comm("PASS COMMAND");
					
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
					
					check_comm("PASS COMMAND");
					
				}
				break;
			
			case F6:
				if (!input_mode){
					input_buf[buf_id] = '\0';
					buf_id = 0;
					
					lsdevs();
					
					check_comm("PASS COMMAND");
					
				}
				break;
				
			case F7:
				if (!input_mode){
					input_buf[buf_id] = '\0';
					buf_id = 0;
					
					files_list();
					
					check_comm("PASS COMMAND");
					
				}
				break;
				
			case 0x7D:
				if (!input_mode){
					input_buf[buf_id] = '\0';
					buf_id = 0;
					
					push_text("\n88");
					
					check_comm("PASS COMMAND");
					
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

void logoff(){
	
	outw(0x604, 0x2000);
	
	outw(0xB004, 0x2000);
	
	push_text("\nSystem halted. Now you can turn off the computer.");
	asm volatile("cli");
	while(1){
		asm volatile("hlt");
	}
}

void rest(){
	push_text("\nSystem is rebooting...");
	
	while(inb(0x64) & 0x02);
	outb(0x64, 0xFE);
	
	asm volatile ("cli");
	while(1){
		asm volatile ("hlt");
	}
}

unsigned char read_cmos(unsigned char registr){
	outb(0x70, registr);
	return inb(0x71);
}

void write_cmos(unsigned char addr, unsigned char value){
	outb(0x70, addr);
	outb(0x71, value);
}

unsigned char is_upd(){
	return read_cmos(0x0A) & 0x80;
}

unsigned char read_cmos_s(unsigned char registr){
	while (is_upd());
	return read_cmos(registr);
}

unsigned char bcd_to_bin(unsigned char bcd){
	return (bcd & 0x0F) + ((bcd >> 4) * 10);
}

unsigned char bin_to_bcd(unsigned char bin){
	return ((bin / 10) << 4) | (bin % 10);
}

Time get_t() {
	Time t;
	
	t.second = read_cmos_s(0x00);
	t.minute = read_cmos_s(0x02);
	t.hour = read_cmos_s(0x04);
	t.day = read_cmos_s(0x07);
	t.month = read_cmos_s(0x08);
	t.year = read_cmos_s(0x09);
	
	unsigned registrB = read_cmos(0x0B);
	if (!(registrB & 0x04)) {
		t.second = bcd_to_bin(t.second);
		t.minute = bcd_to_bin(t.minute);
		t.hour = bcd_to_bin(t.hour);
		t.day = bcd_to_bin(t.day);
		t.month = bcd_to_bin(t.month);
		t.year = bcd_to_bin(t.year);
	}
	
	t.year += 2000;
	
	return t;
}

unsigned int get_unix_t(){
	Time t = get_t();
	
	unsigned int days = (t.year - 1970) * 365 + (t.month - 1) * 30 + (t.day - 1);
	unsigned int hours = days * 24 + t.hour;
	unsigned int minutes = hours * 60 + t.minute;
	
	return minutes * 60 + t.second;
}

void digrostr(unsigned int number, char * buffer) {
	if (number == 0) {
		buffer[0] = '0';
		buffer[1] = '\0';
		return;
	}
	
	char temp[16];
	int i = 0;
	
	while (number > 0){
		temp[i++] = '0' + (number % 10);
		number /= 10;
	}
	
	for (int j = 0; j < i; j++){
		buffer[j] = temp[i - j - 1];
	}
	
	buffer[i] = '\0';
}

void strcopy(char *dest, const char *src){
	while(*src) {
		*dest++ = *src++;
	}
	*dest = '\0';
}
void strcat(char *dest, const char *src){
	while (*dest) dest++;
	while (*src) *dest++ = *src++;
	*dest = '\0';
}

void push_time(){
	Time t = get_t();
	
	char time_str[16];
	char hour_str[3], min_str[3], sec_str[3];
	
	push_text("Time: ");
	
	digrostr(t.hour, hour_str);
	digrostr(t.minute, min_str);
	digrostr(t.second, sec_str);
	
	push_text(hour_str);
	push_char(':');
	push_text(min_str);
	push_char(':');
	push_text(sec_str);
}

void push_date(){
	Time t = get_t();
	
	char day_str[3], month_str[3], year_str[5];
	
	push_text("Date: ");
	
	digrostr(t.day, day_str);
	digrostr(t.month, month_str);
	digrostr(t.year, year_str);
	
	push_text(day_str);
	push_char('|');
	push_text(month_str);
	push_char('|');
	push_text(year_str);
}

void push_unix_t(){
	
	Time t = get_t();
	
	unsigned int unix_t = get_unix_t();
	
	char unix_str[16];
	digrostr(unix_t, unix_str);
	
	push_text("Unix time: ");
	push_text(unix_str);
}

void *setmemory(void *ptr, int value, unsigned int number){
	unsigned char *p = ptr;
	while (number--) {
		*p++ = (unsigned char)value;
	}
	return ptr;
}
void *memset(void *s, int c, unsigned int n){
	return setmemory(s, c, n);
}

void mdfs_ini(){
	setmemory(&mdfs, 0, sizeof(mdfs));
}

int file_cr(const char* filename) {
	
	for (int i = 0; i < MAX_FILES; i++){
		if (mdfs.files[i].used && (streq(mdfs.files[i].name, filename) == 0)) {
			push_text("\nError: this file is exist before.");
			return -1;
		}
	}
	
	for (int i = 0; i < MAX_FILES; i++) {
		if (!mdfs.files[i].used){
			strnumbercopy(mdfs.files[i].name, filename, MAX_FILENAME_LEN);
			mdfs.files[i].size = 0;
			mdfs.files[i].used = 1;
			mdfs.file_count++;
			
			push_text("\nFile successfully created.");
			
			return 0;
		}
	}
	
	push_text("\nError: maximum files count was done.");
	
	return -2;
}

int file_wr(const char* filename, const char * data, unsigned int size){
	
	char normalized_name[MAX_FILENAME_LEN];
    strnumbercopy(normalized_name, filename, MAX_FILENAME_LEN);
    
    char* end = normalized_name + strlen(normalized_name) - 1;
    while (end >= normalized_name && (*end == ' ' || *end == '\r' || *end == '\n')) {
        *end = '\0';
		end--;
    }
    
    for (int i = 0; i < MAX_FILES; i++) {
        if (mdfs.files[i].used && streq(mdfs.files[i].name, normalized_name) == 0) {
            if (size > MAX_FILE_SIZE) {
				push_text("\nError: size file more that max size.");
                return -2;
            }
            
            copymemory(mdfs.files[i].data, data, size);
            mdfs.files[i].size = size;
			push_text("\nData wrote in file.");
            return 0;
        }
    }
	
	push_text("\nError: file not found.");
    return -1;
}

int file_read(const char *filename, char * buffer, unsigned int buffer_size) {
	for (int i = 0; i < MAX_FILES; i++){
		if (mdfs.files[i].used && streq(mdfs.files[i].name, filename) == 0) {
			unsigned int to_copy = (buffer_size < mdfs.files[i].size) ? buffer_size : mdfs.files[i].size;
			copymemory(buffer, mdfs.files[i].data, to_copy);
			return to_copy;
		}
	}
	
	return -1;
}

int file_del(const char *filename){
	for( int i = 0; i < MAX_FILES;  i++){
		if (mdfs.files[i].used && streq(mdfs.files[i].name, filename) == 0) {
			setmemory(&mdfs.files[i], 0, sizeof(FileEntry));
			mdfs.file_count--;
		
			push_text("\nFile successfully deleted.");
			return 0;
		}
	}
	
	push_text("Error: file not found!");
	
	return -1;
}

int files_er(){
	
	for( int i = 0; i < MAX_FILES;  i++){
		if ( mdfs.files[i].used){
			setmemory(&mdfs.files[i], 0, sizeof(FileEntry));
		}
	}
	
	push_text("\nAll files was erased.");
	
	mdfs.file_count = 0;
	
	return 0;
}

void files_list() {
	if (mdfs.file_count == 0){
		push_text("\nNo files found");
		return;
	}
	
	push_text("\nFiles:\n");
	for (int i = 0; i < MAX_FILES; i++) {
		if (mdfs.files[i].used){
			push_text("  ");
			push_text(mdfs.files[i].name);
			
			char size_str[16];
			digrostr(mdfs.files[i].size, size_str);
			
			push_text(" : ");
			push_text(size_str);
			push_text(" bytes;");
		}
	}
}

void strnumbercopy(char *dest, const char *src, unsigned int n) {
	unsigned int i;
	for (i = 0;i < n && src[i] != '\0'; i++){
		dest[i] = src[i];
	}
	dest[i] = '\0';
}

void *copymemory(void *dest, const void *src, unsigned int number){
	char *d = (char *)dest;
	const char *s = (const char *)src;
	
	for (unsigned int i = 0; i < number; i++){
		d[i] = s[i];
	}
	
	return dest;
}

unsigned int strlen(const char *str) {
	unsigned int len = 0;
	while (str[len] && str[len] != '\0') {
		len++;
	}
	return len;
}

unsigned long long get_memory_size(){
	unsigned long long total;
	total = read_cmos(0x30) << 8 | read_cmos(0x31);
	return total;
}

void lglg_to_str(unsigned long long number, char * buffer){
	if (number == 0) {
		buffer[0] = '0';
		buffer[1] = '\0';
		return;
	}
	
	char temp[21];
	int i = 0;
	
	while (number > 0){
		temp[i++] = '0' + (number % 10);
		number /= 10;
	}
	
	for (int j = 0; j < i; j++){
		buffer[j] = temp[i - j - 1];
	}
	
	buffer[i] = '\0';
}

unsigned short get_vde() {
	outb(0x3D4, 0x11);
	return (inb(0x3D5) + 1);
}

unsigned short get_hde() {
	outb(0x3D4, 0x01);
	return (inb(0x3D5) + 1);
}

void get_cpu_vendor(char vendor[13]){
	unsigned int eax, ebx, ecx, edx;
	
	asm volatile(
		"cpuid"
		: "=b" (ebx), "=c"(ecx), "=d" (edx)
		: "a" (0)
	);
	
	*(unsigned int*)&vendor[0] = ebx;
	*(unsigned int*)&vendor[4] = edx;
	*(unsigned int*)&vendor[8] = ecx;
	vendor[12] = '\0';
}

void beep(unsigned int freq, unsigned int durations){
	unsigned int divisor = 1193180 / freq;
	
	outb(0x43, 0xB6);
	outb(0x42, divisor & 0xFF);
	outb(0x42, (divisor >> 8) & 0xFF);
	
	unsigned char sp_state = inb(0x61);
	if(sp_state != (sp_state | 3)){
		outb(0x61, sp_state | 3);
	}
	
	slp(durations);
	
	outb(0x61, sp_state & -3);
}

int isspace(char symbol){
	if (symbol == ' '){
		return 1;
	}
	return 0;
}

int isnumber (char symbol){
	if (symbol >= '0' && symbol <= '9'){
		return 1;
	}
	return 0;
}

int strtodig(const char *str, int *result) {
    int sign = 1, value = 0, i = 0;
    
    while (isspace(str[i])) i++;
    
    if (str[i] == '-') {
        sign = -1;
        i++;
    } else if (str[i] == '+') {
        i++;
    }
    
    if (!isnumber(str[i])) return 0;


    while (isnumber(str[i])) {
        value = 10 * value + (str[i] - '0');
        i++;
    }
    
    *result = sign * value;
    return 1;
}
int strtoudig(const char *str, unsigned int *result) {
    int sign = 1, value = 0, i = 0;
    
    while (isspace(str[i])) i++;
    
    if (str[i] == '-') {
        i++;
    } else if (str[i] == '+') {
        i++;
    }
    
    if (!isnumber(str[i])) return 0;

    while (isnumber(str[i])) {
        value = 10 * value + (str[i] - '0');
        i++;
    }
    
    *result = sign * value;
    return 1;
}


int power(int x, int y){
	int result = 1;
	
	if (y < 0){
		x = 1 / x;
		y = -y;
	}
	
	for (int i; i < y; i++){
		result *= x;
	}
	
	return result;
	
}

void slp(unsigned int ms){
	for (volatile unsigned int i = 0; i < ms * 5000; i++);
}

char strchr(const char* str, char c) {
	int i = 0;
	while ((str[i] != '\0') && (str[i] != c)) i++;
	if (str[i] == '\0'){
		
		return '\0'; //(void*)0;
	}
	else{
		return str[i];
	}
}

char *strtok(char *str, const char *delim) {
    static char *last;
    char *token;

    if (str) {
        last = str;
    } else if (!last) {
        return (void*)0; 
    }

    token = last;

    while (*last) { 
        if (strchr(delim, *last)) { 
            *last = '\0';
            last++;
            return token;
        }
        last++;
    }

    last = (void*)0;
    return token;
}

void wait_dsk_ready(){
	int timeout = 10000;
	while (timeout--){
		unsigned char status = inb(IDE_STATUS);
		
		if (!(status & ST_BSY) && (status & ST_DRDY)){
			return;
		}
		
		if (status & 0x01) {
			push_text("\nError with worked disk!\n");
			return;
		}
		
		asm volatile("pause");
	}
	
	push_text("\nTime out!\n");
}

int wait_dsk_drq(){
	int timeout = 10000;
	while (timeout--){
		unsigned char status = inb(IDE_STATUS);
		
		if (status & ST_DRQ){
			return 1;
		}
		
		if (status & 0x01) {
			push_text("\nError with worked disk!\n");
			return 0;
		}
		
		if (status & ST_BSY){
			continue;
		}
		
		asm volatile("pause");
	}
	
	push_text("\nTime out!\n");
}

void rd_sector(unsigned int lba, void* buffer){
	wait_dsk_ready();
	
	asm volatile ("cli");
	
	outb(IDE_SCTR_CNT, 1);
	outb(IDE_LBA_LOW, lba & 0xFF);
	outb(IDE_LBA_MID, (lba >> 8) & 0xFF);
	outb(IDE_LBA_HIGH, (lba >> 16) & 0xFF);
	outb(IDE_DRIVE_SEL, 0xE0 | ((lba >> 24) & 0x0F));
	
	outb(IDE_COM, COMM_RD);
	
	if (!wait_dsk_drq()){
		asm volatile ("sti");
		return;
	}
	
	unsigned short* pointer = (unsigned short*)buffer;
	for(int i = 0; i < 256; i++){
		pointer[i] = inw(IDE_DATA);
	}
	
	wait_dsk_ready();
	
	asm volatile ("sti");
}

void wr_sector(unsigned int lba, const void* buffer){
	wait_dsk_ready();
	
	asm volatile ("cli");
	
	outb(IDE_SCTR_CNT, 1);
	outb(IDE_LBA_LOW, lba & 0xFF);
	outb(IDE_LBA_MID, (lba >> 8) & 0xFF);
	outb(IDE_LBA_HIGH, (lba >> 16) & 0xFF);
	outb(IDE_DRIVE_SEL, 0xE0 | ((lba >> 24) & 0x0F));
	
	outb(IDE_COM, COMM_WR);
	
	if (!wait_dsk_drq()){
		asm volatile("sti");
		return;
	}
	
	const unsigned short* pointer = (const unsigned short*)buffer;
	for (int i = 0; i < 256; i++){
		outw(IDE_DATA, pointer[i]);
	}
	
	wait_dsk_ready();
	
	asm volatile ("sti");
}

void mdfs_format(const char * dev_name) {
	
	asm volatile("cli");
	
	if (in_format){
		push_text("\nRecursive format call blocked!");
		in_format = 0;
		return;
	}
	in_format = 1;
	
	ide_reset();
	
	if (!chkdsk()){
		push_text("\nNo disk detected! Cannot detected");
		in_format = 0;
		return;
	}
	
	if (storage_devices[get_dev(dev_name)].readonly){
		in_format = 0;
		push_text("\nCannot format readonly device!");
		return;
	}
	
	unsigned char superblock_buf[SECTOR_SIZE];
	setmemory(superblock_buf, 0, sizeof(superblock_buf));
	
	struct Superblock* sb = (struct Superblock*)superblock_buf;
	
	sb->magic[0] = 'M';
	sb->magic[1] = 'D';
	sb->magic[2] = 'F';
	sb->magic[3] = 'S';
	
	sb->version = 1;
	sb->sector_sz = SECTOR_SIZE;
	sb->inode_count = 0;
	sb->free_blocks[0] = 0x3;

	wr_sector(SUPERBLOCK_SECTOR, superblock_buf);
	
	unsigned char read_buf[SECTOR_SIZE];
	rd_sector(SUPERBLOCK_SECTOR, read_buf);
	
	push_char('\n');
	
	for (int i = 0; i <= 3; i++){
		push_text("|");
		push_char(read_buf[i]);
		push_text("| ");
	}
	
	unsigned char empty_sector[SECTOR_SIZE];
	setmemory(empty_sector, 0, sizeof(empty_sector));
	
	for (int i = 0; i < DIR_SECTORS; i++){
		ide_reset();
		
		wr_sector(ROOT_DIR_SECTOR + i, empty_sector);
	}
	
	ide_reset();
	
	asm volatile("sti");
	
	push_text("\nDisk formated!");
	
	in_format = 0;
}

void svdsk(){
	
	asm volatile("cli");
	
	struct Superblock sb = {
		.magic = "MDFS",
		.version = 1,
		.sector_sz = SECTOR_SIZE,
		.inode_count = mdfs.file_count
	};
	
	wr_sector(SUPERBLOCK_SECTOR, &sb);
	
	struct DirEntry dir_entries[MAX_FILES];
	
	setmemory(dir_entries, 0, sizeof(dir_entries));
	
	unsigned int cur_data_sect = DATA_START_SECTOR;
	
	for(unsigned int i = 0; i < MAX_FILES; i++){
		if (mdfs.files[i].used) {
			strnumbercopy(dir_entries[i].name, mdfs.files[i].name, MAX_FILENAME_LEN - 1);
			dir_entries[i].name[MAX_FILENAME_LEN - 1] = '\0';
			
			dir_entries[i].start_sctr = cur_data_sect;
			dir_entries[i].size = mdfs.files[i].size;
			
			unsigned int sect_count = (mdfs.files[i].size + SECTOR_SIZE - 1) / SECTOR_SIZE;
			
			cur_data_sect += sect_count;
		}
	}
	
	unsigned int dir_sz = MAX_FILES * sizeof(struct DirEntry);
	unsigned int dir_sectr_count = (dir_sz + SECTOR_SIZE - 1) / SECTOR_SIZE;
	
	for (unsigned int i = 0; i < dir_sectr_count; i++){
		unsigned int cur_sector = ROOT_DIR_SECTOR + i;
		void *src = (char *)dir_entries + i * SECTOR_SIZE;
		
		wr_sector(cur_sector, src);
	}
	
	for (unsigned int i = 0; i < MAX_FILES; i++){
		if (mdfs.files[i].used){
			unsigned int file_sectors = (mdfs.files[i].size + SECTOR_SIZE - 1) / SECTOR_SIZE;
			unsigned int start_sctr = dir_entries[i].start_sctr;
			
			for (unsigned int sctr; sctr < file_sectors; sctr++){
				unsigned int sector = start_sctr + sctr;
				void *src = (char *)mdfs.files[i].data + sctr * SECTOR_SIZE;
				unsigned int size = (sctr == file_sectors - 1) ?
					mdfs.files[i].size % SECTOR_SIZE : SECTOR_SIZE;
					
				if (size < SECTOR_SIZE){
					unsigned char buffer[SECTOR_SIZE];
					setmemory(buffer, 0, SECTOR_SIZE);
					copymemory(buffer, src, size);
					
					wr_sector(sector, buffer);
				}
				else {
					wr_sector(sector, src);
				}
			}
		}
	}
	
	push_text("\nFilesystem saved successfully!");

	asm volatile("sti");
}

void lddsk(const char * dev_name){
	
	asm volatile ("cli");

	if (get_dev(dev_name) == -1 || !storage_devices[get_dev(dev_name)].available){
		push_text("\nError - device not available!");
		asm volatile ("sti");
		return;
	}
	
	struct Superblock sb;
	rd_sector(SUPERBLOCK_SECTOR, &sb);
	
	push_char('\n');
	
	for (int i = 0; i <= 3; i++){
		push_text("|");
		push_char(sb.magic[i]);
		push_text("| ");
	}
	
	if (sb.magic[0] != 'M' && sb.magic[1] != 'D' && sb.magic[2] != 'F' && sb.magic[3] != 'S'){
		push_text("\nInvalid filesystem! Format disc first!");
		asm volatile ("sti");
		return;
	}
	
	mdfs_ini();
	
	static unsigned char sector_buf[SECTOR_SIZE] __attribute__((aligned(4)));
	const int ent_per_sectr = SECTOR_SIZE / sizeof(struct DirEntry); //12 ;)
	
	setmemory(dir_entries, 0, sizeof(dir_entries));
	setmemory(sector_buf, 0, sizeof(sector_buf));

	
	for (int i = 0; i < DIR_SECTORS; i++){
		ide_reset();
		rd_sector(ROOT_DIR_SECTOR + i, sector_buf);
		
		int ent_to_copy = ent_per_sectr;
		if (i == DIR_SECTORS - 1){
			ent_to_copy = MAX_FILES - (i * ent_per_sectr);
			if (ent_to_copy < 0) ent_to_copy = 0;
			if (ent_to_copy > ent_per_sectr) ent_to_copy = ent_per_sectr;
		}
		
		for (int j = 0; j < ent_to_copy; j++){
			unsigned int offset_in_sectr = j * sizeof(struct DirEntry);
			unsigned int index_in_arr = (i * ent_per_sectr) + j;
			
			copymemory((unsigned char*)&dir_entries[index_in_arr], sector_buf + offset_in_sectr, sizeof(struct DirEntry));
		}
	}
	
	for (int i = 0; i < MAX_FILES; i++){
		if (dir_entries[i].size > 0){
			if (strlen(dir_entries[i].name) > 0){
				
				strnumbercopy(mdfs.files[i].name, dir_entries[i].name, MAX_FILENAME_LEN);
			
				mdfs.files[i].size = dir_entries[i].size;
			
				mdfs.files[i].used = 1;
				mdfs.file_count++;
				
				if (dir_entries[i].size > MAX_FILE_SIZE){
					push_text("\n[ File ");
					push_text(dir_entries[i].name);
					push_text("too big file, skipping data...]\n");
					mdfs.files[i].size = 1;
					continue;
				}
			    
				unsigned int sectors = (dir_entries[i].size + SECTOR_SIZE - 1) / SECTOR_SIZE;
				for (int s = 0; s < sectors; s++){
					rd_sector(dir_entries[i].start_sctr + s, mdfs.files[i].data + s * SECTOR_SIZE);
				}
			}
			else {
				continue;
			}
		}
	}
	
	push_text("\nLddsk successfully end work");
	
	asm volatile ("sti");
	
}

int chkdsk(){
	outb(IDE_DRIVE_SEL, 0xE0);
	outb(IDE_SCTR_CNT, 0);
	outb(IDE_LBA_LOW, 0);
	outb(IDE_LBA_MID, 0);
	outb(IDE_LBA_HIGH, 0);
	outb(IDE_COM, 0xEC);
	
	if (inb(IDE_STATUS) == 0){
		return 0;
	}
	
	while (1) {
		unsigned char status = inb(IDE_STATUS);
		if (status & ST_DRQ) {
			break;
		} 
		if (status & ((inb(IDE_ERR)) | 0x01)) {
			return 0;
		}
	}
	
	return 1;
}

void devdetect(){
	
	setmemory(&storage_devices, 0, sizeof(storage_devices));
	dev_count = 0;
	
	storage_devices[dev_count] = (StDev){
		.name = "isovirt",
		.type = DEV_TYPE_VIRTUAL,
		.available = 1,
		.readonly = 1
	};
	dev_count++;
	
	if (chkide()){
		storage_devices[dev_count] = (StDev){
			.name = "ide0",
			.type = DEV_TYPE_IDE,
			.available = 1,
			.readonly = 0
		};
		dev_count++;
	}
}

int chkide(){
	outb(IDE_DRIVE_SEL, 0xA0);
	return (inb(IDE_STATUS) != 0xFF);
}

int get_dev(const char * name){
	for (int i = 0; i < dev_count; i++){
		if (strnumbereq(storage_devices[i].name, name, sizeof(storage_devices[i].name)) == 0) {
			return i;
		}
	}
	return -1;
}

void lsdevs() {
	push_text("\nAvailable storage devices:\n");
	for (int i = 0; i < dev_count; i++){
		push_text(" - ");
		push_text(storage_devices[i].name);
		push_text(" [");
		
		switch(storage_devices[i].type){
			case DEV_TYPE_IDE:
				push_text("IDE DISK");
				break;
			case DEV_TYPE_RAMDSK:
				push_text("RAMDISK");
				break;
			case DEV_TYPE_VIRTUAL:
				push_text("Virtual disk");
				break;
			default:
				push_text("Unknown device");
		}
		
		if (storage_devices[i].readonly){
			push_text(", Read only");
		}
		if (storage_devices[i].available){
			push_text(", available");
		}
		else {
			push_text(", not available");
		}
		push_text("]\n");
	}
}

void ide_reset(){
	asm volatile("cli");
	outb(IDE_COM, 0x04);
	wait_dsk_ready();
	outb(IDE_COM, 0x00);
	wait_dsk_ready();
	asm volatile("sti");
}

int file_add_data(const char* filename, const char * data, unsigned int size){
	char normalized_name[MAX_FILENAME_LEN];
    strnumbercopy(normalized_name, filename, MAX_FILENAME_LEN);
    
    char* end = normalized_name + strlen(normalized_name) - 1;
    while (end >= normalized_name && (*end == ' ' || *end == '\r' || *end == '\n')) {
        *end = '\0';
		end--;
    }
    
    for (int i = 0; i < MAX_FILES; i++) {
        if (mdfs.files[i].used && streq(mdfs.files[i].name, normalized_name) == 0) {
			unsigned int new_size = mdfs.files[i].size + size;
			
            if (new_size > MAX_FILE_SIZE) {
				push_text("\nError: new file size more that max size.\n");
                return -2;
            }

			copymemory(mdfs.files[i].data + mdfs.files[i].size, data, size);
			
            mdfs.files[i].size = new_size;
			
            return 0;
        }
    }
	
	push_text("\nError: file not found.\n");
    return -1;
}

void interpret_program(unsigned char *program){
	
	setmemory(input_buf, 0, sizeof(input_buf));
	setmemory(input_buf_exec, 0, sizeof(input_buf));
	
	Executor state;
	state.program = program;
	state.program_c = 0;
	setmemory(state.work_stack, 0, sizeof(state.work_stack));
	state.stack_pointer = 0;
	state.flags = 0;
	
	while(1) {
		unsigned char opcode_o_arr[2];
		unsigned char opcode_t_arr[2];
		
		unsigned int int_opcode_h;
		unsigned int int_opcode = 0;
		
		opcode_o_arr[0] = state.program[state.program_c++];
		opcode_o_arr[1] = '\0';
		opcode_t_arr[0] = state.program[state.program_c];
		opcode_t_arr[1] = '\0';
		
		if (isnumber(opcode_o_arr[0])){
			if (isnumber(opcode_t_arr[0]) && opcode_o_arr[0] != '0'){
				strtodig(opcode_t_arr, &int_opcode);
				strtodig(opcode_o_arr, &int_opcode_h);
				
				int_opcode_h = int_opcode_h * 10;
				
				state.program_c++;
			}
			else {
				strtodig(opcode_o_arr, &int_opcode_h);
			}
		}
		else {
			continue;
		}
		if (debug_mode){
			
			char address[10];
			digrostr((state.program_c - 1), address);
			
			push_text("[DEBUG STACK] - ['");
			push_text(state.work_stack);
			push_text("'; command address (for goto) - '");
			push_text(address);
			push_text("', OP: '");
			push_text(opcode_o_arr);
			push_text(opcode_t_arr);
			push_text("']\n");
		}
		switch ((int_opcode_h + int_opcode)){
			case OP_PUSH_CHAR: {
				char value = state.program[state.program_c++];
				state.work_stack[state.stack_pointer++] = value;
				break;
			}
			case OP_PUSH_TEXT: {
				const char *text = (const char *)(state.program + state.program_c);
				while (state.program[state.program_c] != '\0' && state.program[state.program_c] != '\n'){
					state.work_stack[state.stack_pointer++] = state.program[state.program_c++];
				}
				
				state.work_stack[state.stack_pointer++] = '\n';
				
				state.program_c++;
				break;
			}
			case OP_INPUT: {
				
				setmemory(input_buf, 0, sizeof(input_buf));
				setmemory(input_buf_exec, 0, sizeof(input_buf));
				
				input_mode = 1;
				input_exec_ready = 0;
				buf_id = 0;
				input_buf[0] = '\0';
				
				idt_ini();
				
				push_text("> ");
				
				pic_remap();
				
				while (!input_exec_ready){
					asm volatile("hlt");
				}
				input_exec_ready = 0;
				input_mode = 0;
				
				for (char *p = input_buf_exec; *p != '\0'; p++){
					state.work_stack[state.stack_pointer++] = *p;
				}
				state.work_stack[state.stack_pointer++] = '\0';
				push_char('\n');
				
				break;
			}
			case OP_EXIT: {
				input_mode = 0;
				return;
			}
			case OP_SYS_EXEC: {
				const char *text = (const char *)(state.program + state.program_c);
				check_comm(text);
			}
			case OP_PRINT: {
				int i = 0;
				while (i < state.stack_pointer){
					push_char(state.work_stack[i]);
					i++;
				}
				state.stack_pointer = 0;
				break;
			}
			case OP_CLEAR_BUF: {
				
				for (int i = 0; i < sizeof(state.work_stack); i++){
					state.work_stack[i] = 0;
				}
				state.stack_pointer = 0;
				break;
			}
			case OP_CALC: {
				if (state.stack_pointer < 3) {
					push_text("Run time error! Stack underflow!\nExit the program...\n");
					input_mode = 0;
					return;
				}
				char num1_str[32] = {0};
				char num2_str[32] = {0};
				char op = 0;

				int num1_len = 0;
				int num2_len = 0;
    
				int sp = state.stack_pointer - 1;

				while (sp >= 0 && (state.work_stack[sp] == '\0' || state.work_stack[sp] == ' ')) {
					sp--;
				}
				while (sp >= 0 && isnumber(state.work_stack[sp])) {
					num2_str[num2_len++] = state.work_stack[sp];
					sp--;
				}
				
				for (int i = 0; i < num2_len / 2; i++) {
					char temp = num2_str[i];
					num2_str[i] = num2_str[num2_len - 1 - i];
					num2_str[num2_len - 1 - i] = temp;
				}
				
				while (sp >= 0 && (state.work_stack[sp] == ' ' || state.work_stack[sp] == '\0')) {
					sp--;
				}
				
				if (sp >= 0) {
					op = state.work_stack[sp];
					sp--;
				}
				
				while (sp >= 0 && (state.work_stack[sp] == ' ' || state.work_stack[sp] == '\0')) {
					sp--;
				}
				
				while (sp >= 0 && isnumber(state.work_stack[sp])) {
					num1_str[num1_len++] = state.work_stack[sp];
					sp--;
				}
				
				for (int i = 0; i < num1_len / 2; i++) {
					char temp = num1_str[i];
					num1_str[i] = num1_str[num1_len - 1 - i];
					num1_str[num1_len - 1 - i] = temp;
				}
				
				if (op == 0) {
					push_text("\nRun time error: No operator found!\n");
					input_mode = 0;
					return;
				}
				
				int num1 = 0, num2 = 0;
				for (int i = 0; i < num1_len; i++) {
					num1 = num1 * 10 + (num1_str[i] - '0');
				}
				
				if (state.work_stack[sp] == '-'){
					num1 = -num1;
					sp--;
				}
				else{
					asm("hlt");
				}
				
				for (int i = 0; i < num2_len; i++) {
					num2 = num2 * 10 + (num2_str[i] - '0');
				}
				
				int answer;
				switch (op) {
					case '+': 
						answer = num1 + num2;
						break;
					case '-': 
						answer = num1 - num2;
						break;
					case '*': 
						answer = num1 * num2;
						break;
					case '/': 
						if (num2 == 0) {
							push_text("\nRun time error! Division by zero!\nExit the program...\n");
							input_mode = 0;
							return;
						}
						answer = num1 / num2;
						break;
					default:
						push_text("\nRun time error! Unknown operator (");
						push_char(op);
						push_text(")\nExit the program...\n");
						input_mode = 0;
						return;
				}
				
				state.stack_pointer = sp + 1;
				
				if (answer < 0){
					state.work_stack[state.stack_pointer++] = '-';
					answer = -answer;
				}
				
				char strans[32];
				int temp = answer;
				int digits = 0;
				
				do {
					digits++;
					temp /= 10;
				} while (temp > 0);
				
				temp = answer;
				for (int i = digits - 1; i >= 0; i--) {
					strans[i] = '0' + (temp % 10);
					temp /= 10;
				}
				strans[digits] = '\0';
				
				for (int i = 0; i < digits; i++) {
					state.work_stack[state.stack_pointer++] = strans[i];
				}
    
				break;

			}
			case OP_GOTO:{
				state.program_c++;
				
				char addr_str[8] = {0};
				int addr_len = 0;
				int sp = state.stack_pointer - 1;

				while (sp >= 0 && (state.work_stack[sp] == '\0' || state.work_stack[sp] == ' ')) {
					sp--;
				}
				while (sp >= 0 && isnumber(state.work_stack[sp])) {
					addr_str[addr_len++] = state.work_stack[sp];
					sp--;
				}
				for (int i = 0; i < addr_len / 2; i++){
					char temp = addr_str[i];
					addr_str[i] = addr_str[addr_len -1 -i];
					addr_str[addr_len -1 -i] = temp;
				}
				
				strtodig(addr_str, &state.program_c);
				if (state.program_c > MAX_FILE_SIZE){
					state.program_c = 0;
				}
				state.stack_pointer = sp + 1;
				break;
			}
			case OP_ABS_GOTO: {
				state.program_c = 0;
				break;
			}
			case OP_EQ: {
				if (state.stack_pointer < 3) {
					push_text("Run time error! Stack underflow!\nExit the program...\n");
					input_mode = 0;
					return;
				}
				char num1_str[32] = {0};
				char num2_str[32] = {0};
				char op = 0;

				int num1_len = 0;
				int num2_len = 0;
    
				int sp = state.stack_pointer - 1;
				
				while (sp >= 0 && (state.work_stack[sp] == '\0' || state.work_stack[sp] == ' ')) {
					sp--;
				}
				
				while (sp >= 0 && isnumber(state.work_stack[sp])) {
					num2_str[num2_len++] = state.work_stack[sp];
					sp--;
				}
				
				for (int i = 0; i < num2_len / 2; i++) {
					char temp = num2_str[i];
					num2_str[i] = num2_str[num2_len - 1 - i];
					num2_str[num2_len - 1 - i] = temp;
				}
				
				while (sp >= 0 && (state.work_stack[sp] == ' ' || state.work_stack[sp] == '\0')) {
					sp--;
				}
				
				if (sp >= 0) {
					op = state.work_stack[sp];
					sp--;
				}
				
				while (sp >= 0 && (state.work_stack[sp] == ' ' || state.work_stack[sp] == '\0')) {
					sp--;
				}
				
				while (sp >= 0 && isnumber(state.work_stack[sp])) {
					num1_str[num1_len++] = state.work_stack[sp];
					sp--;
				}
				
				for (int i = 0; i < num1_len / 2; i++) {
					char temp = num1_str[i];
					
					num1_str[i] = num1_str[num1_len - 1 - i];
					num1_str[num1_len - 1 - i] = temp;
				}
				
				int num1 = 0, num2 = 0;
				
				for (int i = 0; i < num1_len; i++) {
					num1 = num1 * 10 + (num1_str[i] - '0');
				}
				
				for (int i = 0; i < num2_len; i++) {
					num2 = num2 * 10 + (num2_str[i] - '0');
				}
				
				int result;
				switch (op) {
					case '=': 
						result = (num1 == num2);
						break;
					case '>': 
						result = (num1 > num2);
						break;
					case '<': 
						result = (num1 < num2);
						break;
					case '!': 
						result = (num1 != num2);
						break;
					default:
						push_text("\nRun time error! Unknown comparison operator:(");
						push_char(op);
						push_text(")\n");
						input_mode = 0;
						return;
				}
				state.stack_pointer = sp + 1;
    
                //1 - eq; 0 - not eq
				state.work_stack[state.stack_pointer++] = result ? '1' : '0';
    
				break;
			}
			case OP_JMP_IF_TRUE: {
				if (state.stack_pointer < 1) {
					push_text("Run time error! Stack underflow!\nExit the program...\n");
					input_mode = 0;
					return;
				}
				
				state.program_c++;
				char condition = state.work_stack[--state.stack_pointer];
    
				if (condition == '1') {
					state.program_c++;
					unsigned int address = 0;
					while (state.program_c < MAX_FILE_SIZE && isnumber(state.program[state.program_c])) {
						address = address * 10 + (state.program[state.program_c] - '0');
						state.program_c++;
					}
					
					if (state.program_c > MAX_FILE_SIZE){
						state.program_c = 0;
					}
        
					state.program_c = address;
				} else {
					state.program_c++; 
					while (state.program_c < MAX_FILE_SIZE && isnumber(state.program[state.program_c])) {
						state.program_c++;
					}
				}
				break;
			}
			case OP_JMP_IF_FALSE: {
				if (state.stack_pointer < 1) {
					push_text("Run time error! Stack underflow!\nExit the program...\n");
					input_mode = 0;
					return;
				}
				state.program_c++;
				char condition = state.work_stack[--state.stack_pointer];
    
				if (condition == '0') {
					state.program_c++;
					unsigned int address = 0;
					while (state.program_c < MAX_FILE_SIZE && isnumber(state.program[state.program_c])) {
						address = address * 10 + (state.program[state.program_c] - '0');
						state.program_c++;
					}
					
					if (state.program_c > MAX_FILE_SIZE){
						state.program_c = 0;
					}
        
					state.program_c = address;
				} else {
					state.program_c++; 
					while (state.program_c < MAX_FILE_SIZE && isnumber(state.program[state.program_c])) {
						state.program_c++;
					}
				}
				break;
			}
			case OP_ABS: {
				char num1_str[32] = {0};
				char op;
				
				int num1_len = 0;
				int sp = state.stack_pointer - 1;

				while (sp >= 0 && (state.work_stack[sp] == '\0' || state.work_stack[sp] == ' ')) {
					sp--;
				}
				while (sp >= 0 && isnumber(state.work_stack[sp])) {
					num1_str[num1_len++] = state.work_stack[sp];
					sp--;
				}
				
				for (int i = 0; i < num1_len / 2; i++) {
					char temp = num1_str[i];
					num1_str[i] = num1_str[num1_len - 1 - i];
					num1_str[num1_len - 1 - i] = temp;
				}
				
				if (sp >= 0) {
					op = state.work_stack[sp];
					sp--;
				}
				
				int num1 = 0;
				
				for (int i = 0; i < num1_len; i++) {
					num1 = num1 * 10 + (num1_str[i] - '0');
				}
				
				if (op == '-'){
					num1 = -num1;
				}
				
				if (num1 < 0){
					num1 = -num1;
				}
				else{
					num1 = num1;
				}
				
				state.stack_pointer = sp + 1;
				
				char strans[32];
				int temp = num1;
				int digits = 0;
				
				do {
					digits++;
					temp /= 10;
				} while (temp > 0);
				
				temp = num1;
				for (int i = digits - 1; i >= 0; i--) {
					strans[i] = '0' + (temp % 10);
					temp /= 10;
				}
				strans[digits] = '\0';
				
				for (int i = 0; i < digits; i++) {
					state.work_stack[state.stack_pointer++] = strans[i];
				}
    
				break;
				
			}
			case OP_DELAY: {
				char num1_str[32] = {0};
				
				int num1_len = 0;
				int sp = state.stack_pointer - 1;

				while (sp >= 0 && (state.work_stack[sp] == '\0' || state.work_stack[sp] == ' ')) {
					sp--;
				}
				while (sp >= 0 && isnumber(state.work_stack[sp])) {
					num1_str[num1_len++] = state.work_stack[sp];
					sp--;
				}
				
				for (int i = 0; i < num1_len / 2; i++) {
					char temp = num1_str[i];
					num1_str[i] = num1_str[num1_len - 1 - i];
					num1_str[num1_len - 1 - i] = temp;
				}
				
				int num1 = 0;
				
				for (int i = 0; i < num1_len; i++) {
					num1 = num1 * 10 + (num1_str[i] - '0');
				}
				
				slp(num1);
				push_text(num1_str);
				break;
				
				
			}
			case OP_RAW_TIME: {
				unsigned long unixt = get_unix_t();
				
				char time_str[16];
				digrostr(unixt, time_str);
				
				for (int i = 0; time_str[i] != '\0'; i++){
					state.work_stack[state.stack_pointer++] = time_str[i];
				}
				break;
				
			}
			case OP_SET_REG: {
				state.program_c++;
				int reg_num = state.program[state.program_c] - '0';
				if (reg_num >= 0 && reg_num <= 7){
					strnumbercopy(state.registers[reg_num], state.work_stack, 16);
					if (state.stack_pointer >= 16){
						state.stack_pointer -= 16;
					}
					else {
						state.stack_pointer = 0;
					}
				}
				else {
					push_text("\nRun time error! Unknown register number!\nExit the program...\n");
					input_mode = 0;
					return;
				}
				break;
			}
			case OP_LOAD_REG: {
				state.program_c++;
				int reg_num = state.program[state.program_c] - '0';
				if (reg_num >= 0 && reg_num <= 7){
					state.stack_pointer = 0;
					for (int i = 0; state.registers[reg_num][i] != '\0'; i++){
						state.work_stack[state.stack_pointer++] = state.registers[reg_num][i];
					}
				}
				else {
					push_text("\nRun time error! Unknown register number!\nExit the program...\n");
					input_mode = 0;
					return;
				}
				break;
			}
			default:
				push_text("\nRun time error! Unknown operation code!\nExit the program...\n");
				input_mode = 0;
				return;
		}
		if (state.stack_pointer >= sizeof(state.work_stack)) {
			push_text("\nRun time error! Stack overflow!\nExit the program...\n");
			input_mode = 0;
			return;
		}
	}
}

int file_rnm(const char* old_filename, const char* new_filename){
	for (int i = 0; i < MAX_FILES; i++){
		if (mdfs.files[i].used && streq(mdfs.files[i].name, old_filename) == 0) {
			for (int j = 0; j < MAX_FILES; j++){
				if (mdfs.files[j].used && streq(mdfs.files[j].name, new_filename) == 0){
					goto EXIT_ST_ONE;
				}
			}
			strnumbercopy(mdfs.files[i].name, new_filename, MAX_FILENAME_LEN);
			
			push_text("\nFile successfully renamed from ");
			push_text(old_filename);
			push_text(" to ");
			push_text(new_filename);
			push_text(".");
			
			return 0;
			
		}
	}
	
	push_text("\nFile ");
	push_text(old_filename);
	push_text(" not found!");
	return -2;
	
	EXIT_ST_ONE:
		push_text("\nFilename ");
		push_text(new_filename);
		push_text(" is used before!");
	    return -1;
}

void hltmode(){
	push_text("\nNow system entering sleep mode. Press ESC to wake up...\n");
	
	asm volatile("sti");
	
	unsigned char scancode = 0;
	hlt_input_mode = 1;
	
	idt_ini();
	pic_remap();
	while(hlt_input_mode){
		asm volatile ("hlt");
	}
	
	push_text("System waking up...");
}

void set_rtc_time(RTC_Time* time){
	
	unsigned char prev = inb(0x70);
	outb(0x70, prev | 0x80);
	
	write_cmos(0x00, bin_to_bcd(time->second));
	write_cmos(0x02, bin_to_bcd(time->minute));
	write_cmos(0x04, bin_to_bcd(time->hour));
	write_cmos(0x07, bin_to_bcd(time->day));
	write_cmos(0x08, bin_to_bcd(time->month));
	write_cmos(0x09, bin_to_bcd(time->year));
	
	outb(0x70, prev);
}

int is_vis_year(int year){
	return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

void unixt_to_date(unsigned long unixt, int *second, int *minute, int *hour, int *day, int *month, int *year){
	const int SECONDS_PER_DAY = 86400;
	const int SECONDS_PER_HOUR = 3600;
	const int SECONDS_PER_MINUTE = 60;
	
	const int days_arr[2][12] = {
		{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
		{31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
	};
	
	unsigned long days = unixt / SECONDS_PER_DAY;
	*second = unixt % SECONDS_PER_DAY;
	
	*hour = *second / SECONDS_PER_HOUR;
	*second %= SECONDS_PER_HOUR;
	
	*minute = *second / SECONDS_PER_MINUTE;
	*second %= SECONDS_PER_MINUTE;
	
	*year = 1970;
	while (1) {
		int days_in_year = is_vis_year(*year) ? 366 : 365;
		if (days < days_in_year) break;
		days -= days_in_year;
		(*year)++;
	}
	
	int vis = is_vis_year(*year);
	*month = 0;
	while (days >= days_arr[vis][*month]){
		days -= days_arr[vis][*month];
		(*month++);
	}
	*day = days + 1;
	(*month)++;
}