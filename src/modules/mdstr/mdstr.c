#include "mdstr.h"

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

void digtostr(unsigned int number, char * buffer) {
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
void strct(char *dest, const char *src){
	while (*dest) dest++;
	while (*src) *dest++ = *src++;
	*dest = '\0';
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

unsigned int strsz(const char *str) {
	unsigned int len = 0;
	while (str[len] && str[len] != '\0') {
		len++;
	}
	return len;
}

char strchar(const char* str, char c) {
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
        if (strchar(delim, *last)) { 
            *last = '\0';
            last++;
            return token;
        }
        last++;
    }

    last = (void*)0;
    return token;
}

int issymbolspace(char symbol){
	if (symbol == ' '){
		return 1;
	}
	return 0;
}

int isnumber(char symbol){
	if (symbol >= '0' && symbol <= '9'){
		return 1;
	}
	return 0;
}

int strtodig(const char *str, int *result) {
    int sign = 1, value = 0, i = 0;
    
    while (issymbolspace(str[i])) i++;
    
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
    
    while (issymbolspace(str[i])) i++;
    
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

void lgtostr(unsigned long number, char * buffer){
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

int hextodig(const char *str){
	int num = 0;
 
	if(str[1] == 'x') {
		for(int i = 2; isnumber(str[i]) || lowerlc(str[i]) >= 'a' && lowerlc(str[i]) <= 'f';i++){
			
			if (isnumber(str[i])) num = num * 16 + (str[i] - '0');
			else{
				num = num * 16 + (10 + lowerlc(str[i]) - 'a');
			}
			
		}
	}
	else {
		for(int i = 0; isnumber(str[i]) || lowerlc(str[i]) >= 'a' && lowerlc(str[i]) <= 'f';i++){
		
			if (isnumber(str[i])) num = num * 16 + (str[i] - '0');
			else{
				num = num * 16 + (10 + lowerlc(str[i]) - 'a');
			}
		}
	}
	return num;
}
char *lowerl(char *str){
	while(*(str)!='\0'){
		*(str) = lowerlc(*(str));
		str++;
	}
	
	return str; 
}

char lowerlc(char ch){
	if((ch >= 'A') && (ch <= 'Z')){
		ch+=32;
	}
	
	return ch;
}

int issymbolletter(char c){
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}
int issymbolletterlower(char c){
    return (c >= 'a' && c <= 'z');
}
int issymbolletterupper(char c){
    return (c >= 'A' && c <= 'Z');
}

void eng_ces_ciph(char *text, int shift, int dir){
    for (int i = 0; text[i] != '\0'; i++) {
        if (issymbolletter(text[i])) {
			
            char base = issymbolletterlower(text[i]) ? 'a' : 'A';
            char ch = text[i] - base;
			
            if (dir == 1) {
                ch = (ch + shift) % 26;
            } else {
                ch = (ch - shift + 26) % 26;
            }
			
            text[i] = base + ch;
        }
    }
}