#include "pcsp.h"

void pit_ini(unsigned int frequency){
	if (frequency < 20) frequency = 20;
	if (frequency > 20000) frequency = 20000;
	
	unsigned int divisor = 1193180 / frequency;
	
	outb(0x43, 0xB6);
	outb(0x42, divisor & 0xFF);
	outb(0x42, (divisor >> 8) & 0xFF);
}

void speaker_en(){
	outb(0x61, inb(0x61) | 0x03);
}

void speaker_dis(){
	outb(0x61, inb(0x61) & ~0x03);
}

void beep(unsigned int freq, unsigned int durations){
	pit_ini(freq);
	speaker_en();
	
	slp(durations);
	speaker_dis();
}

void slp(unsigned int ms){
	for (volatile unsigned int i = 0; i < ms * 5000; i++){
		asm volatile("pause");
	}
}