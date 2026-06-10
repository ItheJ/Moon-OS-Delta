#include "pci.h"

extern PCI_bus pci_bus;

extern unsigned char __attribute__((aligned(1024))) ahci_commlb[1024];
extern unsigned char __attribute__((aligned(256))) ahci_fb[256];
extern unsigned char __attribute__((aligned(128))) ahci_comm_tabl[128];

extern unsigned char buff[512] __attribute__((aligned(4)));

unsigned int read_pci_conf(unsigned char bus, unsigned char dev, unsigned char func, unsigned char reg){
	unsigned int address = (1U << 31) |
						((unsigned int)bus << 16) |
						((unsigned int)dev << 11) |
						((unsigned int)func << 8) |
						(reg & 0xFC);
	outl(CONF_ADDR, address);
	return inl(CONF_DAT);
}

void scan_pci(){
	push_char('\n');
	
	setmemory(&pci_bus, 0, sizeof(pci_bus));
	
	unsigned int pci_cnt = 0;
	
	for (unsigned char bus = 0; bus < 255; bus++){
		for (unsigned char dev = 0; dev < 32; dev++) {
			for (unsigned char func = 0; func < 8; func++) {
				unsigned int vend_dev = read_pci_conf(bus, dev, func, 0);
				unsigned short vend_id = vend_dev & 0xFFFF;
				if (vend_id == 0xFFFF) {
					if (func == 0) break;
					else continue;
				}
				// dev found
				unsigned short device_id = (vend_dev >> 16) & 0xFFFF;
			
				unsigned int class_rev = read_pci_conf(bus, dev, func, 0x08);
				unsigned char class_code = (class_rev >> 24) & 0xFF;
				unsigned char subclass = (class_rev >> 16) & 0xFF;
				unsigned char prog_if = (class_rev >> 8) & 0xFF;
			
				push_format("Found PCI (Bus=%u, Dev=%u, Func=%u):\nVendor=0x%x02, Device=0x%x02, Class=0x%x01, Subclass=0x%x01, ProgIF=0x%x01 \n", bus, dev, func, vend_id, device_id, class_code, subclass, prog_if);
			
				pci_bus.pci_ch[pci_cnt].bus = bus;
				pci_bus.pci_ch[pci_cnt].dev = dev;
				pci_bus.pci_ch[pci_cnt].func = func;
			
				pci_bus.pci_ch[pci_cnt].vend_id = vend_id;
				pci_bus.pci_ch[pci_cnt].device_id = device_id;
				pci_bus.pci_ch[pci_cnt].class_code = class_code;
				pci_bus.pci_ch[pci_cnt].subclass = subclass;
				pci_bus.pci_ch[pci_cnt].prog_if = prog_if;
			
				pci_bus.pci_ch[pci_cnt].used = 1;
			
				pci_cnt++;
			
				if (func == 0) {
					unsigned int header_type = (read_pci_conf(bus, dev, func, 0x0C) >> 16) & 0xFF;
					if (!(header_type & 0x80)) {
						break;
					}
				}
			}
		}
	}
}

void check_ahci_contr(PCI_channel ahci){
	volatile unsigned int bar5 = read_pci_conf(ahci.bus, ahci.dev, ahci.func, 0x24);
	volatile unsigned int *mem_ahci = (volatile unsigned int *)bar5;
	
	unsigned int cap = mem_ahci[0];
	unsigned int ghc = mem_ahci[1];
	unsigned int pi = mem_ahci[3];
	unsigned int vs = mem_ahci[4];
	
	unsigned short major = (vs >> 16) & 0xFFFF;
	unsigned short minor = vs & 0xFFFF;
	
	push_char('\n');
	push_format("AHCI: CAP=0x%x, GHC=0x%x, PI=0x%x, VS=0x%x\n", cap, ghc, pi, vs);
	
	push_format("\nAHCI version %u.%u%u", (major >> 4), (major & 0xF), minor);
	push_char('\n');
	if (ghc & (1 << 31)) {
		push_text("AHCI enabled (AE bit setted)\n");
	}
	else {
		push_text("AHCI disabled (AE bit not setted)\nTrying to enable...");
		mem_ahci[1] = ghc | (1 << 31);
		
		for (int i = 0; i < 1000; i++) asm volatile("pause");
		
		if (mem_ahci[1] & (1 << 31)){
			push_text(" done!\n");
		}
		else {
			push_text(" failed to enable AHCI!\n");
		}
	}
}

void chk_ahci_ports(PCI_channel ahci){
	volatile unsigned int bar5 = read_pci_conf(ahci.bus, ahci.dev, ahci.func, 0x24);
	volatile unsigned int *mem_ahci = (volatile unsigned int *)bar5;

	unsigned int pi = mem_ahci[3];
	for (int port = 0; port < 32; port++){
		if (pi & (1 << port)){
			
			push_char('\n');
			
			volatile unsigned int * port_base = &mem_ahci[(0x100 + port * 0x80) / 4];
			unsigned int ssts = port_base[10];
			unsigned int det = ssts & 0x0F;
			
			push_format("Port %d: PxSSTS=0x%x, DET=%d (", port, port_base[10], port_base[10] & 0xF);
			if (det == 0x03){
				push_text("Dev present");
				unsigned int sig = port_base[9];
				push_format(" with signature: 0x%x )");
			}
			else {
				push_text("No dev)");
			}
			ini_ahci_ports(mem_ahci, port);
		}
	}
}

void ls_ahci_ports(PCI_channel ahci){
	volatile unsigned int bar5 = read_pci_conf(ahci.bus, ahci.dev, ahci.func, 0x24);
	volatile unsigned int *mem_ahci = (volatile unsigned int *)bar5;
	
	unsigned int pi = mem_ahci[3];
	for (int port = 0; port < 32; port++){
		
		if (pi & (1 << port)){
			if (port % 3 == 0){
				push_char('\n');
			}
			push_format("Port %d present; ", port);
		}
	}
}

void ini_ahci_ports(volatile unsigned int *mem_ahci, int port){
	return;
}

int rd_sector_ahci(volatile unsigned int *mem_ahci, int slot, int port, unsigned int lba, void *buffer){
    return 0;
}

int wr_sector_ahci(volatile unsigned int *mem_ahci, int slot, int port, unsigned int lba, void *buffer){
    return 0;
}

void ahci_formt(Ahci_dev ahci, int port){
	push_format("\nYo, this is erased place. Returning to OS...");
	return;
}

int ident_ahci(volatile unsigned int *mem_ahci, int slot, int port){
	return 0;
}

int get_ahci(const char *name){
	if (streq(name, "ahci0") == 0) return 0;
	if (streq(name, "ahci1") == 0) return 1;
	if (streq(name, "ahci2") == 0) return 2;
	if (streq(name, "ahci3") == 0) return 3;
	return -1;
}