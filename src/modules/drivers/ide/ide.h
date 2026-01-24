#ifndef IDE_H
#define IDE_H

#include "../../drivers/io/io.h"
#include "../../drivers/vga/vga.h"
#include "../../sys/mdfs/mdfs.h"
#include "../../mdstr/mdstr.h"

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

// Dir Entry - name, start sector, size and reserved size

struct DirEntry {
	char name[28];
	unsigned int start_sctr;
	unsigned int size;
	unsigned int reserved;
};

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

// Superblock - magic fs name (MDFS), version MDFS, sector size, count inodes, bit card (free blocks) and reserved bytes for size Superblock is 512 bytes

struct Superblock {
	char magic[4];
	unsigned short version;
	unsigned short sector_sz;
	unsigned int inode_count;
	unsigned int free_blocks[64];
	unsigned int reserved[61];
};

static StDev storage_devices[MAX_DEVICES];
static unsigned char dev_count = 0;

static int in_format = 0;
static struct DirEntry dir_entries[MAX_FILES] __attribute__((aligned(4)));

void wait_dsk_ready();
int wait_dsk_drq();

void rd_sector(unsigned int lba, void* buffer);
void wr_sector(unsigned int lba, const void* buffer);
void mdfs_format(const char * dev_name);

void svdsk();
void lddsk(const char * dev_name);

int chkdsk();
void devdetect();
void lsdevs();
int get_dev(const char* name);

int chkide();
void ide_reset();

#endif