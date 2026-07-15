#ifndef MDSTR_H
#define MDSTR_H

int streq(const char* str1, const char* str2);
int strnumbereq(const char* str1, const char* str2, int number);
char* strsep(const char *str, int c);
void strcopy(char * dest, const char * src);
void strct(char * dest, const char * src);
void strnumbercopy(char *dest, const char *src, unsigned int n);
unsigned int strsz(const char *str);
char *strtok(char *str, const char *delim);
char strchar(const char* str, char c);

int issymbolspace(char symbol);
int isnumber(char symbol);
int issymbolletter(char c);
int issymbolletterlower(char c);
int issymbolletterupper(char c);

void digtostr(unsigned number, char * buffer);
void lgtostr(unsigned long number, char * buffer);
int strtodig(const char *str, int *result);
int strtoudig(const char *str, unsigned int *result);
int hextodig(const char *str);

char *lowerl(char *str);
char lowerlc(char ch);

void eng_ces_ciph(char *text, int shift, int dir);

#endif