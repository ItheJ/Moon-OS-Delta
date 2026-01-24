#ifndef TIME_H
#define TIME_H

#include "../drivers/vga/vga.h"
#include "../sys/cmos/cmos.h"
#include "../mdstr/mdstr.h"

//structs for work with time
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

void push_time();
void push_date();
void push_unix_t();
void set_rtc_time(RTC_Time* time);
int is_vis_year(int year);
void unixt_to_date(unsigned long unixt, int *second, int *minute, int *hour, int *day, int *month, int *year);

Time get_t();
unsigned int get_unix_t();

#endif