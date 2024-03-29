/*PRD format :
        
        word 0 - base address
        word 1 - bits 1 - 15 = byte count, bit 31 = end of transfer
*/
#include <string.h>

#include "ibm.h"
#include "ide.h"
#include "io.h"
#include "pci.h"

#include "piix.h"

uint8_t piix_bus_master_read(uint16_t port, void *priv);
void piix_bus_master_write(uint16_t port, uint8_t val, void *priv);

static uint8_t card_piix[256], card_piix_ide[256];

void piix_write(int func, int addr, uint8_t val, void *priv)
{
        if (func > 1)
           return;
        
        if (func == 1) /*IDE*/
        {
                switch (addr)
                {
                        case 0x00: case 0x01: case 0x02: case 0x03:
                        case 0x08: case 0x09: case 0x0a: case 0x0b:
                        case 0x0e:
                        return;
                        case 0x20:
                        val |= 1;
                        break;
                        case 0x41:
                        if ((val ^ card_piix_ide[0x41]) & 0x80)
                        {
                                ide_pri_disable();
                                if (val & 0x80)
                                   ide_pri_enable();
                        }
                        break;
                        case 0x43:
                        if ((val ^ card_piix_ide[0x43]) & 0x80)
                        {
                                ide_sec_disable();
                                if (val & 0x80)
                                   ide_sec_enable();                                  
                        }
                        break;
                }
                card_piix_ide[addr] = val;
                if ((addr & ~3) == 0x20) /*Bus master base address*/                
                {
                        uint16_t base = (card_piix_ide[0x20] & 0xf0) | (card_piix_ide[0x21] << 8);
                        io_removehandler(0, 0x10000, piix_bus_master_read, NULL, NULL, piix_bus_master_write, NULL, NULL,  NULL);
                        io_sethandler(base, 0x10, piix_bus_master_read, NULL, NULL, piix_bus_master_write, NULL, NULL,  NULL);
                }
//                pclog("PIIX write %02X %02X\n", addr, val);
        }
        else
        {
                switch (addr)
                {
                        case 0x00: case 0x01: case 0x02: case 0x03:
                        case 0x08: case 0x09: case 0x0a: case 0x0b:
                        case 0x0e:
                        return;
                }
                card_piix[addr] = val;
        }
}

uint8_t piix_read(int func, int addr, void *priv)
{
        if (func > 1)
           return 0xff;

        if (func == 1) /*IDE*/
        {
//                pclog("PIIX IDE read %02X %02X\n", addr, card_piix_ide[addr]);                
                return card_piix_ide[addr];
        }
        else
           return card_piix[addr];
}

struct
{
        uint8_t command;
        uint8_t status;
        uint32_t ptr, ptr_cur;
        int count;
        uint32_t addr;
        int eot;
} piix_busmaster[2];

static void piix_bus_master_next_addr(int channel)
{
        piix_busmaster[channel].addr = (*(unsigned long *)(&ram[piix_busmaster[channel].ptr_cur])) & ~1;
        piix_busmaster[channel].count = (*(unsigned long *)(&ram[piix_busmaster[channel].ptr_cur + 4])) & 0xfffe;
        piix_busmaster[channel].eot = (*(unsigned long *)(&ram[piix_busmaster[channel].ptr_cur + 4])) >> 31;
        piix_busmaster[channel].ptr_cur += 8;
//        pclog("New DMA settings on channel %i - Addr %08X Count %04X EOT %i\n", channel, piix_busmaster[channel].addr, piix_busmaster[channel].count, piix_busmaster[channel].eot);
}

void piix_bus_master_write(uint16_t port, uint8_t val, void *priv)
{
        int channel = (port & 8) ? 1 : 0;
//        pclog("PIIX Bus Master write %04X %02X\n", port, val);
        switch (port & 7)
        {
                case 0:
                if ((val & 1) && !(piix_busmaster[channel].command & 1)) /*Start*/
                {
                        piix_busmaster[channel].ptr_cur = piix_busmaster[channel].ptr;
                        piix_bus_master_next_addr(channel);
                        piix_busmaster[channel].status |= 1;
                }
                if (!(val & 1) && (piix_busmaster[channel].command & 1)) /*Stop*/
                   piix_busmaster[channel].status &= ~1;
                   
                piix_busmaster[channel].command = val;
                break;
                case 2:
                piix_busmaster[channel].status = (val & 0x60) | ((piix_busmaster[channel].status & ~val) & 6) | (piix_busmaster[channel].status & 1);
                break;
                case 4:
                piix_busmaster[channel].ptr = (piix_busmaster[channel].ptr & 0xffffff00) | val;
                break;
                case 5:
                piix_busmaster[channel].ptr = (piix_busmaster[channel].ptr & 0xffff00ff) | (val << 8);
                break;
                case 6:
                piix_busmaster[channel].ptr = (piix_busmaster[channel].ptr & 0xff00ffff) | (val << 16);
                break;
                case 7:
                piix_busmaster[channel].ptr = (piix_busmaster[channel].ptr & 0x00ffffff) | (val << 24);
                break;
        }
}
                
uint8_t piix_bus_master_read(uint16_t port, void *priv)
{
        int channel = (port & 8) ? 1 : 0;
        switch (port & 7)
        {
                case 0:
                return piix_busmaster[channel].command;
                case 2:
                return piix_busmaster[channel].status;
                case 4:
                return piix_busmaster[channel].ptr;
                case 5:
                return piix_busmaster[channel].ptr >> 8;
                case 6:
                return piix_busmaster[channel].ptr >> 16;
                case 7:
                return piix_busmaster[channel].ptr >> 24;
        }
        return 0xff;
}

int piix_bus_master_sector_read(int channel, uint8_t *data)
{
        int transferred = 0;
        
        if (!(piix_busmaster[channel].status & 1))
           return 1;                                    /*DMA disabled*/
           
        while (transferred < 512)
        {
                if (piix_busmaster[channel].count < (512 - transferred) && piix_busmaster[channel].eot)
                   fatal("DMA on channel %i - Read count less than 512! Addr %08X Count %04X EOT %i\n", channel, piix_busmaster[channel].addr, piix_busmaster[channel].count, piix_busmaster[channel].eot);
                
                if (piix_busmaster[channel].count < (512 - transferred))
                {
//                        pclog("Transferring smaller - %i bytes\n", piix_busmaster[channel].count);
                        memcpy(&ram[piix_busmaster[channel].addr], data + transferred, piix_busmaster[channel].count);
                        transferred += piix_busmaster[channel].count;
                        piix_busmaster[channel].addr += piix_busmaster[channel].count;
                        piix_busmaster[channel].count = 0;
                }                       
                else
                {
//                        pclog("Transferring larger - %i bytes\n", 512 - transferred);
                        memcpy(&ram[piix_busmaster[channel].addr], data + transferred, 512 - transferred);
                        piix_busmaster[channel].addr += (512 - transferred);
                        piix_busmaster[channel].count -= (512 - transferred);
                        transferred += (512 - transferred);                        
                }

//                pclog("DMA on channel %i - Addr %08X Count %04X EOT %i\n", channel, piix_busmaster[channel].addr, piix_busmaster[channel].count, piix_busmaster[channel].eot);

                if (!piix_busmaster[channel].count)
                {
//                        pclog("DMA on channel %i - block over\n", channel);
                        if (piix_busmaster[channel].eot) /*End of transfer?*/
                        {
//                                pclog("DMA on channel %i - transfer over\n", channel);
                                piix_busmaster[channel].status &= ~1;
                        }
                        else
                           piix_bus_master_next_addr(channel);
                }
        }
        return 0;
}
int piix_bus_master_sector_write(int channel, uint8_t *data)
{
        int transferred = 0;
        
        if (!(piix_busmaster[channel].status & 1))
           return 1;                                    /*DMA disabled*/

        while (transferred < 512)
        {
                if (piix_busmaster[channel].count < (512 - transferred) && piix_busmaster[channel].eot)
                   fatal("DMA on channel %i - Write count less than 512! Addr %08X Count %04X EOT %i\n", channel, piix_busmaster[channel].addr, piix_busmaster[channel].count, piix_busmaster[channel].eot);
                
                if (piix_busmaster[channel].count < (512 - transferred))
                {
//                        pclog("Transferring smaller - %i bytes\n", piix_busmaster[channel].count);
                        memcpy(data + transferred, &ram[piix_busmaster[channel].addr], piix_busmaster[channel].count);
                        transferred += piix_busmaster[channel].count;
                        piix_busmaster[channel].addr += piix_busmaster[channel].count;
                        piix_busmaster[channel].count = 0;
                }                       
                else
                {
//                        pclog("Transferring larger - %i bytes\n", 512 - transferred);
                        memcpy(data + transferred, &ram[piix_busmaster[channel].addr], 512 - transferred);
                        piix_busmaster[channel].addr += (512 - transferred);
                        piix_busmaster[channel].count -= (512 - transferred);
                        transferred += (512 - transferred);                        
                }

//                pclog("DMA on channel %i - Addr %08X Count %04X EOT %i\n", channel, piix_busmaster[channel].addr, piix_busmaster[channel].count, piix_busmaster[channel].eot);

                if (!piix_busmaster[channel].count)
                {
//                        pclog("DMA on channel %i - block over\n", channel);
                        if (piix_busmaster[channel].eot) /*End of transfer?*/
                        {
//                                pclog("DMA on channel %i - transfer over\n", channel);
                                piix_busmaster[channel].status &= ~1;
                        }
                        else
                           piix_bus_master_next_addr(channel);
                }
        }
        return 0;
}

void piix_bus_master_set_irq(int channel)
{
        piix_busmaster[channel].status |= 4;
}

void piix_init(int card)
{
        pci_add_specific(card, piix_read, piix_write, NULL);
        
        memset(card_piix, 0, 256);
        card_piix[0x00] = 0x86; card_piix[0x01] = 0x80; /*Intel*/
        card_piix[0x02] = 0x2e; card_piix[0x03] = 0x12; /*82371FB (PIIX)*/
        card_piix[0x04] = 0x07; card_piix[0x05] = 0x00;
        card_piix[0x06] = 0x00; card_piix[0x07] = 0x02;
        card_piix[0x08] = 0x00; /*A0 stepping*/
        card_piix[0x09] = 0x00; card_piix[0x0a] = 0x01; card_piix[0x0b] = 0x06;
        card_piix[0x0e] = 0x80; /*Multi-function device*/
        card_piix[0x4c] = 0x4d;
        card_piix[0x4e] = 0x03;
        card_piix[0x60] = card_piix[0x61] = card_piix[0x62] = card_piix[0x63] = 0x80;
        card_piix[0x69] = 0x02;
        card_piix[0x70] = card_piix[0x71] = 0x80;
        card_piix[0x76] = card_piix[0x77] = 0x0c;
        card_piix[0x78] = 0x02; card_piix[0x79] = 0x00;
        card_piix[0xa0] = 0x08;
        card_piix[0xa2] = card_piix[0xa3] = 0x00;
        card_piix[0xa4] = card_piix[0xa5] = card_piix[0xa6] = card_piix[0xa7] = 0x00;
        card_piix[0xa8] = 0x0f;
        card_piix[0xaa] = card_piix[0xab] = 0x00;
        card_piix[0xac] = 0x00;
        card_piix[0xae] = 0x00;

        card_piix_ide[0x00] = 0x86; card_piix_ide[0x01] = 0x80; /*Intel*/
        card_piix_ide[0x02] = 0x30; card_piix_ide[0x03] = 0x12; /*82371FB (PIIX)*/
        card_piix_ide[0x04] = 0x00; card_piix_ide[0x05] = 0x00;
        card_piix_ide[0x06] = 0x80; card_piix_ide[0x07] = 0x02;
        card_piix_ide[0x08] = 0x00;
        card_piix_ide[0x09] = 0x80; card_piix_ide[0x0a] = 0x01; card_piix_ide[0x0b] = 0x01;
        card_piix_ide[0x0d] = 0x00;
        card_piix_ide[0x0e] = 0x00;
        card_piix_ide[0x20] = 0x01; card_piix_ide[0x21] = card_piix_ide[0x22] = card_piix_ide[0x23] = 0x00; /*Bus master interface base address*/
        card_piix_ide[0x40] = card_piix_ide[0x41] = 0x00;
        card_piix_ide[0x42] = card_piix_ide[0x43] = 0x00;
        
        ide_set_bus_master(piix_bus_master_sector_read, piix_bus_master_sector_write, piix_bus_master_set_irq);
}
