#include <stdlib.h>
#include "ibm.h"
#include "device.h"
#include "io.h"
#include "mem.h"
#include "pci.h"

#include "sis496.h"

typedef struct sis496_t
{
        uint8_t pci_conf[256];
} sis496_t;

void sis496_recalcmapping(sis496_t *sis496)
{
        if (sis496->pci_conf[0x44] & 0x10)
        {
                if (sis496->pci_conf[0x45] & 0x01)
                        mem_bios_set_state(0xe0000, 0x08000, 1, 0);
                else
                        mem_bios_set_state(0xe0000, 0x08000, 1, 1);
        }
        else
                mem_bios_set_state(0xe0000, 0x08000, 0, 1);

        if (sis496->pci_conf[0x44] & 0x20)
        {
                if (sis496->pci_conf[0x45] & 0x01)
                        mem_bios_set_state(0xe8000, 0x08000, 1, 0);
                else
                        mem_bios_set_state(0xe8000, 0x08000, 1, 1);
        }
        else
                mem_bios_set_state(0xe8000, 0x08000, 0, 1);
                
        if (sis496->pci_conf[0x44] & 0x40)
        {
                if (sis496->pci_conf[0x45] & 0x01)
                        mem_bios_set_state(0xf0000, 0x08000, 1, 0);
                else
                        mem_bios_set_state(0xf0000, 0x08000, 1, 1);
        }
        else
                mem_bios_set_state(0xf0000, 0x08000, 0, 1);
                
        if (sis496->pci_conf[0x44] & 0x80)
        {
                if (sis496->pci_conf[0x45] & 0x01)
                        mem_bios_set_state(0xf8000, 0x08000, 1, 0);
                else
                        mem_bios_set_state(0xf8000, 0x08000, 1, 1);
        }
        else
                mem_bios_set_state(0xf8000, 0x08000, 0, 1);

        flushmmucache();
        shadowbios = (sis496->pci_conf[0x44] & 0xf0);
}

void sis496_write(int func, int addr, uint8_t val, void *p)
{
        sis496_t *sis496 = (sis496_t *)p;
//pclog("sis496_write : addr=%02x val=%02x\n", addr, val);
        switch (addr)
        {
                case 0x44: /*Shadow configure*/
                if ((sis496->pci_conf[0x44] & val) ^ 0xf0)
                {
                        sis496->pci_conf[0x44] = val;
                        sis496_recalcmapping(sis496);
                }
                break;
                case 0x45: /*Shadow configure*/
                //if (val == 3)
                //        output = 3;
                if ((sis496->pci_conf[0x45] & val) ^ 0x01)
                {
                        sis496->pci_conf[0x45] = val;
                        sis496_recalcmapping(sis496);
                }
                break;
        }
                
        if ((addr >= 4 && addr < 8) || addr >= 0x40)
           sis496->pci_conf[addr] = val;
}

uint8_t sis496_read(int func, int addr, void *p)
{
        sis496_t *sis496 = (sis496_t *)p;
        
        return sis496->pci_conf[addr];
}
 
void *sis496_init()
{
        sis496_t *sis496 = malloc(sizeof(sis496_t));
        memset(sis496, 0, sizeof(sis496_t));
        
        pci_add_specific(5, sis496_read, sis496_write, sis496);
        
        sis496->pci_conf[0x00] = 0x39; /*SiS*/
        sis496->pci_conf[0x01] = 0x10; 
        sis496->pci_conf[0x02] = 0x96; /*496/497*/
        sis496->pci_conf[0x03] = 0x04; 

        sis496->pci_conf[0x04] = 7;
        sis496->pci_conf[0x05] = 0;

        sis496->pci_conf[0x06] = 0x80;
        sis496->pci_conf[0x07] = 0x02;
        
        sis496->pci_conf[0x08] = 2; /*Device revision*/

        sis496->pci_conf[0x09] = 0x00; /*Device class (PCI bridge)*/
        sis496->pci_conf[0x0a] = 0x00;
        sis496->pci_conf[0x0b] = 0x06;
        
        sis496->pci_conf[0x0e] = 0x00; /*Single function device*/
}

void sis496_close(void *p)
{
        sis496_t *sis496 = (sis496_t *)p;

        free(sis496);
}

device_t sis496_device =
{
        "SiS 496/497",
        0,
        sis496_init,
        sis496_close,
        NULL,
        NULL,
        NULL,
        NULL
};
