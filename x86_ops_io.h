int opIN_AL_imm(uint32_t fetchdat)
{       
        uint16_t port = (uint16_t)getbytef();
        check_io_perm(port);
        AL = inb(port);
        cycles -= 12;
        return 0;
}
int opIN_AX_imm(uint32_t fetchdat)
{
        uint16_t port = (uint16_t)getbytef();
        check_io_perm(port);
        check_io_perm(port + 1);
        AX = inw(port);
        cycles -= 12;
        return 0;
}
int opIN_EAX_imm(uint32_t fetchdat)
{
        uint16_t port = (uint16_t)getbytef();
        check_io_perm(port);
        check_io_perm(port + 1);
        check_io_perm(port + 2);
        check_io_perm(port + 3);
        EAX = inl(port);
        cycles -= 12;
        return 0;
}

int opOUT_AL_imm(uint32_t fetchdat)
{
        uint16_t port = (uint16_t)getbytef();        
        check_io_perm(port);
        outb(port, AL);
        cycles -= 10;
        return 0;
}
int opOUT_AX_imm(uint32_t fetchdat)
{
        uint16_t port = (uint16_t)getbytef();        
        check_io_perm(port);
        check_io_perm(port + 1);
        outw(port, AX);
        cycles -= 10;
        return 0;
}
int opOUT_EAX_imm(uint32_t fetchdat)
{
        uint16_t port = (uint16_t)getbytef();        
        check_io_perm(port);
        check_io_perm(port + 1);
        check_io_perm(port + 2);
        check_io_perm(port + 3);
        outl(port, EAX);
        cycles -= 10;
        return 0;
}

int opIN_AL_DX(uint32_t fetchdat)
{       
        check_io_perm(DX);
        AL = inb(DX);
        cycles -= 12;
        return 0;
}
int opIN_AX_DX(uint32_t fetchdat)
{
        check_io_perm(DX);
        check_io_perm(DX + 1);
        AX = inw(DX);
        cycles -= 12;
        return 0;
}
int opIN_EAX_DX(uint32_t fetchdat)
{
        check_io_perm(DX);
        check_io_perm(DX + 1);
        check_io_perm(DX + 2);
        check_io_perm(DX + 3);
        EAX = inl(DX);
        cycles -= 12;
        return 0;
}

int opOUT_AL_DX(uint32_t fetchdat)
{
        check_io_perm(DX);
        outb(DX, AL);
        cycles -= 11;
        return 0;
}
int opOUT_AX_DX(uint32_t fetchdat)
{
        //pclog("OUT_AX_DX %04X %04X\n", DX, AX);
        check_io_perm(DX);
        check_io_perm(DX + 1);
        outw(DX, AX);
        cycles -= 11;
        return 0;
}
int opOUT_EAX_DX(uint32_t fetchdat)
{
        check_io_perm(DX);
        check_io_perm(DX + 1);
        check_io_perm(DX + 2);
        check_io_perm(DX + 3);
        outl(DX, EAX);
        cycles -= 11;
        return 0;
}
