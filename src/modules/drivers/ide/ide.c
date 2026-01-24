#include "ide.h"

extern MDFS mdfs;

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
	
	unsigned int file_count = 0;
	for (int i = 0; i < MAX_FILES; i++) {
		if (mdfs.files[i].used){
			file_count++;
		}
	}
	
	struct Superblock sb = {
		.magic = "MDFS",
		.version = 1,
		.sector_sz = SECTOR_SIZE,
		.inode_count = file_count
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
			if (strsz(dir_entries[i].name) > 0){
				
				strnumbercopy(mdfs.files[i].name, dir_entries[i].name, MAX_FILENAME_LEN);
			
				mdfs.files[i].size = dir_entries[i].size;
			
				mdfs.files[i].used = 1;
				
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
