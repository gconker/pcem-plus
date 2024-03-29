static int opMOVZX_w_b_a16(uint32_t fetchdat)
{
        uint8_t temp;
        
        fetch_ea_16(fetchdat);
        temp = geteab();                        if (abrt) return 0;
        regs[reg].w = (uint16_t)temp;
        
        cycles -= 3;
        return 0;
}
static int opMOVZX_w_b_a32(uint32_t fetchdat)
{
        uint8_t temp;
        
        fetch_ea_32(fetchdat);
        temp = geteab();                        if (abrt) return 0;
        regs[reg].w = (uint16_t)temp;
        
        cycles -= 3;
        return 0;
}
static int opMOVZX_l_b_a16(uint32_t fetchdat)
{
        uint8_t temp;
        
        fetch_ea_16(fetchdat);
        temp = geteab();                        if (abrt) return 0;
        regs[reg].l = (uint32_t)temp;
        
        cycles -= 3;
        return 0;
}
static int opMOVZX_l_b_a32(uint32_t fetchdat)
{
        uint8_t temp;
        
        fetch_ea_32(fetchdat);
        temp = geteab();                        if (abrt) return 0;
        regs[reg].l = (uint32_t)temp;
        
        cycles -= 3;
        return 0;
}
static int opMOVZX_w_w_a16(uint32_t fetchdat)
{
        uint16_t temp;
        
        fetch_ea_16(fetchdat);
        temp = geteaw();                        if (abrt) return 0;
        regs[reg].w = temp;
        
        cycles -= 3;
        return 0;
}
static int opMOVZX_w_w_a32(uint32_t fetchdat)
{
        uint16_t temp;
        
        fetch_ea_32(fetchdat);
        temp = geteaw();                        if (abrt) return 0;
        regs[reg].w = temp;
        
        cycles -= 3;
        return 0;
}
static int opMOVZX_l_w_a16(uint32_t fetchdat)
{
        uint16_t temp;
        
        fetch_ea_16(fetchdat);
        temp = geteaw();                        if (abrt) return 0;
        regs[reg].l = (uint32_t)temp;
        
        cycles -= 3;
        return 0;
}
static int opMOVZX_l_w_a32(uint32_t fetchdat)
{
        uint16_t temp;
        
        fetch_ea_32(fetchdat);
        temp = geteaw();                        if (abrt) return 0;
        regs[reg].l = (uint32_t)temp;
        
        cycles -= 3;
        return 0;
}

static int opMOVSX_w_b_a16(uint32_t fetchdat)
{
        uint8_t temp;
        
        fetch_ea_16(fetchdat);
        temp = geteab();                        if (abrt) return 0;
        regs[reg].w = (uint16_t)temp;
        if (temp & 0x80)        
                regs[reg].w |= 0xff00;
        
        cycles -= 3;
        return 0;
}
static int opMOVSX_w_b_a32(uint32_t fetchdat)
{
        uint8_t temp;
        
        fetch_ea_32(fetchdat);
        temp = geteab();                        if (abrt) return 0;
        regs[reg].w = (uint16_t)temp;
        if (temp & 0x80)        
                regs[reg].w |= 0xff00;
        
        cycles -= 3;
        return 0;
}
static int opMOVSX_l_b_a16(uint32_t fetchdat)
{
        uint8_t temp;
        
        fetch_ea_16(fetchdat);
        temp = geteab();                        if (abrt) return 0;
        regs[reg].l = (uint32_t)temp;
        if (temp & 0x80)        
                regs[reg].l |= 0xffffff00;
        
        cycles -= 3;
        return 0;
}
static int opMOVSX_l_b_a32(uint32_t fetchdat)
{
        uint8_t temp;
        
        fetch_ea_32(fetchdat);
        temp = geteab();                        if (abrt) return 0;
        regs[reg].l = (uint32_t)temp;
        if (temp & 0x80)        
                regs[reg].l |= 0xffffff00;
        
        cycles -= 3;
        return 0;
}
static int opMOVSX_l_w_a16(uint32_t fetchdat)
{
        uint16_t temp;
        
        fetch_ea_16(fetchdat);
        temp = geteaw();                        if (abrt) return 0;
        regs[reg].l = (uint32_t)temp;
        if (temp & 0x8000)
                regs[reg].l |= 0xffff0000;
        
        cycles -= 3;
        return 0;
}
static int opMOVSX_l_w_a32(uint32_t fetchdat)
{
        uint16_t temp;
        
        fetch_ea_32(fetchdat);
        temp = geteaw();                        if (abrt) return 0;
        regs[reg].l = (uint32_t)temp;
        if (temp & 0x8000)
                regs[reg].l |= 0xffff0000;
        
        cycles -= 3;
        return 0;
}
