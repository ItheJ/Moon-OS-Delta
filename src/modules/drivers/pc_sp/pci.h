#ifndef PCI_H
#define PCI_H

#define CONF_ADDR 0xCF8
#define CONF_DAT 0xCFC

#define MAX_AHCI 4

#define ATA_COMM_IDENT 0xEC
#define ATA_COMM_RD_SECTOR 0x20
#define ATA_COMM_WR_SECTOR 0x30

#define AHCI_CAP 0x00
#define AHCI_GHC 0x04
#define AHCI_PI 0x0C
#define AHCI_VS 0x10

#define PORT_CLB 0x00
#define PORT_CLBU 0x04
#define PORT_FB 0x08
#define PORT_FBU 0x0C
#define PORT_IS 0x10
#define PORT_IE 0x14
#define PORT_COMM 0x18
#define PORT_TDF 0x20
#define PORT_SIG 0x24
#define PORT_SSTS 0x28
#define PORT_SCTL 0x2C
#define PORT_SERR 0x30
#define PORT_CI 0x38

#define COMM_ST (1 << 0)
#define COMM_FRE (1 << 4)
#define COMM_CR (1 << 15)

#include "../../drivers/io/io.h"
#include "../../drivers/vga/vga.h"
#include "../../drivers/ide/ide.h"
#include "../../sys/mdfs/mdfs.h"

typedef struct {
	unsigned short bus;
	unsigned char dev;
	unsigned char func;
	
	unsigned short vend_id;
	unsigned short device_id;
	unsigned char class_code;
	unsigned char subclass;
	unsigned char prog_if;
	
	unsigned char used;
} PCI_channel;

typedef struct {
	PCI_channel pci_ch[256];
} PCI_bus;

typedef struct {
	PCI_channel ahci;
	volatile unsigned int bar5;
	unsigned int number;
} Ahci_dev;

unsigned int read_pci_conf(unsigned char bus, unsigned char dev, unsigned char func, unsigned char reg);
void scan_pci();

void check_ahci_contr(PCI_channel ahci);
void ls_ahci_ports(PCI_channel ahci);
void chk_ahci_ports(PCI_channel ahci);
void ini_ahci_ports(volatile unsigned int *mem_ahci, int port);
int rd_sector_ahci(volatile unsigned int *mem_ahci, int slot, int port, unsigned int lba, void *buffer);
int wr_sector_ahci(volatile unsigned int *mem_ahci, int slot, int port, unsigned int lba, void *buffer);
int ident_ahci(volatile unsigned int *mem_ahci, int slot, int port);

int get_ahci(const char *name);
void ahci_formt(Ahci_dev ahci, int port);

char *class_to_str_pci(unsigned char class, unsigned char subclass);

#endif