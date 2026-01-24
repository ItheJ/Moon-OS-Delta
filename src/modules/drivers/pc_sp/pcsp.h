#ifndef PCSP_H
#define PCSP_H

#include "../../drivers/io/io.h"

void pit_ini(unsigned int frequency); //initialisation PIT
void speaker_en(); //turn on speaker
void speaker_dis(); //turn off speaker

void beep(unsigned int freq, unsigned int durations); // create sound

void slp(unsigned int ms);

#endif