typedef struct ati68860_ramdac_t
{
        uint8_t regs[16];
} ati68860_ramdac_t;

void ati68860_ramdac_out(uint16_t addr, uint8_t val, ati68860_ramdac_t *ramdac, svga_t *svga);
uint8_t ati68860_ramdac_in(uint16_t addr, ati68860_ramdac_t *ramdac, svga_t *svga);
