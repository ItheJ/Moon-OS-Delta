#include "ide.h"

extern MDFS mdfs;

int wait_dsk_ready(IDE_channel *dev){
	int timeout = 5000000;
	while (timeout--) {
		unsigned char status = inb(dev->base + 7);
		if (!(status & 0x80)) {
			if (status & 0x40) return 1; 
			else return 0;
		}
		asm volatile("pause");
	}
	return 0;
}

int wait_dsk_drq(IDE_channel *dev){
	int timeout = 1000000;
	while (timeout--) {
		unsigned char status = inb(dev->base + 7);
		if (status & 0x01) return -1;
		if (!(status & 0x80) && (status & 0x08)) return 1;
		asm volatile("pause");
	}
	return 0;
}

int rd_sector(IDE_channel *dev, unsigned int lba, void* buffer){
	if (inb(dev->base + 7) & 0x80){
		push_text("BUSY!\n");
		return -1;
	}
	asm volatile ("cli");
	
	outb(dev->base + 2, 1);
    io_delay();

    outb(dev->base + 3, (unsigned char)(lba));
    io_delay();
    outb(dev->base + 4, (unsigned char)(lba >> 8));
    io_delay();
    outb(dev->base + 5, (unsigned char)(lba >> 16));
    io_delay();

    unsigned char drive = 0xE0 | (dev->slave << 4) | ((lba >> 24) & 0x0F);
    outb(dev->base + 6, drive);
    io_delay();

    outb(dev->base + 7, COMM_RD);
    io_delay();

    int drq_status = wait_dsk_drq(dev);
    if (drq_status != 1) {
        if (drq_status == -1) push_text("\nError: error bit setted\n");
        else push_text("\nError: time out waiting for DRQ\n");
        asm volatile("sti");
        return -1;
    }

    unsigned short *ptr = (unsigned short*)buffer;
    for (int i = 0; i < 256; i++) {
        ptr[i] = inw(dev->base); // data port
    }

    if (!wait_dsk_ready(dev)) {
        push_text("\nError: device not ready after read!\n");
        asm volatile("sti");
        return -1;
    }
	
	asm volatile ("sti");
	return 0;
}

int wr_sector(IDE_channel *dev, unsigned int lba, const void* buffer){

	if (inb(dev->base + 7) & 0x80){
		push_text("BUSY!\n");
		return -1;
	}
	asm volatile ("cli");
	
	outb(dev->base + 2, 1);
    io_delay();
    outb(dev->base + 3, (unsigned char)(lba));
    io_delay();
    outb(dev->base + 4, (unsigned char)(lba >> 8));
    io_delay();
    outb(dev->base + 5, (unsigned char)(lba >> 16));
    io_delay();

    unsigned char drive = 0xE0 | (dev->slave << 4) | ((lba >> 24) & 0x0F);
    outb(dev->base + 6, drive);
    io_delay();

    outb(dev->base + 7, COMM_WR);
    io_delay();

    int drq_status = wait_dsk_drq(dev);
    if (drq_status != 1) {
        if (drq_status == -1) push_text("\nError: error bit setted\n");
        else push_text("\nError: time out waiting for DRQ\n");
        asm volatile("sti");
        return -1;
    }

    unsigned short *ptr = (unsigned short*)buffer;
    for (int i = 0; i < 256; i++) {
        outw(dev->base, ptr[i]);
    }

    if (!wait_dsk_ready(dev)) {
        push_text("\nError: device not available after write\n");
        asm volatile("sti");
        return -1;
    }
	
	asm volatile ("sti");
	return 0;
}

void mdfs_format(const char *dev_name) {
    asm volatile("cli");
	
	push_char('\n');
	
    int id = get_dev(dev_name);
	if (get_dev(dev_name) == -1 || !storage_devices[get_dev(dev_name)].available){
		push_text("\nError - device not available!");
		asm volatile ("sti");
		return;
	}
	
    IDE_channel *dev = &ide_devs[id];
    
    if (!chkdsk(dev)) { 
		push_text("\nError - No disk detected!");
		return;
	}
    
    unsigned char sb_buf[512] = {0};
    struct Superblock *sb = (struct Superblock*)sb_buf;
	
	copymemory(sb->magic, "MDFS", 4);
    sb->version = 1;
    sb->sector_sz = 512;
    sb->inode_count = 0;
    sb->free_blocks[0] = 0x3;
    
    if (wr_sector(dev, SUPERBLOCK_SECTOR, sb_buf) != 0) {
        push_text("\nError - write superblock failed!");
        return;
    }
    
    unsigned char check_buf[512];
    if (rd_sector(dev, SUPERBLOCK_SECTOR, check_buf) != 0) {
        push_text("\nError - read back failed");
        return;
    }
    
    if (memoryeq(sb_buf, check_buf, 4) == 0) {
        push_text("\nSUCCESS: Superblock verified");
    } else {
        push_text("\nError - Superblock mismatch");
    }
    
    unsigned char empty[512] = {0};
    for (int i = 0; i < DIR_SECTORS; i++) {
        wr_sector(dev, ROOT_DIR_SECTOR + i, empty);
    }
	
    asm volatile("sti");
}

void svdsk(const char *dev_name){
    asm volatile("cli");

    unsigned int file_count = 0;
    for (int i = 0; i < MAX_FILES; i++) {
        if (mdfs.files[i].used){
			file_count++;
		}
    }

    IDE_channel *dev = &ide_devs[get_dev(dev_name)];

    int mdfs_status = check_mdfs(dev);
    if (mdfs_status == -1) {
        push_text("\nError - reading superblock from device");
        asm volatile("sti");
        return;
    }
    struct Superblock sb;

    if (mdfs_status == 1) {
        if (rd_sector(dev, SUPERBLOCK_SECTOR, &sb) != 0) {
            push_text("\nError - failed to read superblock for update");
            asm volatile("sti");
            return;
        }
        sb.inode_count = file_count;
        push_text("\nUpdating existing MDFS...");
		
    } else {
        push_text("\nError - no MDFS detected, formate disk first!");
        return;
    }
    if (wr_sector(dev, SUPERBLOCK_SECTOR, &sb) != 0) {
        push_text("\nError - failed to write superblock");
        asm volatile("sti");
        return;
    }
    struct DirEntry dir_entries[MAX_FILES];
    setmemory(dir_entries, 0, sizeof(dir_entries));

    unsigned int cur_data_sect = DATA_START_SECTOR;
    for (unsigned int i = 0; i < MAX_FILES; i++) {
        if (mdfs.files[i].used) {
            strnumbercopy(dir_entries[i].name, mdfs.files[i].name, MAX_FILENAME_LEN - 1);
            dir_entries[i].name[MAX_FILENAME_LEN - 1] = '\0';
            dir_entries[i].start_sctr = cur_data_sect;
            dir_entries[i].size = mdfs.files[i].size;
            cur_data_sect += (mdfs.files[i].size + SECTOR_SIZE - 1) / SECTOR_SIZE;
        }
    }

    unsigned int dir_sz = MAX_FILES * sizeof(struct DirEntry);
    unsigned int dir_sectr_count = (dir_sz + SECTOR_SIZE - 1) / SECTOR_SIZE;
    for (unsigned int i = 0; i < dir_sectr_count; i++) {
        unsigned int sector = ROOT_DIR_SECTOR + i;
        void *src = (char*)dir_entries + i * SECTOR_SIZE;
        if (wr_sector(dev, sector, src) != 0) {
            push_text("\nError - failed to write directory sector");
            asm volatile("sti");
            return;
        }
    }
    for (unsigned int i = 0; i < MAX_FILES; i++) {
        if (mdfs.files[i].used) {
            unsigned int file_sectors = (mdfs.files[i].size + SECTOR_SIZE - 1) / SECTOR_SIZE;
            unsigned int start_sctr = dir_entries[i].start_sctr;

            for (unsigned int s = 0; s < file_sectors; s++) {
                unsigned int sector = start_sctr + s;
                void *src = (char*)mdfs.files[i].data + s * SECTOR_SIZE;
                unsigned int size = (s == file_sectors - 1) ?
                    (mdfs.files[i].size % SECTOR_SIZE) : SECTOR_SIZE;

                if (size < SECTOR_SIZE) {
                    unsigned char buffer[SECTOR_SIZE];
                    setmemory(buffer, 0, SECTOR_SIZE);
                    copymemory(buffer, src, size);
					if (wr_sector(dev, sector, buffer) != 0) {
                        push_text("\nError - failed to write file data");
                        asm volatile("sti");
                        return;
                    }
                } else {
                    if (wr_sector(dev, sector, src) != 0) {
                        push_text("\nError - failed to write file data");
                        asm volatile("sti");
                        return;
                    }
                }
            }
        }
    }
	
	if (dsk_fflush(dev) != 0){
		push_text("\nWarning - filesystem saved, cache flush failed, data may be unsafe");
	} else {
		push_text("\nFilesystem saved successfully!");
	}
    
    asm volatile("sti");
}

void lddsk(const char * dev_name){
	
	asm volatile ("cli");

	if (get_dev(dev_name) == -1){
		push_text("\nError - device not available!");
		asm volatile ("sti");
		return;
	}
	
	IDE_channel *dev = &ide_devs[get_dev(dev_name)];
	
	struct Superblock sb;
	rd_sector(dev, SUPERBLOCK_SECTOR, &sb);
	
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
		ide_reset(dev);
		rd_sector(dev, ROOT_DIR_SECTOR + i, sector_buf);
		
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
					rd_sector(dev, dir_entries[i].start_sctr + s, mdfs.files[i].data + s * SECTOR_SIZE);
				}
			}
			else {
				continue;
			}
		}
	}
	
	push_text("\nLdide successfully end work");
	
	asm volatile ("sti");
}

int chkdsk(IDE_channel *dev){
	unsigned char sel_drv = (dev->slave ? 0xB0 : 0xA0);
    outb(dev->base + 6, sel_drv);
    
    outb(dev->base + 2, 0);
    outb(dev->base + 3, 0);
    outb(dev->base + 4, 0);
    outb(dev->base + 5, 0);
    outb(dev->base + 7, 0xEC);
    
    unsigned char status = inb(dev->base + 7);
    if (status == 0) {
        return 0;
    }
    
    while (1) {
        status = inb(dev->base + 7);
        if (status & ST_DRQ) {
            break;
        }
        if (status & (ST_ERR | 0x01)) {
            return 0;
        }
    }
    
    for(int i = 0; i < 256; i++) {
        inw(dev->base);
    }
    return 1;
}

void devdetect(){
	push_text("[DETECTING IDE DEVICES...]\n");
	
	setmemory(&storage_devices, 0, sizeof(storage_devices));
	dev_count = 0;
	
	char ide_name[5];
	ide_name[0] = 'i';
	ide_name[1] = 'd';
	ide_name[2] = 'e';
	ide_name[3] = '\0';
	ide_name[4] = '\0';
	
	for (int i = 0; i < 4; i++){
		if (chkide(i)){
			char dsknum[2];
			
			digtostr(i, dsknum);
			ide_name[3] = dsknum[0];
			
			storage_devices[dev_count] = (StDev){
				.type = DEV_TYPE_IDE,
				.available = 1,
				.readonly = 0,
				.number = i
			};
			
			for (int i = 0; i < 5; i++){
				storage_devices[dev_count].name[i] = ide_name[i];
			}
			
			push_text("Found IDE dev: ");
			push_text(ide_name);
			push_char('\n');
			dev_count++;
		}
	}
}

int chkide(unsigned char dev_id){
	IDE_channel *dev = &ide_devs[dev_id];
	
	unsigned char sel_drv = 0xA0 | (dev->slave << 4);
	outb(dev->base + 6, sel_drv);
	
	for (int i = 0; i < 4; i++){
		inb(dev->base + 7);
	}
	
	outb(dev->base + 2, 0x55);
	outb(dev->base + 3, 0xAA);
	
	if(inb(dev->base + 2) == 0x55 && inb(dev->base + 3) == 0xAA){
		return 1;
	}
	return 0;
}

int get_dev(const char *name){
	if (streq(name, "ide0") == 0) return 0;
	if (streq(name, "ide1") == 0) return 1;
	if (streq(name, "ide2") == 0) return 2;
	if (streq(name, "ide3") == 0) return 3;
	return -1;
}

void lsdevs() {
	push_text("\nAvailable ide disks:\n");
	for (int i = 0; i < dev_count; i++){
		push_text(" - ");
		push_text(storage_devices[i].name);
		push_text(" [");
		
		switch(storage_devices[i].type){
			case DEV_TYPE_IDE:
				push_text("IDE DISK");
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

void ide_reset(IDE_channel *dev){
	asm volatile("cli");
	
	outb(dev->contrl, 0x04);
	
	for (int i = 0; i < 5; i++){
		inb(dev->contrl);
	}
	
	outb(dev->contrl, 0x00);
	
	if (wait_dsk_ready(dev) != 0){
		asm volatile("sti");
		return;
	}
	
	asm volatile("sti");
}

void io_delay(){
	asm volatile("outb %%al, $0x80" : : "a"(0));
}

int check_mdfs(IDE_channel *dev){
    struct Superblock sb;
    if (rd_sector(dev, SUPERBLOCK_SECTOR, &sb) != 0) {
        return -1;
    }
    if (sb.magic[0] == 'M' && sb.magic[1] == 'D' && sb.magic[2] == 'F' && sb.magic[3] == 'S') {
        return 1;
    }
    return 0;
}

int dsk_fflush(IDE_channel *dev){
	if (!wait_dsk_ready(dev)){
		return -1;
	}
	outb(dev->base + 7, 0xE7);
	io_delay();
	
	int timeout = 5000000;
	while (timeout--){
		unsigned char status = inb(dev->base + 7);
		if (!(status & 0x80)) {
			return 0;
		}
		asm volatile("pause");
	}
	return -1;
}