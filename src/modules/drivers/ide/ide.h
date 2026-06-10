#ifndef IDE_H
#define IDE_H

#include "../../drivers/io/io.h"
#include "../../drivers/vga/vga.h"
#include "../../sys/mdfs/mdfs.h"
#include "../../mdstr/mdstr.h"

#define IDE_PR_BS 0x1F0
#define IDE_SL_BS 0x170
#define IDE_PR_CR 0x3F6
#define IDE_SL_CR 0x376

#define COMM_RD 0x20
#define COMM_WR 0x30
#define ST_BSY 0x80
#define ST_DRDY 0x40
#define ST_DRQ 0x08
#define ST_ERR 0x01

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

typedef struct {
	unsigned short base;
	unsigned short contrl;
	unsigned char irq;
	unsigned char slave;
} IDE_channel;

typedef enum {
	DEV_TYPE_NONE,
	DEV_TYPE_IDE
} DevType;

typedef struct {
	char name[13];
	DevType type;
	unsigned char available;
	unsigned char readonly;
	unsigned char number;
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

static IDE_channel ide_devs[4] = {
	{IDE_PR_BS, IDE_PR_CR, 14, 0},
	{IDE_PR_BS, IDE_PR_CR, 14, 1},
	{IDE_SL_BS, IDE_SL_CR, 15, 0},
	{IDE_SL_BS, IDE_SL_CR, 15, 1}
};

static StDev storage_devices[MAX_DEVICES];
static unsigned char dev_count = 0;

static int in_format = 0;
static struct DirEntry dir_entries[MAX_FILES] __attribute__((aligned(4)));

int wait_dsk_ready(IDE_channel *dev);
int wait_dsk_drq(IDE_channel *dev);

int rd_sector(IDE_channel *dev, unsigned int lba, void* buffer);
int wr_sector(IDE_channel *dev, unsigned int lba, const void* buffer);
void mdfs_format(const char *dev_name);

void svdsk();
void lddsk(const char * dev_name);

int chkdsk(IDE_channel *dev);
void devdetect();
void lsdevs();
int get_dev(const char* name);

int chkide(unsigned char dev_id);
void ide_reset(IDE_channel *dev);

void io_delay();
int check_mdfs(IDE_channel *dev);
int dsk_fflush(IDE_channel *dev);

#endif