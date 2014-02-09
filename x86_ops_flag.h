static int opCMC(uint32_t fetchdat)
{
        flags_rebuild();
        flags ^= C_FLAG;
        cycles -= 2;
        return 0;
}


static int opCLC(uint32_t fetchdat)
{
        flags_rebuild();
        flags &= ~C_FLAG;
        cycles -= 2;
        return 0;
}
static int opCLD(uint32_t fetchdat)
{
        flags &= ~D_FLAG;
        cycles -= 2;
        return 0;
}
static int opCLI(uint32_t fetchdat)
{
        if (!IOPLp)
        {
                x86gpf(NULL,0);
        }
        else
                flags &= ~I_FLAG;
                        
        cycles -= 3;
        return 0;
}

static int opSTC(uint32_t fetchdat)
{
        flags_rebuild();
        flags |= C_FLAG;
        cycles -= 2;
        return 0;
}
static int opSTD(uint32_t fetchdat)
{
        flags |= D_FLAG;
        cycles -= 2;
        return 0;
}
static int opSTI(uint32_t fetchdat)
{
        if (!IOPLp)
        {
                x86gpf(NULL,0);
        }
        else
                flags |= I_FLAG;
                        
        cycles -= 2;
        return 0;
}

static int opSAHF(uint32_t fetchdat)
{
        flags_rebuild();
        flags = (flags & 0xff00) | (AH & 0xd5) | 2;
        cycles -= 3;
        return 0;
}
static int opLAHF(uint32_t fetchdat)
{
        flags_rebuild();
        AH = flags & 0xff;
        cycles -= 3;
        return 0;
}

static int opPUSHF(uint32_t fetchdat)
{
        if (ssegs) ss = oldss;
        if ((eflags & VM_FLAG) && (IOPL < 3))
        {
                x86gpf(NULL,0);
                return 0;
        }
        flags_rebuild();
        PUSH_W(flags);
        cycles -= 4;
        return 0;
}
static int opPUSHFD(uint32_t fetchdat)
{
        uint16_t tempw;
        if (ssegs) ss=oldss;
        if ((eflags & VM_FLAG) && (IOPL < 3))
        {
                x86gpf(NULL, 0);
                return 0;
        }
        if (CPUID) tempw = eflags & 0x24;
        else       tempw = eflags & 4;
        flags_rebuild();
        PUSH_L(flags | (tempw << 16));
        cycles -= 4;
        return 0;
}

static int opPOPF_286(uint32_t fetchdat)
{
        uint16_t tempw;
        if (ssegs) ss = oldss;
        
        if ((eflags & VM_FLAG) && (IOPL < 3))
        {
                x86gpf(NULL, 0);
                return 0;
        }
        
        tempw = POP_W();                if (abrt) return 0;

        if (!(msw & 1))           flags = (flags & 0x7000) | (tempw & 0x0fd5) | 2;
        else if (!(CPL))          flags = (tempw & 0x7fd5) | 2;
        else if (IOPLp)           flags = (flags & 0x3000) | (tempw & 0x4fd5) | 2;
        else                      flags = (flags & 0x3200) | (tempw & 0x4dd5) | 2;
        flags_extract();

        cycles -= 5;
        return 0;
}
static int opPOPF(uint32_t fetchdat)
{
        uint16_t tempw;
        if (ssegs) ss = oldss;
        
        if ((eflags & VM_FLAG) && (IOPL < 3))
        {
                x86gpf(NULL, 0);
                return 0;
        }
        
        tempw = POP_W();                if (abrt) return 0;
        
        if (!(CPL) || !(msw & 1)) flags = (tempw & 0xffd5) | 2;
        else if (IOPLp)           flags = (flags & 0x3000) | (tempw & 0xcfd5) | 2;
        else                      flags = (flags & 0x3200) | (tempw & 0xcdd5) | 2;
        flags_extract();

        cycles -= 5;
        return 0;
}
static int opPOPFD(uint32_t fetchdat)
{
        uint32_t templ;
        if (ssegs) ss = oldss;
        
        if ((eflags & VM_FLAG) && (IOPL < 3))
        {
                x86gpf(NULL, 0);
                return 0;                
        }
        
        templ = POP_L();                if (abrt) return 0;

        if (!(CPL) || !(msw & 1)) flags = (templ & 0xffd5) | 2;
        else if (IOPLp)           flags = (flags & 0x3000) | (templ & 0xcfd5) | 2;
        else                      flags = (flags & 0x3200) | (templ & 0xcdd5) | 2;
        
        templ &= is486 ? 0x240000 : 0;
        templ |= ((eflags&3) << 16);
        if (CPUID)      eflags = (templ >> 16) & 0x27;
        else if (is486) eflags = (templ >> 16) & 7;
        else            eflags = (templ >> 16) & 3;
        
        flags_extract();

        cycles -= 5;
        return 0;
}
