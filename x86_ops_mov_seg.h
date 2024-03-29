static int opMOV_w_seg_a16(uint32_t fetchdat)
{
        fetch_ea_16(fetchdat);
        
        switch (rmdat & 0x38)
        {
                case 0x00: /*ES*/
                seteaw(ES);
                break;
                case 0x08: /*CS*/
                seteaw(CS);
                break;
                case 0x18: /*DS*/
                if (ssegs) ds = oldds;
                seteaw(DS);
                break;
                case 0x10: /*SS*/
                if (ssegs) ss = oldss;
                seteaw(SS);
                break;
                case 0x20: /*FS*/
                seteaw(FS);
                break;
                case 0x28: /*GS*/
                seteaw(GS);
                break;
        }
                        
        cycles -= ((mod == 3) ? 2 : 3);
        return 0;
}
static int opMOV_w_seg_a32(uint32_t fetchdat)
{
        fetch_ea_32(fetchdat);
        
        switch (rmdat & 0x38)
        {
                case 0x00: /*ES*/
                seteaw(ES);
                break;
                case 0x08: /*CS*/
                seteaw(CS);
                break;
                case 0x18: /*DS*/
                if (ssegs) ds = oldds;
                seteaw(DS);
                break;
                case 0x10: /*SS*/
                if (ssegs) ss = oldss;
                seteaw(SS);
                break;
                case 0x20: /*FS*/
                seteaw(FS);
                break;
                case 0x28: /*GS*/
                seteaw(GS);
                break;
        }
                        
        cycles -= ((mod == 3) ? 2 : 3);
        return 0;
}

static int opMOV_l_seg_a16(uint32_t fetchdat)
{
        fetch_ea_16(fetchdat);
        
        switch (rmdat & 0x38)
        {
                case 0x00: /*ES*/
                if (mod == 3) regs[rm].l = ES;
                else          seteaw(ES);
                break;
                case 0x08: /*CS*/
                if (mod == 3) regs[rm].l = CS;
                else          seteaw(CS);
                break;
                case 0x18: /*DS*/
                if (ssegs) ds = oldds;
                if (mod == 3) regs[rm].l = DS;
                else          seteaw(DS);
                break;
                case 0x10: /*SS*/
                if (ssegs) ss = oldss;
                if (mod == 3) regs[rm].l = SS;
                else          seteaw(SS);
                break;
                case 0x20: /*FS*/
                if (mod == 3) regs[rm].l = FS;
                else          seteaw(FS);
                break;
                case 0x28: /*GS*/
                if (mod == 3) regs[rm].l = GS;
                else          seteaw(GS);
                break;
        }
        
        cycles -= ((mod == 3) ? 2 : 3);
        return 0;
}
static int opMOV_l_seg_a32(uint32_t fetchdat)
{
        fetch_ea_32(fetchdat);
        
        switch (rmdat & 0x38)
        {
                case 0x00: /*ES*/
                if (mod == 3) regs[rm].l = ES;
                else          seteaw(ES);
                break;
                case 0x08: /*CS*/
                if (mod == 3) regs[rm].l = CS;
                else          seteaw(CS);
                break;
                case 0x18: /*DS*/
                if (ssegs) ds = oldds;
                if (mod == 3) regs[rm].l = DS;
                else          seteaw(DS);
                break;
                case 0x10: /*SS*/
                if (ssegs) ss = oldss;
                if (mod == 3) regs[rm].l = SS;
                else          seteaw(SS);
                break;
                case 0x20: /*FS*/
                if (mod == 3) regs[rm].l = FS;
                else          seteaw(FS);
                break;
                case 0x28: /*GS*/
                if (mod == 3) regs[rm].l = GS;
                else          seteaw(GS);
                break;
        }
        
        cycles -= ((mod == 3) ? 2 : 3);
        return 0;
}

static int opMOV_seg_w_a16(uint32_t fetchdat)
{
        uint16_t new_seg;
        
        fetch_ea_16(fetchdat);
        
        new_seg=geteaw();         if (abrt) return 0;
        
        switch (rmdat & 0x38)
        {
                case 0x00: /*ES*/
                loadseg(new_seg, &_es);
                break;
                case 0x18: /*DS*/
                loadseg(new_seg, &_ds);
                if (ssegs) oldds = ds;
                break;
                case 0x10: /*SS*/
                loadseg(new_seg, &_ss);
                if (ssegs)
                {
                        ds = oldds;
                        rds = DS;
                        ssegs = 0;
                }
                op32 = use32;
                return 1;
                case 0x20: /*FS*/
                loadseg(new_seg, &_fs);
                break;
                case 0x28: /*GS*/
                loadseg(new_seg, &_gs);
                break;
        }
                        
        cycles -= ((mod == 3) ? 2 : 5);
        return 0;
}
static int opMOV_seg_w_a32(uint32_t fetchdat)
{
        uint16_t new_seg;
        
        fetch_ea_32(fetchdat);
        
        new_seg=geteaw();         if (abrt) return 0;
        
        switch (rmdat & 0x38)
        {
                case 0x00: /*ES*/
                loadseg(new_seg, &_es);
                break;
                case 0x18: /*DS*/
                loadseg(new_seg, &_ds);
                if (ssegs) oldds = ds;
                break;
                case 0x10: /*SS*/
                loadseg(new_seg, &_ss);
                if (ssegs)
                {
                        ds = oldds;
                        rds = DS;
                        ssegs = 0;
                }
                op32 = use32;
                return 1;
                case 0x20: /*FS*/
                loadseg(new_seg, &_fs);
                break;
                case 0x28: /*GS*/
                loadseg(new_seg, &_gs);
                break;
        }
                        
        cycles -= ((mod == 3) ? 2 : 5);
        return 0;
}


static int opLDS_w_a16(uint32_t fetchdat)
{
        uint16_t addr, seg;

        fetch_ea_16(fetchdat);
        ILLEGAL_ON(mod == 3);
        addr = readmemw(easeg, eaaddr);
        seg = readmemw(easeg, eaaddr + 2);      if (abrt) return 0;
        loadseg(seg, &_ds);                     if (abrt) return 0;
        regs[reg].w = addr;
        if (ssegs) oldds = ds;
 
        cycles -= 7;       
        return 0;
}
static int opLDS_w_a32(uint32_t fetchdat)
{
        uint16_t addr, seg;

        fetch_ea_32(fetchdat);
        ILLEGAL_ON(mod == 3);
        addr = readmemw(easeg, eaaddr);
        seg = readmemw(easeg, eaaddr + 2);      if (abrt) return 0;
        loadseg(seg, &_ds);                     if (abrt) return 0;
        regs[reg].w = addr;
        if (ssegs) oldds = ds;
 
        cycles -= 7;       
        return 0;
}
static int opLDS_l_a16(uint32_t fetchdat)
{
        uint32_t addr;
        uint16_t seg;

        fetch_ea_16(fetchdat);
        ILLEGAL_ON(mod == 3);
        addr = readmeml(easeg, eaaddr);
        seg = readmemw(easeg, eaaddr + 4);      if (abrt) return 0;
        loadseg(seg, &_ds);                     if (abrt) return 0;
        regs[reg].l = addr;
        if (ssegs) oldds = ds;
 
        cycles -= 7;       
        return 0;
}
static int opLDS_l_a32(uint32_t fetchdat)
{
        uint32_t addr;
        uint16_t seg;

        fetch_ea_32(fetchdat);
        ILLEGAL_ON(mod == 3);
        addr = readmeml(easeg, eaaddr);
        seg = readmemw(easeg, eaaddr + 4);      if (abrt) return 0;
        loadseg(seg, &_ds);                     if (abrt) return 0;
        regs[reg].l = addr;
        if (ssegs) oldds = ds;
 
        cycles -= 7;       
        return 0;
}

static int opLSS_w_a16(uint32_t fetchdat)
{
        uint16_t addr, seg;

        fetch_ea_16(fetchdat);
        ILLEGAL_ON(mod == 3);
        addr = readmemw(easeg, eaaddr);
        seg = readmemw(easeg, eaaddr + 2);      if (abrt) return 0;
        loadseg(seg, &_ss);                     if (abrt) return 0;
        regs[reg].w = addr;
        oldss = ss;
 
        cycles -= 7;       
        return 0;
}
static int opLSS_w_a32(uint32_t fetchdat)
{
        uint16_t addr, seg;

        fetch_ea_32(fetchdat);
        ILLEGAL_ON(mod == 3);
        addr = readmemw(easeg, eaaddr);
        seg = readmemw(easeg, eaaddr + 2);      if (abrt) return 0;
        loadseg(seg, &_ss);                     if (abrt) return 0;
        regs[reg].w = addr;
        oldss = ss;
 
        cycles -= 7;       
        return 0;
}
static int opLSS_l_a16(uint32_t fetchdat)
{
        uint32_t addr;
        uint16_t seg;

        fetch_ea_16(fetchdat);
        ILLEGAL_ON(mod == 3);
        addr = readmeml(easeg, eaaddr);
        seg = readmemw(easeg, eaaddr + 4);      if (abrt) return 0;
        loadseg(seg, &_ss);                     if (abrt) return 0;
        regs[reg].l = addr;
        oldss = ss;
 
        cycles -= 7;       
        return 0;
}
static int opLSS_l_a32(uint32_t fetchdat)
{
        uint32_t addr;
        uint16_t seg;

        fetch_ea_32(fetchdat);
        ILLEGAL_ON(mod == 3);
        addr = readmeml(easeg, eaaddr);
        seg = readmemw(easeg, eaaddr + 4);      if (abrt) return 0;
        loadseg(seg, &_ss);                     if (abrt) return 0;
        regs[reg].l = addr;
        oldss = ss;
 
        cycles -= 7;       
        return 0;
}

#define opLsel(name, sel)                                                       \
        static int opL ## name ## _w_a16(uint32_t fetchdat)                     \
        {                                                                       \
                uint16_t addr, seg;                                             \
                                                                                \
                fetch_ea_16(fetchdat);                                          \
                ILLEGAL_ON(mod == 3);                                           \
                addr = readmemw(easeg, eaaddr);                                 \
                seg = readmemw(easeg, eaaddr + 2);      if (abrt) return 0;     \
                loadseg(seg, &sel);                     if (abrt) return 0;     \
                regs[reg].w = addr;                                             \
                                                                                \
                cycles -= 7;                                                    \
                return 0;                                                       \
        }                                                                       \
                                                                                \
        static int opL ## name ## _w_a32(uint32_t fetchdat)                     \
        {                                                                       \
                uint16_t addr, seg;                                             \
                                                                                \
                fetch_ea_32(fetchdat);                                          \
                ILLEGAL_ON(mod == 3);                                           \
                addr = readmemw(easeg, eaaddr);                                 \
                seg = readmemw(easeg, eaaddr + 2);      if (abrt) return 0;     \
                loadseg(seg, &sel);                     if (abrt) return 0;     \
                regs[reg].w = addr;                                             \
                                                                                \
                cycles -= 7;                                                    \
                return 0;                                                       \
        }                                                                       \
                                                                                \
        static int opL ## name ## _l_a16(uint32_t fetchdat)                     \
        {                                                                       \
                uint32_t addr;                                                  \
                uint16_t seg;                                                   \
                                                                                \
                fetch_ea_16(fetchdat);                                          \
                ILLEGAL_ON(mod == 3);                                           \
                addr = readmeml(easeg, eaaddr);                                 \
                seg = readmemw(easeg, eaaddr + 4);      if (abrt) return 0;     \
                loadseg(seg, &sel);                     if (abrt) return 0;     \
                regs[reg].l = addr;                                             \
                                                                                \
                cycles -= 7;                                                    \
                return 0;                                                       \
        }                                                                       \
                                                                                \
        static int opL ## name ## _l_a32(uint32_t fetchdat)                     \
        {                                                                       \
                uint32_t addr;                                                  \
                uint16_t seg;                                                   \
                                                                                \
                fetch_ea_32(fetchdat);                                          \
                ILLEGAL_ON(mod == 3);                                           \
                addr = readmeml(easeg, eaaddr);                                 \
                seg = readmemw(easeg, eaaddr + 4);      if (abrt) return 0;     \
                loadseg(seg, &sel);                     if (abrt) return 0;     \
                regs[reg].l = addr;                                             \
                                                                                \
                cycles -= 7;                                                    \
                return 0;                                                       \
        }
        
opLsel(ES, _es)
opLsel(FS, _fs)
opLsel(GS, _gs)
