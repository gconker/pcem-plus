#define cond_O   ( VF_SET())
#define cond_NO  (!VF_SET())
#define cond_B   ( CF_SET())
#define cond_NB  (!CF_SET())
#define cond_E   ( ZF_SET())
#define cond_NE  (!ZF_SET())
#define cond_BE  ( CF_SET() ||  ZF_SET())
#define cond_NBE (!CF_SET() && !ZF_SET())
#define cond_S   ( NF_SET())
#define cond_NS  (!NF_SET())
#define cond_P   ( PF_SET())
#define cond_NP  (!PF_SET())
#define cond_L   (((NF_SET()) ? 1 : 0) != ((VF_SET()) ? 1 : 0))
#define cond_NL  (((NF_SET()) ? 1 : 0) == ((VF_SET()) ? 1 : 0))
#define cond_LE  (((NF_SET()) ? 1 : 0) != ((VF_SET()) ? 1 : 0) ||  (ZF_SET()))
#define cond_NLE (((NF_SET()) ? 1 : 0) == ((VF_SET()) ? 1 : 0) && (!ZF_SET()))

#define opJ(condition)                                  \
        static int opJ ## condition(uint32_t fetchdat)  \
        {                                               \
                int8_t offset = (int8_t)getbytef();     \
                if (cond_ ## condition)                 \
                {                                       \
                        pc += offset;                   \
                        cycles -= timing_bt;            \
                }                                       \
                cycles -= timing_bnt;                   \
                return 0;                               \
        }                                               \
                                                        \
        static int opJ ## condition ## _w(uint32_t fetchdat)  \
        {                                               \
                int16_t offset = (int16_t)getwordf();   \
                if (cond_ ## condition)                 \
                {                                       \
                        pc += offset;                   \
                        cycles -= timing_bt;            \
                }                                       \
                cycles -= timing_bnt;                   \
                return 0;                               \
        }                                               \
                                                        \
        static int opJ ## condition ## _l(uint32_t fetchdat)  \
        {                                               \
                uint32_t offset = getlong();            \
                if (cond_ ## condition)                 \
                {                                       \
                        pc += offset;                   \
                        cycles -= timing_bt;            \
                }                                       \
                cycles -= timing_bnt;                   \
                return 0;                               \
        }                                               \
        
opJ(O)
opJ(NO)
opJ(B)
opJ(NB)
opJ(E)
opJ(NE)
opJ(BE)
opJ(NBE)
opJ(S)
opJ(NS)
opJ(P)
opJ(NP)
opJ(L)
opJ(NL)
opJ(LE)
opJ(NLE)



static int opLOOPNE_w(uint32_t fetchdat)
{
        int8_t offset = (int8_t)getbytef();
        CX--;
        if (CX && !ZF_SET()) 
                pc += offset;
        cycles -= (is486) ? 7 : 11;
        return 0;
}
static int opLOOPNE_l(uint32_t fetchdat)
{
        int8_t offset = (int8_t)getbytef();
        ECX--;
        if (ECX && !ZF_SET()) 
                pc += offset;
        cycles -= (is486) ? 7 : 11;
        return 0;
}

static int opLOOPE_w(uint32_t fetchdat)
{
        int8_t offset = (int8_t)getbytef();
        CX--;
        if (CX && ZF_SET()) 
                pc += offset;
        cycles -= (is486) ? 7 : 11;
        return 0;
}
static int opLOOPE_l(uint32_t fetchdat)
{
        int8_t offset = (int8_t)getbytef();
        ECX--;
        if (ECX && ZF_SET()) 
                pc += offset;
        cycles -= (is486) ? 7 : 11;
        return 0;
}

static int opLOOP_w(uint32_t fetchdat)
{
        int8_t offset = (int8_t)getbytef();
        CX--;
        if (CX)
                pc += offset;
        cycles -= (is486) ? 7 : 11;
        return 0;
}
static int opLOOP_l(uint32_t fetchdat)
{
        int8_t offset = (int8_t)getbytef();
        ECX--;
        if (ECX)
                pc += offset;
        cycles -= (is486) ? 7 : 11;
        return 0;
}

static int opJCXZ(uint32_t fetchdat)
{
        int8_t offset = (int8_t)getbytef();
        if (!CX)
        {
                pc += offset;
                cycles -= 4;
        }
        cycles -= 5;
        return 0;
}
static int opJECXZ(uint32_t fetchdat)
{
        int8_t offset = (int8_t)getbytef();
        if (!ECX)
        {
                pc += offset;
                cycles -= 4;
        }
        cycles -= 5;
        return 0;
}


static int opJMP_r8(uint32_t fetchdat)
{
        int8_t offset = (int8_t)getbytef();
        pc += offset;
        cycles -= (is486) ? 3 : 7;
        return 0;
}
static int opJMP_r16(uint32_t fetchdat)
{
        int16_t offset = (int16_t)getwordf();
        pc += offset;
        cycles -= (is486) ? 3 : 7;
        return 0;
}
static int opJMP_r32(uint32_t fetchdat)
{
        int32_t offset = (int32_t)getlong();            if (abrt) return 0;
        pc += offset;
        cycles -= (is486) ? 3 : 7;
        return 0;
}

static int opJMP_far_a16(uint32_t fetchdat)
{
        uint16_t addr = getwordf();
        uint16_t seg = getword();                       if (abrt) return 0;
        uint32_t oxpc = pc;
        pc = addr;
        loadcsjmp(seg, oxpc);
        cycles -= (is486) ? 17 : 12;
        return 0;
}
static int opJMP_far_a32(uint32_t fetchdat)
{
        uint32_t addr = getlong();
        uint16_t seg = getword();                       if (abrt) return 0;
        uint32_t oxpc = pc;
        pc = addr;
        loadcsjmp(seg, oxpc);
        cycles -= (is486) ? 17 : 12;
        return 0;
}

int opCALL_r16(uint32_t fetchdat)
{
        int16_t addr = (int16_t)getwordf();
        if (ssegs) ss=oldss;
        PUSH_W(pc);
        pc += addr;
        cycles -= (is486) ? 3 : 7;
        return 0;
}
int opCALL_r32(uint32_t fetchdat)
{
        int32_t addr = getlong();                       if (abrt) return 0;       
        if (ssegs) ss=oldss;
        PUSH_L(pc);
        pc += addr;
        cycles -= (is486) ? 3 : 7;
        return 0;
}

int opRET_w(uint32_t fetchdat)
{
        uint16_t ret;
        
        if (ssegs) ss=oldss;        
        ret = POP_W();                          if (abrt) return 0;
        pc = ret;
        
        cycles -= (is486) ? 5 : 10;
        return 0;
}
int opRET_l(uint32_t fetchdat)
{
        uint32_t ret;

        if (ssegs) ss=oldss;        
        ret = POP_L();                          if (abrt) return 0;
        pc = ret;
        
        cycles -= (is486) ? 5 : 10;
        return 0;
}

int opRET_w_imm(uint32_t fetchdat)
{
        uint16_t offset = getwordf();
        uint16_t ret;

        if (ssegs) ss=oldss;       
        ret = POP_W();                          if (abrt) return 0;
        if (stack32) ESP += offset;
        else          SP += offset;       
        pc = ret;
        
        cycles -= (is486) ? 5 : 10;
        return 0;
}
int opRET_l_imm(uint32_t fetchdat)
{
        uint16_t offset = getwordf();
        uint32_t ret;

        if (ssegs) ss=oldss;        
        ret = POP_L();                          if (abrt) return 0;
        if (stack32) ESP += offset;
        else          SP += offset;       
        pc = ret;
        
        cycles -= (is486) ? 5 : 10;
        return 0;
}

