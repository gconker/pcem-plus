#define INC_DEC_OP(name, reg, inc, setflags) \
        static int op ## name (uint32_t fetchdat)       \
        {                                               \
                setflags(reg, 1);                       \
                reg += inc;                             \
                cycles -= timing_rr;                    \
                return 0;                               \
        }

INC_DEC_OP(INC_AX, AX, 1, setadd16nc)
INC_DEC_OP(INC_BX, BX, 1, setadd16nc)
INC_DEC_OP(INC_CX, CX, 1, setadd16nc)
INC_DEC_OP(INC_DX, DX, 1, setadd16nc)
INC_DEC_OP(INC_SI, SI, 1, setadd16nc)
INC_DEC_OP(INC_DI, DI, 1, setadd16nc)
INC_DEC_OP(INC_BP, BP, 1, setadd16nc)
INC_DEC_OP(INC_SP, SP, 1, setadd16nc)

INC_DEC_OP(INC_EAX, EAX, 1, setadd32nc)
INC_DEC_OP(INC_EBX, EBX, 1, setadd32nc)
INC_DEC_OP(INC_ECX, ECX, 1, setadd32nc)
INC_DEC_OP(INC_EDX, EDX, 1, setadd32nc)
INC_DEC_OP(INC_ESI, ESI, 1, setadd32nc)
INC_DEC_OP(INC_EDI, EDI, 1, setadd32nc)
INC_DEC_OP(INC_EBP, EBP, 1, setadd32nc)
INC_DEC_OP(INC_ESP, ESP, 1, setadd32nc)

INC_DEC_OP(DEC_AX, AX, -1, setsub16nc)
INC_DEC_OP(DEC_BX, BX, -1, setsub16nc)
INC_DEC_OP(DEC_CX, CX, -1, setsub16nc)
INC_DEC_OP(DEC_DX, DX, -1, setsub16nc)
INC_DEC_OP(DEC_SI, SI, -1, setsub16nc)
INC_DEC_OP(DEC_DI, DI, -1, setsub16nc)
INC_DEC_OP(DEC_BP, BP, -1, setsub16nc)
INC_DEC_OP(DEC_SP, SP, -1, setsub16nc)

INC_DEC_OP(DEC_EAX, EAX, -1, setsub32nc)
INC_DEC_OP(DEC_EBX, EBX, -1, setsub32nc)
INC_DEC_OP(DEC_ECX, ECX, -1, setsub32nc)
INC_DEC_OP(DEC_EDX, EDX, -1, setsub32nc)
INC_DEC_OP(DEC_ESI, ESI, -1, setsub32nc)
INC_DEC_OP(DEC_EDI, EDI, -1, setsub32nc)
INC_DEC_OP(DEC_EBP, EBP, -1, setsub32nc)
INC_DEC_OP(DEC_ESP, ESP, -1, setsub32nc)


static int opINCDEC_b_a16(uint32_t fetchdat)
{
        uint8_t temp;
        
        fetch_ea_16(fetchdat);       
        temp=geteab();                  if (abrt) return 0;

        if (rmdat&0x38)
        {
                seteab(temp - 1);       if (abrt) return 0;
                setsub8nc(temp, 1);
        }
        else
        {
                seteab(temp + 1);       if (abrt) return 0;
                setadd8nc(temp, 1);
        }
        cycles -= (mod == 3) ? timing_rr : timing_mm;
        return 0;
}
static int opINCDEC_b_a32(uint32_t fetchdat)
{
        uint8_t temp;
        
        fetch_ea_32(fetchdat);       
        temp=geteab();                  if (abrt) return 0;

        if (rmdat&0x38)
        {
                seteab(temp - 1);       if (abrt) return 0;
                setsub8nc(temp, 1);
        }
        else
        {
                seteab(temp + 1);       if (abrt) return 0;
                setadd8nc(temp, 1);
        }
        cycles -= (mod == 3) ? timing_rr : timing_mm;
        return 0;
}
