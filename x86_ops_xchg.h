static int opXCHG_b_a16(uint32_t fetchdat)
{
        uint8_t temp;
        fetch_ea_16(fetchdat);
        temp = geteab();                        if (abrt) return 0;
        seteab(getr8(reg));                     if (abrt) return 0;
        setr8(reg, temp);
        cycles -= ((mod == 3) ? 3 : 5);
        return 0;
}
static int opXCHG_b_a32(uint32_t fetchdat)
{
        uint8_t temp;
        fetch_ea_32(fetchdat);
        temp = geteab();                        if (abrt) return 0;
        seteab(getr8(reg));                     if (abrt) return 0;
        setr8(reg, temp);
        cycles -= ((mod == 3) ? 3 : 5);
        return 0;
}

static int opXCHG_w_a16(uint32_t fetchdat)
{
        uint16_t temp;
        fetch_ea_16(fetchdat);
        temp = geteaw();                        if (abrt) return 0;
        seteaw(regs[reg].w);                    if (abrt) return 0;
        regs[reg].w = temp;
        cycles -= ((mod == 3) ? 3 : 5);
        return 0;
}
static int opXCHG_w_a32(uint32_t fetchdat)
{
        uint16_t temp;
        fetch_ea_32(fetchdat);
        temp = geteaw();                        if (abrt) return 0;
        seteaw(regs[reg].w);                    if (abrt) return 0;
        regs[reg].w = temp;
        cycles -= ((mod == 3) ? 3 : 5);
        return 0;
}

static int opXCHG_l_a16(uint32_t fetchdat)
{
        uint32_t temp;
        fetch_ea_16(fetchdat);
        temp = geteal();                        if (abrt) return 0;
        seteal(regs[reg].l);                    if (abrt) return 0;
        regs[reg].l = temp;
        cycles -= ((mod == 3) ? 3 : 5);
        return 0;
}
static int opXCHG_l_a32(uint32_t fetchdat)
{
        uint32_t temp;
        fetch_ea_32(fetchdat);
        temp = geteal();                        if (abrt) return 0;
        seteal(regs[reg].l);                    if (abrt) return 0;
        regs[reg].l = temp;
        cycles -= ((mod == 3) ? 3 : 5);
        return 0;
}


static int opXCHG_AX_BX(uint32_t fetchdat)
{
        uint16_t temp = AX;
        AX = BX;
        BX = temp;
        cycles -= 3;
        return 0;
}
static int opXCHG_AX_CX(uint32_t fetchdat)
{
        uint16_t temp = AX;
        AX = CX;
        CX = temp;
        cycles -= 3;
        return 0;
}
static int opXCHG_AX_DX(uint32_t fetchdat)
{
        uint16_t temp = AX;
        AX = DX;
        DX = temp;
        cycles -= 3;
        return 0;
}
static int opXCHG_AX_SI(uint32_t fetchdat)
{
        uint16_t temp = AX;
        AX = SI;
        SI = temp;
        cycles -= 3;
        return 0;
}
static int opXCHG_AX_DI(uint32_t fetchdat)
{
        uint16_t temp = AX;
        AX = DI;
        DI = temp;
        cycles -= 3;
        return 0;
}
static int opXCHG_AX_BP(uint32_t fetchdat)
{
        uint16_t temp = AX;
        AX = BP;
        BP = temp;
        cycles -= 3;
        return 0;
}
static int opXCHG_AX_SP(uint32_t fetchdat)
{
        uint16_t temp = AX;
        AX = SP;
        SP = temp;
        cycles -= 3;
        return 0;
}

static int opXCHG_EAX_EBX(uint32_t fetchdat)
{
        uint32_t temp = EAX;
        EAX = EBX;
        EBX = temp;
        cycles -= 3;
        return 0;
}
static int opXCHG_EAX_ECX(uint32_t fetchdat)
{
        uint32_t temp = EAX;
        EAX = ECX;
        ECX = temp;
        cycles -= 3;
        return 0;
}
static int opXCHG_EAX_EDX(uint32_t fetchdat)
{
        uint32_t temp = EAX;
        EAX = EDX;
        EDX = temp;
        cycles -= 3;
        return 0;
}
static int opXCHG_EAX_ESI(uint32_t fetchdat)
{
        uint32_t temp = EAX;
        EAX = ESI;
        ESI = temp;
        cycles -= 3;
        return 0;
}
static int opXCHG_EAX_EDI(uint32_t fetchdat)
{
        uint32_t temp = EAX;
        EAX = EDI;
        EDI = temp;
        cycles -= 3;
        return 0;
}
static int opXCHG_EAX_EBP(uint32_t fetchdat)
{
        uint32_t temp = EAX;
        EAX = EBP;
        EBP = temp;
        cycles -= 3;
        return 0;
}
static int opXCHG_EAX_ESP(uint32_t fetchdat)
{
        uint32_t temp = EAX;
        EAX = ESP;
        ESP = temp;
        cycles -= 3;
        return 0;
}


#define opBSWAP(reg)                                                            \
        static int opBSWAP_ ## reg(uint32_t fetchdat)                           \
        {                                                                       \
                reg = (reg >> 24) | ((reg >> 8) & 0xff00) | ((reg << 8) & 0xff0000) | ((reg << 24) & 0xff000000);       \
                cycles--;                                                       \
                return 0;                                                       \
        }

opBSWAP(EAX)
opBSWAP(EBX)
opBSWAP(ECX)
opBSWAP(EDX)
opBSWAP(ESI)
opBSWAP(EDI)
opBSWAP(EBP)
opBSWAP(ESP)
