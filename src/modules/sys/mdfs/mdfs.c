#include "mdfs.h"

extern MDFS mdfs;

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
			push_text("\nError: this file already exists.");
			return -1;
		}
	}
	
	for (int i = 0; i < MAX_FILES; i++) {
		if (!mdfs.files[i].used){
			strnumbercopy(mdfs.files[i].name, filename, MAX_FILENAME_LEN);
			mdfs.files[i].size = 0;
			mdfs.files[i].used = 1;
			
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
    
    char* end = normalized_name + strsz(normalized_name) - 1;
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
            return 0;
        }
    }
	
	push_text("\nError: file not found.");
    return -1;
}

int file_read(const char *filename, char * buffer, unsigned int buffer_size, unsigned int offset) {
	for (int i = 0; i < MAX_FILES; i++){
		if (mdfs.files[i].used && streq(mdfs.files[i].name, filename) == 0) {
			unsigned int to_copy = (buffer_size < mdfs.files[i].size) ? buffer_size : mdfs.files[i].size;
			if (offset >= mdfs.files[i].size) offset = mdfs.files[i].size - 1;
			copymemory(buffer, mdfs.files[i].data + offset, to_copy);
			return to_copy;
		}
	}
	
	return -1;
}

int file_del(const char *filename){
	for( int i = 0; i < MAX_FILES;  i++){
		if (mdfs.files[i].used && streq(mdfs.files[i].name, filename) == 0) {
			setmemory(&mdfs.files[i], 0, sizeof(FileEntry));
		
			push_text("\nFile successfully deleted.");
			return 0;
		}
	}
	
	push_text("Error: file not found!");
	
	return -1;
}

int files_er(const char *filename){
	for(int i = 0; i < MAX_FILES; i++){
		if (mdfs.files[i].used && streq(mdfs.files[i].name, filename) == 0){
			setmemory(&mdfs.files[i].data, 0, sizeof(mdfs.files[i].data));
			mdfs.files[i].size = 0;
			push_text("\nFile was erased.");
			
			return 0;
		}
	}
	push_text("Error: file not found!");
	
	return -1;
}

void files_list() {
	int is_found = 0;
	
	push_text("\nFiles:\n");
	for (int i = 0; i < MAX_FILES; i++) {
		if (mdfs.files[i].used){
			is_found = 1;
			push_text("  ");
			push_text(mdfs.files[i].name);
			
			char size_str[16];
			digtostr(mdfs.files[i].size, size_str);
			
			push_text(" : ");
			push_text(size_str);
			push_text(" bytes;");
		}
	}
	if (!is_found){
		push_text("No files found");
	}
}

int file_add_data(const char* filename, const char * data, unsigned int size){
	char normalized_name[MAX_FILENAME_LEN];
    strnumbercopy(normalized_name, filename, MAX_FILENAME_LEN);
    
    char* end = normalized_name + strsz(normalized_name) - 1;
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

int memoryeq(const void *s1, const void *s2, unsigned int number){
    const unsigned char *p1 = (const unsigned char *)s1;
    const unsigned char *p2 = (const unsigned char *)s2;

    for (unsigned int i = 0; i < number; i++) {
        if (p1[i] != p2[i]) {
            return (p1[i] < p2[i]) ? -1 : 1;
        }
    }
    return 0;
}