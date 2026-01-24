#include "time.h"

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

void push_time(){
	Time t = get_t();
	
	char time_str[16];
	char hour_str[3], min_str[3], sec_str[3];
	
	push_text("Time: ");
	
	digtostr(t.hour, hour_str);
	digtostr(t.minute, min_str);
	digtostr(t.second, sec_str);
	
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
	
	digtostr(t.day, day_str);
	digtostr(t.month, month_str);
	digtostr(t.year, year_str);
	
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
	digtostr(unix_t, unix_str);
	
	push_text("Unix time: ");
	push_text(unix_str);
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
