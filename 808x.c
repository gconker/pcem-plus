//1B64 - Vid_SetMode (Vid_Vesa.c)
//6689c - CONS_Printf
/*SHR AX,1

        4 clocks - fetch opcode
        4 clocks - fetch mod/rm
        2 clocks - execute              2 clocks - fetch opcode 1
                                        2 clocks - fetch opcode 2
                                        4 clocks - fetch mod/rm
        2 clocks - fetch opcode 1       2 clocks - execute
        2 clocks - fetch opcode 2  etc*/
#include <stdio.h>
#include "ibm.h"

#include "cpu.h"
#include "keyboard.h"
#include "mem.h"
#include "pic.h"
#include "timer.h"
#include "x86.h"

int nmi = 0;

int nextcyc=0;
int cycdiff;
int is8086=0;

int memcycs;
int nopageerrors=0;

void FETCHCOMPLETE();

uint8_t readmembl(uint32_t addr);
void writemembl(uint32_t addr, uint8_t val);
uint16_t readmemwl(uint32_t seg, uint32_t addr);
void writememwl(uint32_t seg, uint32_t addr, uint16_t val);
uint32_t readmemll(uint32_t seg, uint32_t addr);
void writememll(uint32_t seg, uint32_t addr, uint32_t val);

#undef readmemb
#undef readmemw
uint8_t readmemb(uint32_t a)
{
        if (a!=(cs+pc)) memcycs+=4;
        if (readlookup2[(a)>>12]==0xFFFFFFFF) return readmembl(a);
        else return ram[readlookup2[(a)>>12]+((a)&0xFFF)];
}

uint8_t readmembf(uint32_t a)
{
        if (readlookup2[(a)>>12]==0xFFFFFFFF) return readmembl(a);
        else return ram[readlookup2[(a)>>12]+((a)&0xFFF)];
}

uint16_t readmemw(uint32_t s, uint16_t a)
{
        if (a!=(cs+pc)) memcycs+=(8>>is8086);
        if ((readlookup2[((s)+(a))>>12]==0xFFFFFFFF || (s)==0xFFFFFFFF)) return readmemwl(s,a);
        else return *((uint16_t *)(&ram[readlookup2[((s)+(a))>>12]+(((s)+(a))&0xFFF)]));
}

void refreshread() { /*pclog("Refreshread\n"); */FETCHCOMPLETE(); memcycs+=4; }

#undef fetchea
#define fetchea()   { rmdat=FETCH();  \
                    reg=(rmdat>>3)&7;             \
                    mod=(rmdat>>6)&3;             \
                    rm=rmdat&7;                   \
                    if (mod!=3) fetcheal(); }

void writemembl(uint32_t addr, uint8_t val);
void writememb(uint32_t a, uint8_t v)
{
        memcycs+=4;
        if (writelookup2[(a)>>12]==0xFFFFFFFF) writemembl(a,v);
        else ram[writelookup2[(a)>>12]+((a)&0xFFF)]=v;
}
void writememwl(uint32_t seg, uint32_t addr, uint16_t val);
void writememw(uint32_t s, uint32_t a, uint16_t v)
{
        memcycs+=(8>>is8086);
        if (writelookup2[((s)+(a))>>12]==0xFFFFFFFF || (s)==0xFFFFFFFF) writememwl(s,a,v);
        else *((uint16_t *)(&ram[writelookup2[((s)+(a))>>12]+(((s)+(a))&0xFFF)]))=v;
}
void writememll(uint32_t seg, uint32_t addr, uint32_t val);
void writememl(uint32_t s, uint32_t a, uint32_t v)
{
        if (writelookup2[((s)+(a))>>12]==0xFFFFFFFF || (s)==0xFFFFFFFF) writememll(s,a,v);
        else *((uint32_t *)(&ram[writelookup2[((s)+(a))>>12]+(((s)+(a))&0xFFF)]))=v;
}


void dumpregs();
uint16_t oldcs;
int oldcpl;

int tempc;
uint8_t opcode;
int times=0;
uint16_t pc2,pc3;
int noint=0;

int output=0;

int shadowbios=0;

int ins=0;
//#define readmemb(a) (((a)<0xA0000)?ram[a]:readmembl(a))

int ssegs;

int fetchcycles=0,memcycs,fetchclocks;

uint8_t prefetchqueue[6];
uint16_t prefetchpc;
int prefetchw=0;
inline uint8_t FETCH()
{
        uint8_t temp;
/*        temp=prefetchqueue[0];
        prefetchqueue[0]=prefetchqueue[1];
        prefetchqueue[1]=prefetchqueue[2];
        prefetchqueue[2]=prefetchqueue[3];
        prefetchqueue[3]=prefetchqueue[4];
        prefetchqueue[4]=prefetchqueue[5];
        if (prefetchw<=((is8086)?4:3))
        {
                prefetchqueue[prefetchw++]=readmembf(cs+prefetchpc); prefetchpc++;
                if (is8086 && (prefetchpc&1))
                {
                        prefetchqueue[prefetchw++]=readmembf(cs+prefetchpc); prefetchpc++;
                }
        }*/

//        uint8_t temp=readmemb(cs+pc);
//        if (output) printf("FETCH %04X %i\n",pc,fetchcycles);
        if (prefetchw==0) //(fetchcycles<4)
        {
                cycles-=(4-(fetchcycles&3));
                fetchclocks+=(4-(fetchcycles&3));
                fetchcycles=4;
                temp=readmembf(cs+pc);
                prefetchpc=pc=pc+1;
//                if (output) printf("   FETCH %04X:%04X %02X %04X %04X %i\n",CS,pc-1,temp,pc,prefetchpc,prefetchw);
                if (is8086 && (pc&1))
                {
                        prefetchqueue[0]=readmembf(cs+pc);
//                        if (output) printf("   PREFETCHED from %04X:%04X %02X 8086\n",CS,prefetchpc,prefetchqueue[prefetchw]);
                        prefetchpc++;
                        prefetchw++;
                }
        }
        else
        {
                temp=prefetchqueue[0];
                prefetchqueue[0]=prefetchqueue[1];
                prefetchqueue[1]=prefetchqueue[2];
                prefetchqueue[2]=prefetchqueue[3];
                prefetchqueue[3]=prefetchqueue[4];
                prefetchqueue[4]=prefetchqueue[5];
                prefetchw--;
//                if (output) printf("PREFETCH %04X:%04X %02X %04X %04X %i\n",CS,pc,temp,pc,prefetchpc,prefetchw);
                fetchcycles-=4;
//                fetchclocks+=4;
                pc++;
        }
//        if (output) printf("%i\n",fetchcycles);
        return temp;
}

inline void FETCHADD(int c)
{
        int d;
//        if (output) printf("FETCHADD %i\n",c);
        if (c<0) return;
        if (prefetchw>((is8086)?4:3)) return;
        d=c+(fetchcycles&3);
        while (d>3 && prefetchw<((is8086)?6:4))
        {
                d-=4;
                if (is8086 && !(prefetchpc&1))
                {
                        prefetchqueue[prefetchw]=readmembf(cs+prefetchpc);
//                        printf("PREFETCHED from %04X:%04X %02X 8086\n",CS,prefetchpc,prefetchqueue[prefetchw]);
                        prefetchpc++;
                        prefetchw++;
                }
                if (prefetchw<6)
                {
                        prefetchqueue[prefetchw]=readmembf(cs+prefetchpc);
//                        printf("PREFETCHED from %04X:%04X %02X\n",CS,prefetchpc,prefetchqueue[prefetchw]);
                        prefetchpc++;
                        prefetchw++;
                }
        }
        fetchcycles+=c;
        if (fetchcycles>16) fetchcycles=16;
//        if (fetchcycles>24) fetchcycles=24;
}

void FETCHCOMPLETE()
{
//        pclog("Fetchcomplete %i %i %i\n",fetchcycles&3,fetchcycles,prefetchw);
        if (!(fetchcycles&3)) return;
        if (prefetchw>((is8086)?4:3)) return;
        if (!prefetchw) nextcyc=(4-(fetchcycles&3));
        cycles-=(4-(fetchcycles&3));
        fetchclocks+=(4-(fetchcycles&3));
                if (is8086 && !(prefetchpc&1))
                {
                        prefetchqueue[prefetchw]=readmembf(cs+prefetchpc);
//                        printf("PREFETCHEDc from %04X:%04X %02X 8086\n",CS,prefetchpc,prefetchqueue[prefetchw]);
                        prefetchpc++;
                        prefetchw++;
                }
                if (prefetchw<6)
                {
                        prefetchqueue[prefetchw]=readmembf(cs+prefetchpc);
//                        printf("PREFETCHEDc from %04X:%04X %02X\n",CS,prefetchpc,prefetchqueue[prefetchw]);
                        prefetchpc++;
                        prefetchw++;
                }
                fetchcycles+=(4-(fetchcycles&3));
}

inline void FETCHCLEAR()
{
/*        int c;
        fetchcycles=0;
        prefetchpc=pc;
        if (is8086 && (prefetchpc&1)) cycles-=4;
        for (c=0;c<((is8086)?6:4);c++)
        {
                prefetchqueue[c]=readmembf(cs+prefetchpc);
                if (!is8086 || !(prefetchpc&1)) cycles-=4;
                prefetchpc++;
        }
        prefetchw=(is8086)?6:4;*/
//        fetchcycles=0;
        prefetchpc=pc;
        prefetchw=0;
        memcycs=cycdiff-cycles;
        fetchclocks=0;
//        memcycs=cycles;
/*        prefetchqueue[0]=readmembf(cs+prefetchpc);
        prefetchpc++;
        prefetchw=1;
        if (is8086 && prefetchpc&1)
        {
                prefetchqueue[1]=readmembf(cs+prefetchpc);
                prefetchpc++;
        }*/
}

static uint16_t getword()
{
        uint8_t temp=FETCH();
        return temp|(FETCH()<<8);
}


/*EA calculation*/

/*R/M - bits 0-2 - R/M   bits 3-5 - Reg   bits 6-7 - mod
  From 386 programmers manual :
r8(/r)                     AL    CL    DL    BL    AH    CH    DH    BH
r16(/r)                    AX    CX    DX    BX    SP    BP    SI    DI
r32(/r)                    EAX   ECX   EDX   EBX   ESP   EBP   ESI   EDI
/digit (Opcode)            0     1     2     3     4     5     6     7
REG =                      000   001   010   011   100   101   110   111
  ����Address
disp8 denotes an 8-bit displacement following the ModR/M byte, to be
sign-extended and added to the index. disp16 denotes a 16-bit displacement
following the ModR/M byte, to be added to the index. Default segment
register is SS for the effective addresses containing a BP index, DS for
other effective addresses.
            �Ŀ �Mod R/M� ���������ModR/M Values in Hexadecimal�������Ŀ

[BX + SI]            000   00    08    10    18    20    28    30    38
[BX + DI]            001   01    09    11    19    21    29    31    39
[BP + SI]            010   02    0A    12    1A    22    2A    32    3A
[BP + DI]            011   03    0B    13    1B    23    2B    33    3B
[SI]             00  100   04    0C    14    1C    24    2C    34    3C
[DI]                 101   05    0D    15    1D    25    2D    35    3D
disp16               110   06    0E    16    1E    26    2E    36    3E
[BX]                 111   07    0F    17    1F    27    2F    37    3F

[BX+SI]+disp8        000   40    48    50    58    60    68    70    78
[BX+DI]+disp8        001   41    49    51    59    61    69    71    79
[BP+SI]+disp8        010   42    4A    52    5A    62    6A    72    7A
[BP+DI]+disp8        011   43    4B    53    5B    63    6B    73    7B
[SI]+disp8       01  100   44    4C    54    5C    64    6C    74    7C
[DI]+disp8           101   45    4D    55    5D    65    6D    75    7D
[BP]+disp8           110   46    4E    56    5E    66    6E    76    7E
[BX]+disp8           111   47    4F    57    5F    67    6F    77    7F

[BX+SI]+disp16       000   80    88    90    98    A0    A8    B0    B8
[BX+DI]+disp16       001   81    89    91    99    A1    A9    B1    B9
[BX+SI]+disp16       010   82    8A    92    9A    A2    AA    B2    BA
[BX+DI]+disp16       011   83    8B    93    9B    A3    AB    B3    BB
[SI]+disp16      10  100   84    8C    94    9C    A4    AC    B4    BC
[DI]+disp16          101   85    8D    95    9D    A5    AD    B5    BD
[BP]+disp16          110   86    8E    96    9E    A6    AE    B6    BE
[BX]+disp16          111   87    8F    97    9F    A7    AF    B7    BF

EAX/AX/AL            000   C0    C8    D0    D8    E0    E8    F0    F8
ECX/CX/CL            001   C1    C9    D1    D9    E1    E9    F1    F9
EDX/DX/DL            010   C2    CA    D2    DA    E2    EA    F2    FA
EBX/BX/BL            011   C3    CB    D3    DB    E3    EB    F3    FB
ESP/SP/AH        11  100   C4    CC    D4    DC    E4    EC    F4    FC
EBP/BP/CH            101   C5    CD    D5    DD    E5    ED    F5    FD
ESI/SI/DH            110   C6    CE    D6    DE    E6    EE    F6    FE
EDI/DI/BH            111   C7    CF    D7    DF    E7    EF    F7    FF

mod = 11 - register
      10 - address + 16 bit displacement
      01 - address + 8 bit displacement
      00 - address

reg = If mod=11,  (depending on data size, 16 bits/8 bits, 32 bits=extend 16 bit registers)
      0=AX/AL   1=CX/CL   2=DX/DL   3=BX/BL
      4=SP/AH   5=BP/CH   6=SI/DH   7=DI/BH

      Otherwise, LSB selects SI/DI (0=SI), NMSB selects BX/BP (0=BX), and MSB
      selects whether BX/BP are used at all (0=used).

      mod=00 is an exception though
      6=16 bit displacement only
      7=[BX]

      Usage varies with instructions.

      MOV AL,BL has ModR/M as C3, for example.
      mod=11, reg=0, r/m=3
      MOV uses reg as dest, and r/m as src.
      reg 0 is AL, reg 3 is BL

      If BP or SP are in address calc, seg is SS, else DS
*/

int cycles=0;
uint32_t easeg,eaaddr;
int rm,reg,mod,rmdat;

uint16_t zero=0;
uint16_t *mod1add[2][8];
uint32_t *mod1seg[8];

int slowrm[8];

void makemod1table()
{
        mod1add[0][0]=&BX; mod1add[0][1]=&BX; mod1add[0][2]=&BP; mod1add[0][3]=&BP;
        mod1add[0][4]=&SI; mod1add[0][5]=&DI; mod1add[0][6]=&BP; mod1add[0][7]=&BX;
        mod1add[1][0]=&SI; mod1add[1][1]=&DI; mod1add[1][2]=&SI; mod1add[1][3]=&DI;
        mod1add[1][4]=&zero; mod1add[1][5]=&zero; mod1add[1][6]=&zero; mod1add[1][7]=&zero;
        slowrm[0]=0; slowrm[1]=1; slowrm[2]=1; slowrm[3]=0;
        mod1seg[0]=&ds; mod1seg[1]=&ds; mod1seg[2]=&ss; mod1seg[3]=&ss;
        mod1seg[4]=&ds; mod1seg[5]=&ds; mod1seg[6]=&ss; mod1seg[7]=&ds;
}

static void fetcheal()
{
        if (!mod && rm==6) { eaaddr=getword(); easeg=ds; FETCHADD(6); }
        else
        {
                switch (mod)
                {
                        case 0:
                        eaaddr=0;
                        if (rm&4) FETCHADD(5);
                        else      FETCHADD(7+slowrm[rm]);
                        break;
                        case 1:
                        eaaddr=(uint16_t)(int8_t)FETCH();
                        if (rm&4) FETCHADD(9);
                        else      FETCHADD(11+slowrm[rm]);
                        break;
                        case 2:
                        eaaddr=getword();
                        if (rm&4) FETCHADD(9);
                        else      FETCHADD(11+slowrm[rm]);
                        break;
                }
                eaaddr+=(*mod1add[0][rm])+(*mod1add[1][rm]);
                easeg=*mod1seg[rm];
                eaaddr&=0xFFFF;
        }
}

static inline uint8_t geteab()
{
        if (mod==3)
           return (rm&4)?regs[rm&3].b.h:regs[rm&3].b.l;
        return readmemb(easeg+eaaddr);
}

static inline uint16_t geteaw()
{
        if (mod==3)
           return regs[rm].w;
//        if (output==3) printf("GETEAW %04X:%08X\n",easeg,eaaddr);
        return readmemw(easeg,eaaddr);
}

static inline uint16_t geteaw2()
{
        if (mod==3)
           return regs[rm].w;
//        printf("Getting addr from %04X:%04X %05X\n",easeg,eaaddr+2,easeg+eaaddr+2);
        return readmemw(easeg,(eaaddr+2)&0xFFFF);
}

static inline void seteab(uint8_t val)
{
        if (mod==3)
        {
                if (rm&4) regs[rm&3].b.h=val;
                else      regs[rm&3].b.l=val;
        }
        else
        {
                writememb(easeg+eaaddr,val);
        }
}

static inline void seteaw(uint16_t val)
{
        if (mod==3)
           regs[rm].w=val;
        else
        {
                writememw(easeg,eaaddr,val);
//                writememb(easeg+eaaddr+1,val>>8);
        }
}

#define getr8(r)   ((r&4)?regs[r&3].b.h:regs[r&3].b.l)

#define setr8(r,v) if (r&4) regs[r&3].b.h=v; \
                   else     regs[r&3].b.l=v;


/*Flags*/
uint8_t znptable8[256];
uint16_t znptable16[65536];

void makeznptable()
{
        int c,d;
        for (c=0;c<256;c++)
        {
                d=0;
                if (c&1) d++;
                if (c&2) d++;
                if (c&4) d++;
                if (c&8) d++;
                if (c&16) d++;
                if (c&32) d++;
                if (c&64) d++;
                if (c&128) d++;
                if (d&1)
                   znptable8[c]=0;
                else
                   znptable8[c]=P_FLAG;
                   if (c == 0xb1) pclog("znp8 b1 = %i %02X\n", d, znptable8[c]);
                if (!c) znptable8[c]|=Z_FLAG;
                if (c&0x80) znptable8[c]|=N_FLAG;
        }
        for (c=0;c<65536;c++)
        {
                d=0;
                if (c&1) d++;
                if (c&2) d++;
                if (c&4) d++;
                if (c&8) d++;
                if (c&16) d++;
                if (c&32) d++;
                if (c&64) d++;
                if (c&128) d++;
                if (d&1)
                   znptable16[c]=0;
                else
                   znptable16[c]=P_FLAG;
                if (c == 0xb1) pclog("znp16 b1 = %i %02X\n", d, znptable16[c]);
                if (c == 0x65b1) pclog("znp16 65b1 = %i %02X\n", d, znptable16[c]);
                if (!c) znptable16[c]|=Z_FLAG;
                if (c&0x8000) znptable16[c]|=N_FLAG;
      }
      
//      makemod1table();
}
int timetolive=0;

extern uint32_t oldcs2;
extern uint32_t oldpc2;

int indump = 0;

void dumpregs()
{
        int c,d=0,e=0,ff;
#ifndef RELEASE_BUILD
        FILE *f;
        if (indump) return;
        indump = 1;
//        return;
        output=0;
//        return;
//        savenvr();
//        return;
chdir(pcempath);
        nopageerrors=1;
/*        f=fopen("rram3.dmp","wb");
        for (c=0;c<0x8000000;c++) putc(readmemb(c+0x10000000),f);
        fclose(f);*/
        f=fopen("ram.dmp","wb");
        fwrite(ram,mem_size*1024*1024,1,f);
        fclose(f);
/*        pclog("Dumping rram5.dmp\n");
        f=fopen("rram5.dmp","wb");
        for (c=0;c<0x1000000;c++) putc(readmemb(c+0x10150000),f);
        fclose(f);*/
        pclog("Dumping rram.dmp\n");
        f=fopen("rram.dmp","wb");
        for (c=0;c<0x1000000;c++) putc(readmemb(c),f);
        fclose(f);
/*        f=fopen("rram2.dmp","wb");
        for (c=0;c<0x100000;c++) putc(readmemb(c+0xbff00000),f);
        fclose(f);
        f = fopen("stack.dmp","wb");
        for (c = 0; c < 0x6000; c++) putc(readmemb(c+0xFFDFA000), f);
        fclose(f);
        f = fopen("tempx.dmp","wb");
        for (c = 0; c < 0x10000; c++) putc(readmemb(c+0xFC816000), f);
        fclose(f);
        f = fopen("tempx2.dmp","wb");
        for (c = 0; c < 0x10000; c++) putc(readmemb(c+0xFDEF5000), f);
        fclose(f);*/
        pclog("Dumping rram4.dmp\n");
        f=fopen("rram4.dmp","wb");
        for (c=0;c<0x0050000;c++) 
        {
                abrt = 0;
                putc(readmemb386l(0,c+0x80000000),f);
        }
        fclose(f);
        pclog("Dumping done\n");        
/*        f=fopen("rram6.dmp","wb");
        for (c=0;c<0x1000000;c++) putc(readmemb(c+0xBF000000),f);
        fclose(f);*/
/*        f=fopen("ram6.bin","wb");
        fwrite(ram+0x10100,0xA000,1,f);
        fclose(f);
        f=fopen("boot.bin","wb");
        fwrite(ram+0x7C00,0x200,1,f);
        fclose(f);
        f=fopen("ram7.bin","wb");
        fwrite(ram+0x11100,0x2000,1,f);
        fclose(f);
        f=fopen("ram8.bin","wb");
        fwrite(ram+0x3D210,0x200,1,f);
        fclose(f);        */
/*        f=fopen("bios.dmp","wb");
        fwrite(rom,0x20000,1,f);
        fclose(f);*/
/*        f=fopen("kernel.dmp","wb");
        for (c=0;c<0x200000;c++) putc(readmemb(c+0xC0000000),f);
        fclose(f);*/
/*        f=fopen("rram.dmp","wb");
        for (c=0;c<0x1500000;c++) putc(readmemb(c),f);
        fclose(f);
        if (!times)
        {
                f=fopen("thing.dmp","wb");
                fwrite(ram+0x11E50,0x1000,1,f);
                fclose(f);
        }*/
#endif
        if (is386)
           printf("EAX=%08X EBX=%08X ECX=%08X EDX=%08X\nEDI=%08X ESI=%08X EBP=%08X ESP=%08X\n",EAX,EBX,ECX,EDX,EDI,ESI,EBP,ESP);
        else
           printf("AX=%04X BX=%04X CX=%04X DX=%04X DI=%04X SI=%04X BP=%04X SP=%04X\n",AX,BX,CX,DX,DI,SI,BP,SP);
        printf("PC=%04X CS=%04X DS=%04X ES=%04X SS=%04X FLAGS=%04X\n",pc,CS,DS,ES,SS,flags);
        printf("%04X:%04X %04X:%04X\n",oldcs,oldpc, oldcs2, oldpc2);
        printf("%i ins\n",ins);
        if (is386)
           printf("In %s mode\n",(msw&1)?((eflags&VM_FLAG)?"V86":"protected"):"real");
        else
           printf("In %s mode\n",(msw&1)?"protected":"real");
        printf("CS : base=%06X limit=%04X access=%02X\n",cs,_cs.limit,_cs.access);
        printf("DS : base=%06X limit=%04X access=%02X\n",ds,_ds.limit,_ds.access);
        printf("ES : base=%06X limit=%04X access=%02X\n",es,_es.limit,_es.access);
        if (is386)
        {
                printf("FS : base=%06X limit=%04X access=%02X\n",fs,_fs.limit,_fs.access);
                printf("GS : base=%06X limit=%04X access=%02X\n",gs,_gs.limit,_gs.access);
        }
        printf("SS : base=%06X limit=%04X access=%02X\n",ss,_ss.limit,_ss.access);
        printf("GDT : base=%06X limit=%04X\n",gdt.base,gdt.limit);
        printf("LDT : base=%06X limit=%04X\n",ldt.base,ldt.limit);
        printf("IDT : base=%06X limit=%04X\n",idt.base,idt.limit);
        printf("TR  : base=%06X limit=%04X\n", tr.base, tr.limit);
        if (is386)
        {
                printf("386 in %s mode   stack in %s mode\n",(use32)?"32-bit":"16-bit",(stack32)?"32-bit":"16-bit");
                printf("CR0=%08X CR2=%08X CR3=%08X\n",cr0,cr2,cr3);
        }
        printf("Entries in readlookup : %i    writelookup : %i\n",readlnum,writelnum);
        for (c=0;c<1024*1024;c++)
        {
                if (readlookup2[c]!=0xFFFFFFFF) d++;
                if (writelookup2[c]!=0xFFFFFFFF) e++;
        }
        printf("Entries in readlookup : %i    writelookup : %i\n",d,e);
        x87_dumpregs();
/*        for (c=0;c<1024*1024;c++)
        {
                if (mmucache[c]!=0xFFFFFFFF) d++;
        }
        printf("Entries in MMU cache : %i\n",d);*/
        indump = 0;
}

int resets = 0;
void resetx86()
{
        pclog("x86 reset\n");
        resets++;
        ins = 0;
        use32=0;
        stack32=0;
//        i86_Reset();
//        cs=0xFFFF0;
        pc=0;
        msw=0;
        cr0=0;
        eflags=0;
        cgate32=0;
        loadcs(0xFFFF);
        rammask=0xFFFFFFFF;
        idt.base = 0;
        flags=2;
        makeznptable();
        initmmucache();
        resetreadlookup();
        makemod1table();
        resetmcr();
        FETCHCLEAR();
        x87_reset();
        cpu_set_edx();
        ESP=0;
        mmu_perm=4;
        memset(inscounts, 0, sizeof(inscounts));
}

void softresetx86()
{
//      dumpregs();
//        exit(-1);
        use32=0;
        stack32=0;
//        i86_Reset();
//        cs=0xFFFF0;
        pc=0;
        msw=0;
        cr0=0;
        eflags=0;
        cgate32=0;
        loadcs(0xFFFF);
        //rammask=0xFFFFFFFF;
        flags=2;
        idt.base = 0;
}

static void setznp8(uint8_t val)
{
        flags&=~0xC4;
        flags|=znptable8[val];
}

static void setznp16(uint16_t val)
{
        flags&=~0xC4;
        flags|=znptable16[val];
}

static void setadd8(uint8_t a, uint8_t b)
{
        uint16_t c=(uint16_t)a+(uint16_t)b;
        flags&=~0x8D5;
        flags|=znptable8[c&0xFF];
        if (c&0x100) flags|=C_FLAG;
        if (!((a^b)&0x80)&&((a^c)&0x80)) flags|=V_FLAG;
        if (((a&0xF)+(b&0xF))&0x10)      flags|=A_FLAG;
}
static void setadd8nc(uint8_t a, uint8_t b)
{
        uint16_t c=(uint16_t)a+(uint16_t)b;
        flags&=~0x8D4;
        flags|=znptable8[c&0xFF];
        if (!((a^b)&0x80)&&((a^c)&0x80)) flags|=V_FLAG;
        if (((a&0xF)+(b&0xF))&0x10)      flags|=A_FLAG;
}
static void setadc8(uint8_t a, uint8_t b)
{
        uint16_t c=(uint16_t)a+(uint16_t)b+tempc;
        flags&=~0x8D5;
        flags|=znptable8[c&0xFF];
        if (c&0x100) flags|=C_FLAG;
        if (!((a^b)&0x80)&&((a^c)&0x80)) flags|=V_FLAG;
        if (((a&0xF)+(b&0xF))&0x10)      flags|=A_FLAG;
}
static void setadd16(uint16_t a, uint16_t b)
{
        uint32_t c=(uint32_t)a+(uint32_t)b;
        flags&=~0x8D5;
        flags|=znptable16[c&0xFFFF];
        if (c&0x10000) flags|=C_FLAG;
        if (!((a^b)&0x8000)&&((a^c)&0x8000)) flags|=V_FLAG;
        if (((a&0xF)+(b&0xF))&0x10)      flags|=A_FLAG;
}
static void setadd16nc(uint16_t a, uint16_t b)
{
        uint32_t c=(uint32_t)a+(uint32_t)b;
        flags&=~0x8D4;
        flags|=znptable16[c&0xFFFF];
        if (!((a^b)&0x8000)&&((a^c)&0x8000)) flags|=V_FLAG;
        if (((a&0xF)+(b&0xF))&0x10)      flags|=A_FLAG;
}
static void setadc16(uint16_t a, uint16_t b)
{
        uint32_t c=(uint32_t)a+(uint32_t)b+tempc;
        flags&=~0x8D5;
        flags|=znptable16[c&0xFFFF];
        if (c&0x10000) flags|=C_FLAG;
        if (!((a^b)&0x8000)&&((a^c)&0x8000)) flags|=V_FLAG;
        if (((a&0xF)+(b&0xF))&0x10)      flags|=A_FLAG;
}

static void setsub8(uint8_t a, uint8_t b)
{
        uint16_t c=(uint16_t)a-(uint16_t)b;
        flags&=~0x8D5;
        flags|=znptable8[c&0xFF];
        if (c&0x100) flags|=C_FLAG;
        if ((a^b)&(a^c)&0x80) flags|=V_FLAG;
        if (((a&0xF)-(b&0xF))&0x10)      flags|=A_FLAG;
}
static void setsub8nc(uint8_t a, uint8_t b)
{
        uint16_t c=(uint16_t)a-(uint16_t)b;
        flags&=~0x8D4;
        flags|=znptable8[c&0xFF];
        if ((a^b)&(a^c)&0x80) flags|=V_FLAG;
        if (((a&0xF)-(b&0xF))&0x10)      flags|=A_FLAG;
}
static void setsbc8(uint8_t a, uint8_t b)
{
        uint16_t c=(uint16_t)a-(((uint16_t)b)+tempc);
        flags&=~0x8D5;
        flags|=znptable8[c&0xFF];
        if (c&0x100) flags|=C_FLAG;
        if ((a^b)&(a^c)&0x80) flags|=V_FLAG;
        if (((a&0xF)-(b&0xF))&0x10)      flags|=A_FLAG;
}
static void setsub16(uint16_t a, uint16_t b)
{
        uint32_t c=(uint32_t)a-(uint32_t)b;
        flags&=~0x8D5;
        flags|=znptable16[c&0xFFFF];
        if (c&0x10000) flags|=C_FLAG;
        if ((a^b)&(a^c)&0x8000) flags|=V_FLAG;
//        if (output) printf("%04X %04X %i\n",a^b,a^c,flags&V_FLAG);
        if (((a&0xF)-(b&0xF))&0x10)      flags|=A_FLAG;
}
static void setsub16nc(uint16_t a, uint16_t b)
{
        uint32_t c=(uint32_t)a-(uint32_t)b;
        flags&=~0x8D4;
        flags|=(znptable16[c&0xFFFF]&~4);
        flags|=(znptable8[c&0xFF]&4);
        if ((a^b)&(a^c)&0x8000) flags|=V_FLAG;
        if (((a&0xF)-(b&0xF))&0x10)      flags|=A_FLAG;
}
static void setsbc16(uint16_t a, uint16_t b)
{
        uint32_t c=(uint32_t)a-(((uint32_t)b)+tempc);
        flags&=~0x8D5;
        flags|=(znptable16[c&0xFFFF]&~4);
        flags|=(znptable8[c&0xFF]&4);
        if (c&0x10000) flags|=C_FLAG;
        if ((a^b)&(a^c)&0x8000) flags|=V_FLAG;
        if (((a&0xF)-(b&0xF))&0x10)      flags|=A_FLAG;
}

int current_diff = 0;
void clockhardware()
{
        int diff = cycdiff - cycles - current_diff;
        
        current_diff += diff;
        if (pit.running[0]) pit.c[0] -= diff;
        if (pit.running[1]) pit.c[1] -= diff;
        if (pit.running[2]) pit.c[2] -= diff;
        if ((pit.c[0] < 1) || (pit.c[1] < 1) || (pit.c[2] < 1)) pit_poll();
        
        timer_clock(diff);
}

static int takeint = 0;


int firstrepcycle=1;

void rep(int fv)
{
        uint8_t temp;
        int c=CX;
        uint8_t temp2;
        uint16_t tempw,tempw2;
        uint16_t ipc=oldpc;//pc-1;
        int changeds=0;
        uint32_t oldds;
        startrep:
        temp=FETCH();

//        if (firstrepcycle && temp==0xA5) printf("REP MOVSW %06X:%04X %06X:%04X\n",ds,SI,es,DI);
//        if (output) printf("REP %02X %04X\n",temp,ipc);
        switch (temp)
        {
                case 0x08:
                pc=ipc+1;
                cycles-=2;
                FETCHCLEAR();
                break;
                case 0x26: /*ES:*/
                oldds=ds;
                ds=es;
                changeds=1;
                cycles-=2;
                goto startrep;
                break;
                case 0x2E: /*CS:*/
                oldds=ds;
                ds=cs;
                changeds=1;
                cycles-=2;
                goto startrep;
                break;
                case 0x36: /*SS:*/
                oldds=ds;
                ds=ss;
                changeds=1;
                cycles-=2;
                goto startrep;
                break;
                case 0x6E: /*REP OUTSB*/
                if (c>0)
                {
                        temp2=readmemb(ds+SI);
                        outb(DX,temp2);
                        if (flags&D_FLAG) SI--;
                        else              SI++;
                        c--;
                        cycles-=5;
                }
                if (c>0) { firstrepcycle=0; pc=ipc; if (ssegs) ssegs++; FETCHCLEAR(); }
                else firstrepcycle=1;
                break;
                case 0xA4: /*REP MOVSB*/
                while (c>0 && !IRQTEST)
                {
                        temp2=readmemb(ds+SI);
                        writememb(es+DI,temp2);
//                        if (output) printf("Moved %02X from %04X:%04X to %04X:%04X\n",temp2,ds>>4,SI,es>>4,DI);
                        if (flags&D_FLAG) { DI--; SI--; }
                        else              { DI++; SI++; }
                        c--;
                        cycles-=17;
                        clockhardware();
                        FETCHADD(17-memcycs);
                }
                if (IRQTEST && c>0) pc=ipc;
//                if (c>0) { firstrepcycle=0; pc=ipc; if (ssegs) ssegs++; FETCHCLEAR(); }
//                else firstrepcycle=1;
//                }
                break;
                case 0xA5: /*REP MOVSW*/
                while (c>0 && !IRQTEST)
                {
                        memcycs=0;
                        tempw=readmemw(ds,SI);
                        writememw(es,DI,tempw);
                        if (flags&D_FLAG) { DI-=2; SI-=2; }
                        else              { DI+=2; SI+=2; }
                        c--;
                        cycles-=17;
                        clockhardware();
                        FETCHADD(17 - memcycs);
                }
                if (IRQTEST && c>0) pc=ipc;
//                if (c>0) { firstrepcycle=0; pc=ipc; if (ssegs) ssegs++; FETCHCLEAR(); }
//                else firstrepcycle=1;
//                }
                break;
                case 0xA6: /*REP CMPSB*/
                if (fv) flags|=Z_FLAG;
                else    flags&=~Z_FLAG;
                while ((c>0) && (fv==((flags&Z_FLAG)?1:0)) && !IRQTEST)
                {
                        memcycs=0;
                        temp=readmemb(ds+SI);
                        temp2=readmemb(es+DI);
//                        printf("CMPSB %c %c %i %05X %05X %04X:%04X\n",temp,temp2,c,ds+SI,es+DI,cs>>4,pc);
                        if (flags&D_FLAG) { DI--; SI--; }
                        else              { DI++; SI++; }
                        c--;
                        cycles -= 30;
                        setsub8(temp,temp2);
                        clockhardware();
                        FETCHADD(30 - memcycs);
                }
                if (IRQTEST && c>0 && (fv==((flags&Z_FLAG)?1:0))) pc=ipc;
//                if ((c>0) && (fv==((flags&Z_FLAG)?1:0))) { pc=ipc; firstrepcycle=0; if (ssegs) ssegs++; FETCHCLEAR(); }
//                else firstrepcycle=1;
                break;
                case 0xA7: /*REP CMPSW*/
                if (fv) flags|=Z_FLAG;
                else    flags&=~Z_FLAG;
                while ((c>0) && (fv==((flags&Z_FLAG)?1:0)) && !IRQTEST)
                {
                        memcycs=0;
                        tempw=readmemw(ds,SI);
                        tempw2=readmemw(es,DI);
                        if (flags&D_FLAG) { DI-=2; SI-=2; }
                        else              { DI+=2; SI+=2; }
                        c--;
                        cycles -= 30;
                        setsub16(tempw,tempw2);
                        clockhardware();
                        FETCHADD(30 - memcycs);
                }
                if (IRQTEST && c>0 && (fv==((flags&Z_FLAG)?1:0))) pc=ipc;
//                if ((c>0) && (fv==((flags&Z_FLAG)?1:0))) { pc=ipc; firstrepcycle=0; if (ssegs) ssegs++; FETCHCLEAR(); }
//                else firstrepcycle=1;
//                if (firstrepcycle) printf("REP CMPSW  %06X:%04X %06X:%04X %04X %04X\n",ds,SI,es,DI,tempw,tempw2);
                break;
                case 0xAA: /*REP STOSB*/
                while (c>0 && !IRQTEST)
                {
                        memcycs=0;
                        writememb(es+DI,AL);
                        if (flags&D_FLAG) DI--;
                        else              DI++;
                        c--;
                        cycles -= 10;
                        clockhardware();
                        FETCHADD(10 - memcycs);
                }
                if (IRQTEST && c>0) pc=ipc;
//                if (c>0) { firstrepcycle=0; pc=ipc; if (ssegs) ssegs++; FETCHCLEAR(); }
//                else firstrepcycle=1;
                break;
                case 0xAB: /*REP STOSW*/
                while (c>0 && !IRQTEST)
                {
                        memcycs=0;
                        writememw(es,DI,AX);
                        if (flags&D_FLAG) DI-=2;
                        else              DI+=2;
                        c--;
                        cycles -= 10;
                        clockhardware();
                        FETCHADD(10 - memcycs);
                }
                if (IRQTEST && c>0) pc=ipc;
//                if (c>0) { firstrepcycle=0; pc=ipc; if (ssegs) ssegs++; FETCHCLEAR(); }
//                else firstrepcycle=1;
                break;
                case 0xAC: /*REP LODSB*/
                if (c>0)
                {
                        temp2=readmemb(ds+SI);
                        if (flags&D_FLAG) SI--;
                        else              SI++;
                        c--;
                        cycles-=4;
                }
                if (c>0) { firstrepcycle=0; pc=ipc; if (ssegs) ssegs++; FETCHCLEAR(); }
                else firstrepcycle=1;
                break;
                case 0xAD: /*REP LODSW*/
                if (c>0)
                {
                        tempw2=readmemw(ds,SI);
                        if (flags&D_FLAG) SI-=2;
                        else              SI+=2;
                        c--;
                        cycles-=4;
                }
                if (c>0) { firstrepcycle=0; pc=ipc; if (ssegs) ssegs++; FETCHCLEAR(); }
                else firstrepcycle=1;
                break;
                case 0xAE: /*REP SCASB*/
                if (fv) flags|=Z_FLAG;
                else    flags&=~Z_FLAG;
                if ((c>0) && (fv==((flags&Z_FLAG)?1:0)))
                {
                        temp2=readmemb(es+DI);
//                        if (output) printf("SCASB %02X %c %02X %05X  ",temp2,temp2,AL,es+DI);
                        setsub8(AL,temp2);
//                        if (output && flags&Z_FLAG) printf("Match %02X %02X\n",AL,temp2);
                        if (flags&D_FLAG) DI--;
                        else              DI++;
                        c--;
                        cycles -= 15;
                }
//if (output)                printf("%i %i %i %i\n",c,(c>0),(fv==((flags&Z_FLAG)?1:0)),((c>0) && (fv==((flags&Z_FLAG)?1:0))));
                if ((c>0) && (fv==((flags&Z_FLAG)?1:0)))  { pc=ipc; firstrepcycle=0; if (ssegs) ssegs++; FETCHCLEAR(); }
                else firstrepcycle=1;
//                cycles-=120;
                break;
                case 0xAF: /*REP SCASW*/
                if (fv) flags|=Z_FLAG;
                else    flags&=~Z_FLAG;
                if ((c>0) && (fv==((flags&Z_FLAG)?1:0)))
                {
                        tempw=readmemw(es,DI);
                        setsub16(AX,tempw);
                        if (flags&D_FLAG) DI-=2;
                        else              DI+=2;
                        c--;
                        cycles -= 15;
                }
                if ((c>0) && (fv==((flags&Z_FLAG)?1:0)))  { pc=ipc; firstrepcycle=0; if (ssegs) ssegs++; FETCHCLEAR(); }
                else firstrepcycle=1;
                break;
                default:
                        pc=ipc;
                        cycles-=20;
                        FETCHCLEAR();
//                printf("Bad REP %02X\n",temp);
//                dumpregs();
//                exit(-1);
        }
        CX=c;
        if (changeds) ds=oldds;
        if (IRQTEST)
                takeint = 1;
//        if (pc==ipc) FETCHCLEAR();
}


int inhlt=0;
uint16_t lastpc,lastcs;
int firstrepcycle;
int skipnextprint=0;

int instime=0;
//#if 0
void execx86(int cycs)
{
        uint8_t temp,temp2;
        uint16_t addr,tempw,tempw2,tempw3,tempw4;
        int8_t offset;
        int tempws;
        uint32_t templ;
        int c;
        int tempi;
        int trap;

//        printf("Run x86! %i %i\n",cycles,cycs);
        cycles+=cycs;
//        i86_Execute(cycs);
//        return;
        while (cycles>0)
        {
//                old83=old82;
//                old82=old8;
//                old8=oldpc|(oldcs<<16);
//                if (pc==0x96B && cs==0x9E040) { printf("Hit it\n"); output=1; timetolive=150; }
//                if (pc<0x8000) printf("%04X : %04X %04X %04X %04X %04X %04X %04X %04X %04X %04X %04X %04X %02X %04X %i\n",pc,AX,BX,CX,DX,cs>>4,ds>>4,es>>4,ss>>4,DI,SI,BP,SP,opcode,flags,disctime);
                cycdiff=cycles;
                current_diff = 0;
                cycles-=nextcyc;
//                if (instime) pclog("Cycles %i %i\n",cycles,cycdiff);
                nextcyc=0;
//        if (output) printf("CLOCK %i %i\n",cycdiff,cycles);
                fetchclocks=0;
                oldcs=CS;
                oldpc=pc;
                opcodestart:
                opcode=FETCH();
                tempc=flags&C_FLAG;
                trap=flags&T_FLAG;
                pc--;
//                output=1;
//                if (output) printf("%04X:%04X : %04X %04X %04X %04X %04X %04X %04X %04X %04X %04X %04X %04X %02X %04X\n",cs>>4,pc,AX,BX,CX,DX,cs>>4,ds>>4,es>>4,ss>>4,DI,SI,BP,SP,opcode,flags&~0x200,rmdat);
//#if 0
                if (output)
                {
//                        if ((opcode!=0xF2 && opcode!=0xF3) || firstrepcycle)
//                        {
                                if (!skipnextprint) printf("%04X:%04X : %04X %04X %04X %04X %04X %04X %04X %04X %04X %04X %04X %04X %02X %04X  %i %p %02X\n",cs,pc,AX,BX,CX,DX,CS,DS,ES,SS,DI,SI,BP,SP,opcode,flags, ins, ram, ram[0x1a925]);
                                skipnextprint=0;
//                                ins++;
//                        }
                }
//#endif
                pc++;
                inhlt=0;
//                if (ins==500000) { dumpregs(); exit(0); }*/
                switch (opcode)
                {
                        case 0x00: /*ADD 8,reg*/
                        fetchea();
/*                        if (!rmdat) pc--;
                        if (!rmdat)
                        {
                                fatal("Crashed\n");
//                                clear_keybuf();
//                                readkey();
                        }*/
                        temp=geteab();
                        setadd8(temp,getr8(reg));
                        temp+=getr8(reg);
                        seteab(temp);
                        cycles-=((mod==3)?3:24);
                        break;
                        case 0x01: /*ADD 16,reg*/
                        fetchea();
                        tempw=geteaw();
                        setadd16(tempw,regs[reg].w);
                        tempw+=regs[reg].w;
                        seteaw(tempw);
                        cycles-=((mod==3)?3:24);
                        break;
                        case 0x02: /*ADD reg,8*/
                        fetchea();
                        temp=geteab();
                        setadd8(getr8(reg),temp);
                        setr8(reg,getr8(reg)+temp);
                        cycles-=((mod==3)?3:13);
                        break;
                        case 0x03: /*ADD reg,16*/
                        fetchea();
                        tempw=geteaw();
                        setadd16(regs[reg].w,tempw);
                        regs[reg].w+=tempw;
                        cycles-=((mod==3)?3:13);
                        break;
                        case 0x04: /*ADD AL,#8*/
                        temp=FETCH();
                        setadd8(AL,temp);
                        AL+=temp;
                        cycles-=4;
                        break;
                        case 0x05: /*ADD AX,#16*/
                        tempw=getword();
                        setadd16(AX,tempw);
                        AX+=tempw;
                        cycles-=4;
                        break;

                        case 0x06: /*PUSH ES*/
                        if (ssegs) ss=oldss;
                        writememw(ss,((SP-2)&0xFFFF),ES);
                        SP-=2;
                        cycles-=14;
                        break;
                        case 0x07: /*POP ES*/
                        if (ssegs) ss=oldss;
                        tempw=readmemw(ss,SP);
                        loadseg(tempw,&_es);
                        SP+=2;
                        cycles-=12;
                        break;

                        case 0x08: /*OR 8,reg*/
                        fetchea();
                        temp=geteab();
                        temp|=getr8(reg);
                        setznp8(temp);
                        flags&=~(C_FLAG|V_FLAG|A_FLAG);
                        seteab(temp);
                        cycles-=((mod==3)?3:24);
                        break;
                        case 0x09: /*OR 16,reg*/
                        fetchea();
                        tempw=geteaw();
                        tempw|=regs[reg].w;
                        setznp16(tempw);
                        flags&=~(C_FLAG|V_FLAG|A_FLAG);
                        seteaw(tempw);
                        cycles-=((mod==3)?3:24);
                        break;
                        case 0x0A: /*OR reg,8*/
                        fetchea();
                        temp=geteab();
                        temp|=getr8(reg);
                        setznp8(temp);
                        flags&=~(C_FLAG|V_FLAG|A_FLAG);
                        setr8(reg,temp);
                        cycles-=((mod==3)?3:13);
                        break;
                        case 0x0B: /*OR reg,16*/
                        fetchea();
                        tempw=geteaw();
                        tempw|=regs[reg].w;
                        setznp16(tempw);
                        flags&=~(C_FLAG|V_FLAG|A_FLAG);
                        regs[reg].w=tempw;
                        cycles-=((mod==3)?3:13);
                        break;
                        case 0x0C: /*OR AL,#8*/
                        AL|=FETCH();
                        setznp8(AL);
                        flags&=~(C_FLAG|V_FLAG|A_FLAG);
                        cycles-=4;
                        break;
                        case 0x0D: /*OR AX,#16*/
                        AX|=getword();
                        setznp16(AX);
                        flags&=~(C_FLAG|V_FLAG|A_FLAG);
                        cycles-=4;
                        break;

                        case 0x0E: /*PUSH CS*/
                        if (ssegs) ss=oldss;
                        writememw(ss,((SP-2)&0xFFFF),CS);
                        SP-=2;
                        cycles-=14;
                        break;
                        case 0x0F: /*POP CS - 8088/8086 only*/
                        if (ssegs) ss=oldss;
                        tempw=readmemw(ss,SP);
                        loadseg(tempw,&_cs);
                        SP+=2;
                        cycles-=12;
                        break;

                        case 0x10: /*ADC 8,reg*/
                        fetchea();
                        temp=geteab();
                        temp2=getr8(reg);
                        setadc8(temp,temp2);
                        temp+=temp2+tempc;
                        seteab(temp);
                        cycles-=((mod==3)?3:24);
                        break;
                        case 0x11: /*ADC 16,reg*/
                        fetchea();
                        tempw=geteaw();
                        tempw2=regs[reg].w;
                        setadc16(tempw,tempw2);
                        tempw+=tempw2+tempc;
                        seteaw(tempw);
                        cycles-=((mod==3)?3:24);
                        break;
                        case 0x12: /*ADC reg,8*/
                        fetchea();
                        temp=geteab();
                        setadc8(getr8(reg),temp);
                        setr8(reg,getr8(reg)+temp+tempc);
                        cycles-=((mod==3)?3:13);
                        break;
                        case 0x13: /*ADC reg,16*/
                        fetchea();
                        tempw=geteaw();
                        setadc16(regs[reg].w,tempw);
                        regs[reg].w+=tempw+tempc;
                        cycles-=((mod==3)?3:13);
                        break;
                        case 0x14: /*ADC AL,#8*/
                        tempw=FETCH();
                        setadc8(AL,tempw);
                        AL+=tempw+tempc;
                        cycles-=4;
                        break;
                        case 0x15: /*ADC AX,#16*/
                        tempw=getword();
                        setadc16(AX,tempw);
                        AX+=tempw+tempc;
                        cycles-=4;
                        break;

                        case 0x16: /*PUSH SS*/
                        if (ssegs) ss=oldss;
                        writememw(ss,((SP-2)&0xFFFF),SS);
                        SP-=2;
                        cycles-=14;
                        break;
                        case 0x17: /*POP SS*/
                        if (ssegs) ss=oldss;
                        tempw=readmemw(ss,SP);
                        loadseg(tempw,&_ss);
                        SP+=2;
                        noint=1;
                        cycles-=12;
//                        output=1;
                        break;

                        case 0x18: /*SBB 8,reg*/
                        fetchea();
                        temp=geteab();
                        temp2=getr8(reg);
                        setsbc8(temp,temp2);
                        temp-=(temp2+tempc);
                        seteab(temp);
                        cycles-=((mod==3)?3:24);
                        break;
                        case 0x19: /*SBB 16,reg*/
                        fetchea();
                        tempw=geteaw();
                        tempw2=regs[reg].w;
//                        printf("%04X:%04X SBB %04X-%04X,%i\n",cs>>4,pc,tempw,tempw2,tempc);
                        setsbc16(tempw,tempw2);
                        tempw-=(tempw2+tempc);
                        seteaw(tempw);
                        cycles-=((mod==3)?3:24);
                        break;
                        case 0x1A: /*SBB reg,8*/
                        fetchea();
                        temp=geteab();
                        setsbc8(getr8(reg),temp);
                        setr8(reg,getr8(reg)-(temp+tempc));
                        cycles-=((mod==3)?3:13);
                        break;
                        case 0x1B: /*SBB reg,16*/
                        fetchea();
                        tempw=geteaw();
                        tempw2=regs[reg].w;
//                        printf("%04X:%04X SBB %04X-%04X,%i\n",cs>>4,pc,tempw,tempw2,tempc);
                        setsbc16(tempw2,tempw);
                        tempw2-=(tempw+tempc);
                        regs[reg].w=tempw2;
                        cycles-=((mod==3)?3:13);
                        break;
                        case 0x1C: /*SBB AL,#8*/
                        temp=FETCH();
                        setsbc8(AL,temp);
                        AL-=(temp+tempc);
                        cycles-=4;
                        break;
                        case 0x1D: /*SBB AX,#16*/
                        tempw=getword();
                        setsbc16(AX,tempw);
                        AX-=(tempw+tempc);
                        cycles-=4;
                        break;

                        case 0x1E: /*PUSH DS*/
                        if (ssegs) ss=oldss;
                        writememw(ss,((SP-2)&0xFFFF),DS);
                        SP-=2;
                        cycles-=14;
                        break;
                        case 0x1F: /*POP DS*/
                        if (ssegs) ss=oldss;
                        tempw=readmemw(ss,SP);
                        loadseg(tempw,&_ds);
                        if (ssegs) oldds=ds;
                        SP+=2;
                        cycles-=12;
                        break;

                        case 0x20: /*AND 8,reg*/
                        fetchea();
                        temp=geteab();
                        temp&=getr8(reg);
                        setznp8(temp);
                        flags&=~(C_FLAG|V_FLAG|A_FLAG);
                        seteab(temp);
                        cycles-=((mod==3)?3:24);
                        break;
                        case 0x21: /*AND 16,reg*/
                        fetchea();
                        tempw=geteaw();
                        tempw&=regs[reg].w;
                        setznp16(tempw);
                        flags&=~(C_FLAG|V_FLAG|A_FLAG);
                        seteaw(tempw);
                        cycles-=((mod==3)?3:24);
                        break;
                        case 0x22: /*AND reg,8*/
                        fetchea();
                        temp=geteab();
                        temp&=getr8(reg);
                        setznp8(temp);
                        flags&=~(C_FLAG|V_FLAG|A_FLAG);
                        setr8(reg,temp);
                        cycles-=((mod==3)?3:13);
                        break;
                        case 0x23: /*AND reg,16*/
                        fetchea();
                        tempw=geteaw();
                        tempw&=regs[reg].w;
                        setznp16(tempw);
                        flags&=~(C_FLAG|V_FLAG|A_FLAG);
                        regs[reg].w=tempw;
                        cycles-=((mod==3)?3:13);
                        break;
                        case 0x24: /*AND AL,#8*/
                        AL&=FETCH();
                        setznp8(AL);
                        flags&=~(C_FLAG|V_FLAG|A_FLAG);
                        cycles-=4;
                        break;
                        case 0x25: /*AND AX,#16*/
                        AX&=getword();
                        setznp16(AX);
                        flags&=~(C_FLAG|V_FLAG|A_FLAG);
                        cycles-=4;
                        break;

                        case 0x26: /*ES:*/
                        oldss=ss;
                        oldds=ds;
                        ds=ss=es;
                        ssegs=2;
                        cycles-=4;
                        goto opcodestart;
//                        break;

                        case 0x27: /*DAA*/
                        if ((flags&A_FLAG) || ((AL&0xF)>9))
                        {
                                tempi=((uint16_t)AL)+6;
                                AL+=6;
                                flags|=A_FLAG;
                                if (tempi&0x100) flags|=C_FLAG;
                        }
//                        else
//                           flags&=~A_FLAG;
                        if ((flags&C_FLAG) || (AL>0x9F))
                        {
                                AL+=0x60;
                                flags|=C_FLAG;
                        }
//                        else
//                           flags&=~C_FLAG;
                        setznp8(AL);
                        cycles-=4;
                        break;

                        case 0x28: /*SUB 8,reg*/
                        fetchea();
                        temp=geteab();
                        setsub8(temp,getr8(reg));
                        temp-=getr8(reg);
                        seteab(temp);
                        cycles-=((mod==3)?3:24);
                        break;
                        case 0x29: /*SUB 16,reg*/
                        fetchea();
                        tempw=geteaw();
//                        printf("%04X:%04X  %04X-%04X\n",cs>>4,pc,tempw,regs[reg].w);
                        setsub16(tempw,regs[reg].w);
                        tempw-=regs[reg].w;
                        seteaw(tempw);
                        cycles-=((mod==3)?3:24);
                        break;
                        case 0x2A: /*SUB reg,8*/
                        fetchea();
                        temp=geteab();
                        setsub8(getr8(reg),temp);
                        setr8(reg,getr8(reg)-temp);
                        cycles-=((mod==3)?3:13);
                        break;
                        case 0x2B: /*SUB reg,16*/
                        fetchea();
                        tempw=geteaw();
//                        printf("%04X:%04X  %04X-%04X\n",cs>>4,pc,regs[reg].w,tempw);
                        setsub16(regs[reg].w,tempw);
                        regs[reg].w-=tempw;
                        cycles-=((mod==3)?3:13);
                        break;
                        case 0x2C: /*SUB AL,#8*/
                        temp=FETCH();
                        setsub8(AL,temp);
                        AL-=temp;
                        cycles-=4;
                        break;
                        case 0x2D: /*SUB AX,#16*/
//                        printf("INS %i\n",ins);
//                        output=1;
                        tempw=getword();
                        setsub16(AX,tempw);
                        AX-=tempw;
                        cycles-=4;
                        break;
                        case 0x2E: /*CS:*/
                        oldss=ss;
                        oldds=ds;
                        ds=ss=cs;
                        ssegs=2;
                        cycles-=4;
                        goto opcodestart;
                        case 0x2F: /*DAS*/
                        if ((flags&A_FLAG)||((AL&0xF)>9))
                        {
                                tempi=((uint16_t)AL)-6;
                                AL-=6;
                                flags|=A_FLAG;
                                if (tempi&0x100) flags|=C_FLAG;
                        }
//                        else
//                           flags&=~A_FLAG;
                        if ((flags&C_FLAG)||(AL>0x9F))
                        {
                                AL-=0x60;
                                flags|=C_FLAG;
                        }
//                        else
//                           flags&=~C_FLAG;
                        setznp8(AL);
                        cycles-=4;
                        break;
                        case 0x30: /*XOR 8,reg*/
                        fetchea();
                        temp=geteab();
                        temp^=getr8(reg);
                        setznp8(temp);
                        flags&=~(C_FLAG|V_FLAG|A_FLAG);
                        seteab(temp);
                        cycles-=((mod==3)?3:24);
                        break;
                        case 0x31: /*XOR 16,reg*/
                        fetchea();
                        tempw=geteaw();
                        tempw^=regs[reg].w;
                        setznp16(tempw);
                        flags&=~(C_FLAG|V_FLAG|A_FLAG);
                        seteaw(tempw);
                        cycles-=((mod==3)?3:24);
                        break;
                        case 0x32: /*XOR reg,8*/
                        fetchea();
                        temp=geteab();
                        temp^=getr8(reg);
                        setznp8(temp);
                        flags&=~(C_FLAG|V_FLAG|A_FLAG);
                        setr8(reg,temp);
                        cycles-=((mod==3)?3:13);
                        break;
                        case 0x33: /*XOR reg,16*/
                        fetchea();
                        tempw=geteaw();
                        tempw^=regs[reg].w;
                        setznp16(tempw);
                        flags&=~(C_FLAG|V_FLAG|A_FLAG);
                        regs[reg].w=tempw;
                        cycles-=((mod==3)?3:13);
                        break;
                        case 0x34: /*XOR AL,#8*/
                        AL^=FETCH();
                        setznp8(AL);
                        flags&=~(C_FLAG|V_FLAG|A_FLAG);
                        cycles-=4;
                        break;
                        case 0x35: /*XOR AX,#16*/
                        AX^=getword();
                        setznp16(AX);
                        flags&=~(C_FLAG|V_FLAG|A_FLAG);
                        cycles-=4;
                        break;

                        case 0x36: /*SS:*/
                        oldss=ss;
                        oldds=ds;
                        ds=ss=ss;
                        ssegs=2;
                        cycles-=4;
                        goto opcodestart;
//                        break;

                        case 0x37: /*AAA*/
                        if ((flags&A_FLAG)||((AL&0xF)>9))
                        {
                                AL+=6;
                                AH++;
                                flags|=(A_FLAG|C_FLAG);
                        }
                        else
                           flags&=~(A_FLAG|C_FLAG);
                        AL&=0xF;
                        cycles-=8;
                        break;

                        case 0x38: /*CMP 8,reg*/
                        fetchea();
                        temp=geteab();
//                        if (output) printf("CMP %02X-%02X\n",temp,getr8(reg));
                        setsub8(temp,getr8(reg));
                        cycles-=((mod==3)?3:13);
                        break;
                        case 0x39: /*CMP 16,reg*/
                        fetchea();
                        tempw=geteaw();
//                        if (output) printf("CMP %04X-%04X\n",tempw,regs[reg].w);
                        setsub16(tempw,regs[reg].w);
                        cycles-=((mod==3)?3:13);
                        break;
                        case 0x3A: /*CMP reg,8*/
                        fetchea();
                        temp=geteab();
//                        if (output) printf("CMP %02X-%02X\n",getr8(reg),temp);
                        setsub8(getr8(reg),temp);
                        cycles-=((mod==3)?3:13);
                        break;
                        case 0x3B: /*CMP reg,16*/
                        fetchea();
                        tempw=geteaw();
//                        printf("CMP %04X-%04X\n",regs[reg].w,tempw);
                        setsub16(regs[reg].w,tempw);
                        cycles-=((mod==3)?3:13);
                        break;
                        case 0x3C: /*CMP AL,#8*/
                        temp=FETCH();
                        setsub8(AL,temp);
                        cycles-=4;
                        break;
                        case 0x3D: /*CMP AX,#16*/
                        tempw=getword();
                        setsub16(AX,tempw);
                        cycles-=4;
                        break;

                        case 0x3E: /*DS:*/
                        oldss=ss;
                        oldds=ds;
                        ds=ss=ds;
                        ssegs=2;
                        cycles-=4;
                        goto opcodestart;
//                        break;

                        case 0x3F: /*AAS*/
                        if ((flags&A_FLAG)||((AL&0xF)>9))
                        {
                                AL-=6;
                                AH--;
                                flags|=(A_FLAG|C_FLAG);
                        }
                        else
                           flags&=~(A_FLAG|C_FLAG);
                        AL&=0xF;
                        cycles-=8;
                        break;

                        case 0x40: case 0x41: case 0x42: case 0x43: /*INC r16*/
                        case 0x44: case 0x45: case 0x46: case 0x47:
                        setadd16nc(regs[opcode&7].w,1);
                        regs[opcode&7].w++;
                        cycles-=3;
                        break;
                        case 0x48: case 0x49: case 0x4A: case 0x4B: /*DEC r16*/
                        case 0x4C: case 0x4D: case 0x4E: case 0x4F:
                        setsub16nc(regs[opcode&7].w,1);
                        regs[opcode&7].w--;
                        cycles-=3;
                        break;

                        case 0x50: case 0x51: case 0x52: case 0x53: /*PUSH r16*/
                        case 0x54: case 0x55: case 0x56: case 0x57:
                        if (ssegs) ss=oldss;
                        SP-=2;
                        writememw(ss,SP,regs[opcode&7].w);
                        cycles-=15;
                        break;
                        case 0x58: case 0x59: case 0x5A: case 0x5B: /*POP r16*/
                        case 0x5C: case 0x5D: case 0x5E: case 0x5F:
                        if (ssegs) ss=oldss;
                        SP+=2;
                        regs[opcode&7].w=readmemw(ss,(SP-2)&0xFFFF);
                        cycles-=12;
                        break;


                        case 0x70: /*JO*/
                        offset=(int8_t)FETCH();
                        if (flags&V_FLAG) { pc+=offset; cycles-=12; FETCHCLEAR(); }
                        cycles-=4;
                        break;
                        case 0x71: /*JNO*/
                        offset=(int8_t)FETCH();
                        if (!(flags&V_FLAG)) { pc+=offset; cycles-=12; FETCHCLEAR(); }
                        cycles-=4;
                        break;
                        case 0x72: /*JB*/
                        offset=(int8_t)FETCH();
                        if (flags&C_FLAG) { pc+=offset; cycles-=12; FETCHCLEAR(); }
                        cycles-=4;
                        break;
                        case 0x73: /*JNB*/
                        offset=(int8_t)FETCH();
                        if (!(flags&C_FLAG)) { pc+=offset; cycles-=12; FETCHCLEAR(); }
                        cycles-=4;
                        break;
                        case 0x74: /*JE*/
                        offset=(int8_t)FETCH();
                        if (flags&Z_FLAG) { pc+=offset; cycles-=12; FETCHCLEAR(); }
                        cycles-=4;
                        break;
                        case 0x75: /*JNE*/
                        offset=(int8_t)FETCH();
                        cycles-=4;
                        if (!(flags&Z_FLAG)) { pc+=offset; cycles-=12; FETCHCLEAR(); }
                        break;
                        case 0x76: /*JBE*/
                        offset=(int8_t)FETCH();
                        if (flags&(C_FLAG|Z_FLAG)) { pc+=offset; cycles-=12; FETCHCLEAR(); }
                        cycles-=4;
                        break;
                        case 0x77: /*JNBE*/
                        offset=(int8_t)FETCH();
                        if (!(flags&(C_FLAG|Z_FLAG))) { pc+=offset; cycles-=12; FETCHCLEAR(); }
                        cycles-=4;
                        break;
                        case 0x78: /*JS*/
                        offset=(int8_t)FETCH();
                        if (flags&N_FLAG)  { pc+=offset; cycles-=12; FETCHCLEAR(); }
                        cycles-=4;
                        break;
                        case 0x79: /*JNS*/
                        offset=(int8_t)FETCH();
                        if (!(flags&N_FLAG))  { pc+=offset; cycles-=12; FETCHCLEAR(); }
                        cycles-=4;
                        break;
                        case 0x7A: /*JP*/
                        offset=(int8_t)FETCH();
                        if (flags&P_FLAG)  { pc+=offset; cycles-=12; FETCHCLEAR(); }
                        cycles-=4;
                        break;
                        case 0x7B: /*JNP*/
                        offset=(int8_t)FETCH();
                        if (!(flags&P_FLAG))  { pc+=offset; cycles-=12; FETCHCLEAR(); }
                        cycles-=4;
                        break;
                        case 0x7C: /*JL*/
                        offset=(int8_t)FETCH();
                        temp=(flags&N_FLAG)?1:0;
                        temp2=(flags&V_FLAG)?1:0;
                        if (temp!=temp2)  { pc+=offset; cycles-=12; FETCHCLEAR(); }
                        cycles-=4;
                        break;
                        case 0x7D: /*JNL*/
                        offset=(int8_t)FETCH();
                        temp=(flags&N_FLAG)?1:0;
                        temp2=(flags&V_FLAG)?1:0;
                        if (temp==temp2)  { pc+=offset; cycles-=12; FETCHCLEAR(); }
                        cycles-=4;
                        break;
                        case 0x7E: /*JLE*/
                        offset=(int8_t)FETCH();
                        temp=(flags&N_FLAG)?1:0;
                        temp2=(flags&V_FLAG)?1:0;
                        if ((flags&Z_FLAG) || (temp!=temp2))  { pc+=offset; cycles-=12; FETCHCLEAR(); }
                        cycles-=4;
                        break;
                        case 0x7F: /*JNLE*/
                        offset=(int8_t)FETCH();
                        temp=(flags&N_FLAG)?1:0;
                        temp2=(flags&V_FLAG)?1:0;
                        if (!((flags&Z_FLAG) || (temp!=temp2)))  { pc+=offset; cycles-=12; FETCHCLEAR(); }
                        cycles-=4;
                        break;

                        case 0x80: case 0x82:
                        fetchea();
                        temp=geteab();
                        temp2=FETCH();
                        switch (rmdat&0x38)
                        {
                                case 0x00: /*ADD b,#8*/
                                setadd8(temp,temp2);
                                seteab(temp+temp2);
                                cycles-=((mod==3)?4:23);
                                break;
                                case 0x08: /*OR b,#8*/
                                temp|=temp2;
                                setznp8(temp);
                                flags&=~(C_FLAG|V_FLAG|A_FLAG);
                                seteab(temp);
                                cycles-=((mod==3)?4:23);
                                break;
                                case 0x10: /*ADC b,#8*/
//                                temp2+=(flags&C_FLAG);
                                setadc8(temp,temp2);
                                seteab(temp+temp2+tempc);
                                cycles-=((mod==3)?4:23);
                                break;
                                case 0x18: /*SBB b,#8*/
//                                temp2+=(flags&C_FLAG);
                                setsbc8(temp,temp2);
                                seteab(temp-(temp2+tempc));
                                cycles-=((mod==3)?4:23);
                                break;
                                case 0x20: /*AND b,#8*/
                                temp&=temp2;
                                setznp8(temp);
                                flags&=~(C_FLAG|V_FLAG|A_FLAG);
                                seteab(temp);
                                cycles-=((mod==3)?4:23);
                                break;
                                case 0x28: /*SUB b,#8*/
                                setsub8(temp,temp2);
                                seteab(temp-temp2);
                                cycles-=((mod==3)?4:23);
                                break;
                                case 0x30: /*XOR b,#8*/
                                temp^=temp2;
                                setznp8(temp);
                                flags&=~(C_FLAG|V_FLAG|A_FLAG);
                                seteab(temp);
                                cycles-=((mod==3)?4:23);
                                break;
                                case 0x38: /*CMP b,#8*/
                                setsub8(temp,temp2);
                                cycles-=((mod==3)?4:14);
                                break;

//                                default:
//                                printf("Bad 80 opcode %02X\n",rmdat&0x38);
//                                dumpregs();
//                                exit(-1);
                        }
                        break;

                        case 0x81:
                        fetchea();
                        tempw=geteaw();
                        tempw2=getword();
                        switch (rmdat&0x38)
                        {
                                case 0x00: /*ADD w,#16*/
                                setadd16(tempw,tempw2);
                                tempw+=tempw2;
                                seteaw(tempw);
                                cycles-=((mod==3)?4:23);
                                break;
                                case 0x08: /*OR w,#16*/
                                tempw|=tempw2;
                                setznp16(tempw);
                                flags&=~(C_FLAG|V_FLAG|A_FLAG);
                                seteaw(tempw);
                                cycles-=((mod==3)?4:23);
                                break;
                                case 0x10: /*ADC w,#16*/
//                                tempw2+=(flags&C_FLAG);
                                setadc16(tempw,tempw2);
                                tempw+=tempw2+tempc;
                                seteaw(tempw);
                                cycles-=((mod==3)?4:23);
                                break;
                                case 0x20: /*AND w,#16*/
                                tempw&=tempw2;
                                setznp16(tempw);
                                flags&=~(C_FLAG|V_FLAG|A_FLAG);
                                seteaw(tempw);
                                cycles-=((mod==3)?4:23);
                                break;
                                case 0x18: /*SBB w,#16*/
//                                tempw2+=(flags&C_FLAG);
                                setsbc16(tempw,tempw2);
                                seteaw(tempw-(tempw2+tempc));
                                cycles-=((mod==3)?4:23);
                                break;
                                case 0x28: /*SUB w,#16*/
                                setsub16(tempw,tempw2);
                                tempw-=tempw2;
                                seteaw(tempw);
                                cycles-=((mod==3)?4:23);
                                break;
                                case 0x30: /*XOR w,#16*/
                                tempw^=tempw2;
                                setznp16(tempw);
                                flags&=~(C_FLAG|V_FLAG|A_FLAG);
                                seteaw(tempw);
                                cycles-=((mod==3)?4:23);
                                break;
                                case 0x38: /*CMP w,#16*/
//                                printf("CMP %04X %04X\n",tempw,tempw2);
                                setsub16(tempw,tempw2);
                                cycles-=((mod==3)?4:14);
                                break;

//                                default:
//                                printf("Bad 81 opcode %02X\n",rmdat&0x38);
//                                dumpregs();
//                                exit(-1);
                        }
                        break;

                        case 0x83:
                        fetchea();
                        tempw=geteaw();
                        tempw2=FETCH();
                        if (tempw2&0x80) tempw2|=0xFF00;
                        switch (rmdat&0x38)
                        {
                                case 0x00: /*ADD w,#8*/
                                setadd16(tempw,tempw2);
                                tempw+=tempw2;
                                seteaw(tempw);
                                cycles-=((mod==3)?4:23);
                                break;
                                case 0x08: /*OR w,#8*/
                                tempw|=tempw2;
                                setznp16(tempw);
                                seteaw(tempw);
                                flags&=~(C_FLAG|A_FLAG|V_FLAG);
                                cycles-=((mod==3)?4:23);
                                break;
                                case 0x10: /*ADC w,#8*/
//                                tempw2+=(flags&C_FLAG);
                                setadc16(tempw,tempw2);
                                tempw+=tempw2+tempc;
                                seteaw(tempw);
                                cycles-=((mod==3)?4:23);
                                break;
                                case 0x18: /*SBB w,#8*/
//                                tempw2+=(flags&C_FLAG);
                                setsbc16(tempw,tempw2);
                                tempw-=(tempw2+tempc);
                                seteaw(tempw);
                                cycles-=((mod==3)?4:23);
                                break;
                                case 0x20: /*AND w,#8*/
                                tempw&=tempw2;
                                setznp16(tempw);
                                seteaw(tempw);
                                cycles-=((mod==3)?4:23);
                                flags&=~(C_FLAG|A_FLAG|V_FLAG);
                                break;
                                case 0x28: /*SUB w,#8*/
                                setsub16(tempw,tempw2);
                                tempw-=tempw2;
                                seteaw(tempw);
                                cycles-=((mod==3)?4:23);
                                break;
                                case 0x30: /*XOR w,#8*/
                                tempw^=tempw2;
                                setznp16(tempw);
                                seteaw(tempw);
                                cycles-=((mod==3)?4:23);
                                flags&=~(C_FLAG|A_FLAG|V_FLAG);
                                break;
                                case 0x38: /*CMP w,#8*/
                                setsub16(tempw,tempw2);
                                cycles-=((mod==3)?4:14);
                                break;

//                                default:
//                                printf("Bad 83 opcode %02X\n",rmdat&0x38);
//                                dumpregs();
//                                exit(-1);
                        }
                        break;

                        case 0x84: /*TEST b,reg*/
                        fetchea();
                        temp=geteab();
                        temp2=getr8(reg);
                        setznp8(temp&temp2);
                        flags&=~(C_FLAG|V_FLAG|A_FLAG);
                        cycles-=((mod==3)?3:13);
                        break;
                        case 0x85: /*TEST w,reg*/
                        fetchea();
                        tempw=geteaw();
                        tempw2=regs[reg].w;
                        setznp16(tempw&tempw2);
                        flags&=~(C_FLAG|V_FLAG|A_FLAG);
                        cycles-=((mod==3)?3:13);
                        break;
                        case 0x86: /*XCHG b,reg*/
                        fetchea();
                        temp=geteab();
                        seteab(getr8(reg));
                        setr8(reg,temp);
                        cycles-=((mod==3)?4:25);
                        break;
                        case 0x87: /*XCHG w,reg*/
                        fetchea();
                        tempw=geteaw();
                        seteaw(regs[reg].w);
                        regs[reg].w=tempw;
                        cycles-=((mod==3)?4:25);
                        break;

                        case 0x88: /*MOV b,reg*/
                        fetchea();
                        seteab(getr8(reg));
                        cycles-=((mod==3)?2:13);
                        break;
                        case 0x89: /*MOV w,reg*/
                        fetchea();
                        seteaw(regs[reg].w);
                        cycles-=((mod==3)?2:13);
                        break;
                        case 0x8A: /*MOV reg,b*/
                        fetchea();
                        temp=geteab();
                        setr8(reg,temp);
                        cycles-=((mod==3)?2:12);
                        break;
                        case 0x8B: /*MOV reg,w*/
                        fetchea();
                        tempw=geteaw();
                        regs[reg].w=tempw;
                        cycles-=((mod==3)?2:12);
                        break;

                        case 0x8C: /*MOV w,sreg*/
                        fetchea();
                        switch (rmdat&0x38)
                        {
                                case 0x00: /*ES*/
                                seteaw(ES);
                                break;
                                case 0x08: /*CS*/
                                seteaw(CS);
                                break;
                                case 0x18: /*DS*/
                                if (ssegs) ds=oldds;
                                seteaw(DS);
                                break;
                                case 0x10: /*SS*/
                                if (ssegs) ss=oldss;
                                seteaw(SS);
                                break;
                        }
                        cycles-=((mod==3)?2:13);
                        break;

                        case 0x8D: /*LEA*/
                        fetchea();
                        regs[reg].w=eaaddr;
                        cycles-=2;
                        break;

                        case 0x8E: /*MOV sreg,w*/
//                        if (output) printf("MOV %04X  ",pc);
                        fetchea();
//                        if (output) printf("%04X %02X\n",pc,rmdat);
                        switch (rmdat&0x38)
                        {
                                case 0x00: /*ES*/
                                tempw=geteaw();
                                loadseg(tempw,&_es);
                                break;
                                case 0x08: /*CS - 8088/8086 only*/
                                tempw=geteaw();
                                loadseg(tempw,&_cs);
                                break;
                                case 0x18: /*DS*/
                                tempw=geteaw();
                                loadseg(tempw,&_ds);
                                if (ssegs) oldds=ds;
                                break;
                                case 0x10: /*SS*/
                                tempw=geteaw();
                                loadseg(tempw,&_ss);
                                if (ssegs) oldss=ss;
//                                printf("LOAD SS %04X %04X\n",tempw,SS);
//				printf("SS loaded with %04X %04X:%04X %04X %04X %04X\n",ss>>4,cs>>4,pc,CX,DX,es>>4);
                                break;
                        }
                        cycles-=((mod==3)?2:12);
                                skipnextprint=1;
				noint=1;
                        break;

                        case 0x8F: /*POPW*/
                        fetchea();
                        if (ssegs) ss=oldss;
                        tempw=readmemw(ss,SP);
                        SP+=2;
                        seteaw(tempw);
                        cycles-=25;
                        break;

                        case 0x90: /*NOP*/
                        cycles-=3;
                        break;

                        case 0x91: case 0x92: case 0x93: /*XCHG AX*/
                        case 0x94: case 0x95: case 0x96: case 0x97:
                        tempw=AX;
                        AX=regs[opcode&7].w;
                        regs[opcode&7].w=tempw;
                        cycles-=3;
                        break;

                        case 0x98: /*CBW*/
                        AH=(AL&0x80)?0xFF:0;
                        cycles-=2;
                        break;
                        case 0x99: /*CWD*/
                        DX=(AX&0x8000)?0xFFFF:0;
                        cycles-=5;
                        break;
                        case 0x9A: /*CALL FAR*/
                        tempw=getword();
                        tempw2=getword();
                        tempw3=CS;
                        tempw4=pc;
                        if (ssegs) ss=oldss;
                        pc=tempw;
//                        printf("0x9a");
                        loadcs(tempw2);
                        writememw(ss,(SP-2)&0xFFFF,tempw3);
                        writememw(ss,(SP-4)&0xFFFF,tempw4);
                        SP-=4;
                        cycles-=36;
                        FETCHCLEAR();
                        break;
                        case 0x9B: /*WAIT*/
                        cycles-=4;
                        break;
                        case 0x9C: /*PUSHF*/
                        if (ssegs) ss=oldss;
                        writememw(ss,((SP-2)&0xFFFF),flags|0xF000);
                        SP-=2;
                        cycles-=14;
                        break;
                        case 0x9D: /*POPF*/
                        if (ssegs) ss=oldss;
                        flags=readmemw(ss,SP)&0xFFF;
                        SP+=2;
                        cycles-=12;
                        break;
                        case 0x9E: /*SAHF*/
                        flags=(flags&0xFF00)|AH;
                        cycles-=4;
                        break;
                        case 0x9F: /*LAHF*/
                        AH=flags&0xFF;
                        cycles-=4;
                        break;

                        case 0xA0: /*MOV AL,(w)*/
                        addr=getword();
                        AL=readmemb(ds+addr);
                        cycles-=14;
                        break;
                        case 0xA1: /*MOV AX,(w)*/
                        addr=getword();
//                        printf("Reading AX from %05X %04X:%04X\n",ds+addr,ds>>4,addr);
                        AX=readmemw(ds,addr);
                        cycles-=!4;
                        break;
                        case 0xA2: /*MOV (w),AL*/
                        addr=getword();
                        writememb(ds+addr,AL);
                        cycles-=14;
                        break;
                        case 0xA3: /*MOV (w),AX*/
                        addr=getword();
//                        if (!addr) printf("Write !addr %04X:%04X\n",cs>>4,pc);
                        writememw(ds,addr,AX);
                        cycles-=14;
                        break;

                        case 0xA4: /*MOVSB*/
                        temp=readmemb(ds+SI);
                        writememb(es+DI,temp);
                        if (flags&D_FLAG) { DI--; SI--; }
                        else              { DI++; SI++; }
                        cycles-=18;
                        break;
                        case 0xA5: /*MOVSW*/
                        tempw=readmemw(ds,SI);
                        writememw(es,DI,tempw);
                        if (flags&D_FLAG) { DI-=2; SI-=2; }
                        else              { DI+=2; SI+=2; }
                        cycles-=18;
                        break;
                        case 0xA6: /*CMPSB*/
                        temp =readmemb(ds+SI);
                        temp2=readmemb(es+DI);
                        setsub8(temp,temp2);
                        if (flags&D_FLAG) { DI--; SI--; }
                        else              { DI++; SI++; }
                        cycles-=30;
                        break;
                        case 0xA7: /*CMPSW*/
                        tempw =readmemw(ds,SI);
                        tempw2=readmemw(es,DI);
//                        printf("CMPSW %04X %04X\n",tempw,tempw2);
                        setsub16(tempw,tempw2);
                        if (flags&D_FLAG) { DI-=2; SI-=2; }
                        else              { DI+=2; SI+=2; }
                        cycles-=30;
                        break;
                        case 0xA8: /*TEST AL,#8*/
                        temp=FETCH();
                        setznp8(AL&temp);
                        flags&=~(C_FLAG|V_FLAG|A_FLAG);
                        cycles-=5;
                        break;
                        case 0xA9: /*TEST AX,#16*/
                        tempw=getword();
                        setznp16(AX&tempw);
                        flags&=~(C_FLAG|V_FLAG|A_FLAG);
                        cycles-=5;
                        break;
                        case 0xAA: /*STOSB*/
                        writememb(es+DI,AL);
                        if (flags&D_FLAG) DI--;
                        else              DI++;
                        cycles-=11;
                        break;
                        case 0xAB: /*STOSW*/
                        writememw(es,DI,AX);
                        if (flags&D_FLAG) DI-=2;
                        else              DI+=2;
                        cycles-=11;
                        break;
                        case 0xAC: /*LODSB*/
                        AL=readmemb(ds+SI);
//                        printf("LODSB %04X:%04X %02X %04X:%04X\n",cs>>4,pc,AL,ds>>4,SI);
                        if (flags&D_FLAG) SI--;
                        else              SI++;
                        cycles-=16;
                        break;
                        case 0xAD: /*LODSW*/
//                        if (times) printf("LODSW %04X:%04X\n",cs>>4,pc);
                        AX=readmemw(ds,SI);
                        if (flags&D_FLAG) SI-=2;
                        else              SI+=2;
                        cycles-=16;
                        break;
                        case 0xAE: /*SCASB*/
                        temp=readmemb(es+DI);
                        setsub8(AL,temp);
                        if (flags&D_FLAG) DI--;
                        else              DI++;
                        cycles-=19;
                        break;
                        case 0xAF: /*SCASW*/
                        tempw=readmemw(es,DI);
                        setsub16(AX,tempw);
                        if (flags&D_FLAG) DI-=2;
                        else              DI+=2;
                        cycles-=19;
                        break;

                        case 0xB0: /*MOV AL,#8*/
                        AL=FETCH();
                        cycles-=4;
                        break;
                        case 0xB1: /*MOV CL,#8*/
                        CL=FETCH();
                        cycles-=4;
                        break;
                        case 0xB2: /*MOV DL,#8*/
                        DL=FETCH();
                        cycles-=4;
                        break;
                        case 0xB3: /*MOV BL,#8*/
                        BL=FETCH();
                        cycles-=4;
                        break;
                        case 0xB4: /*MOV AH,#8*/
                        AH=FETCH();
                        cycles-=4;
                        break;
                        case 0xB5: /*MOV CH,#8*/
                        CH=FETCH();
                        cycles-=4;
                        break;
                        case 0xB6: /*MOV DH,#8*/
                        DH=FETCH();
                        cycles-=4;
                        break;
                        case 0xB7: /*MOV BH,#8*/
                        BH=FETCH();
                        cycles-=4;
                        break;
                        case 0xB8: case 0xB9: case 0xBA: case 0xBB: /*MOV reg,#16*/
                        case 0xBC: case 0xBD: case 0xBE: case 0xBF:
                        regs[opcode&7].w=getword();
                        cycles-=4;
                        break;

                        case 0xC2: /*RET*/
                        tempw=getword();
                        if (ssegs) ss=oldss;
                        pc=readmemw(ss,SP);
//                        printf("C2\n");
//                        printf("RET to %04X\n",pc);
                        SP+=2+tempw;
                        cycles-=24;
                        FETCHCLEAR();
                        break;
                        case 0xC3: /*RET*/
                        if (ssegs) ss=oldss;
                        pc=readmemw(ss,SP);
//                        printf("C3\n");
//                        if (output) printf("RET to %04X %05X\n",pc,ss+SP);
                        SP+=2;
                        cycles-=20;
                        FETCHCLEAR();
                        break;
                        case 0xC4: /*LES*/
                        fetchea();
                        regs[reg].w=readmemw(easeg,eaaddr); //geteaw();
                        tempw=readmemw(easeg,(eaaddr+2)&0xFFFF); //geteaw2();
                        loadseg(tempw,&_es);
                        cycles-=24;
                        break;
                        case 0xC5: /*LDS*/
                        fetchea();
                        regs[reg].w=readmemw(easeg,eaaddr);
                        tempw=readmemw(easeg,(eaaddr+2)&0xFFFF);
                        loadseg(tempw,&_ds);
                        if (ssegs) oldds=ds;
                        cycles-=24;
                        break;
                        case 0xC6: /*MOV b,#8*/
                        fetchea();
                        temp=FETCH();
                        seteab(temp);
                        cycles-=((mod==3)?4:14);
                        break;
                        case 0xC7: /*MOV w,#16*/
                        fetchea();
                        tempw=getword();
                        seteaw(tempw);
                        cycles-=((mod==3)?4:14);
                        break;

                        case 0xCA: /*RETF*/
                        tempw=getword();
                        if (ssegs) ss=oldss;
                        pc=readmemw(ss,SP);
//                        printf("CA\n");
                        loadcs(readmemw(ss,SP+2));
                        SP+=4;
                        SP+=tempw;
                        cycles-=33;
                        FETCHCLEAR();
                        break;
                        case 0xCB: /*RETF*/
                        if (ssegs) ss=oldss;
                        pc=readmemw(ss,SP);
//                        printf("CB\n");
                        loadcs(readmemw(ss,SP+2));
                        SP+=4;
                        cycles-=34;
                        FETCHCLEAR();
                        break;
                        case 0xCC: /*INT 3*/
                        if (ssegs) ss=oldss;
                        writememw(ss,((SP-2)&0xFFFF),flags|0xF000);
                        writememw(ss,((SP-4)&0xFFFF),CS);
                        writememw(ss,((SP-6)&0xFFFF),pc);
                        SP-=6;
                        addr=3<<2;
                        flags&=~I_FLAG;
                        flags&=~T_FLAG;
//                        printf("CC %04X:%04X  ",CS,pc);
                        pc=readmemw(0,addr);
                        loadcs(readmemw(0,addr+2));
                        FETCHCLEAR();
//                        printf("%04X:%04X\n",CS,pc);
                        cycles-=72;
                        break;
                        case 0xCD: /*INT*/
                        lastpc=pc;
                        lastcs=CS;
                        temp=FETCH();

                        if (ssegs) ss=oldss;
                        writememw(ss,((SP-2)&0xFFFF),flags|0xF000);
                        writememw(ss,((SP-4)&0xFFFF),CS);
                        writememw(ss,((SP-6)&0xFFFF),pc);
                        flags&=~T_FLAG;
                        SP-=6;
                        addr=temp<<2;
                        pc=readmemw(0,addr);

                        loadcs(readmemw(0,addr+2));
                        FETCHCLEAR();

                        cycles-=71;
                        break;
                        case 0xCF: /*IRET*/
                        if (ssegs) ss=oldss;
                        tempw=CS;
                        tempw2=pc;
                        pc=readmemw(ss,SP);
//                        printf("CF\n");
                        loadcs(readmemw(ss,((SP+2)&0xFFFF)));
                        flags=readmemw(ss,((SP+4)&0xFFFF))&0xFFF;
                        SP+=6;
                        cycles-=44;
                        FETCHCLEAR();
                        nmi_enable = 1;
                        break;
                        case 0xD0:
                        fetchea();
                        temp=geteab();
                        switch (rmdat&0x38)
                        {
                                case 0x00: /*ROL b,1*/
                                if (temp&0x80) flags|=C_FLAG;
                                else           flags&=~C_FLAG;
                                temp<<=1;
                                if (flags&C_FLAG) temp|=1;
                                seteab(temp);
//                                setznp8(temp);
                                if ((flags&C_FLAG)^(temp>>7)) flags|=V_FLAG;
                                else                          flags&=~V_FLAG;
                                cycles-=((mod==3)?2:23);
                                break;
                                case 0x08: /*ROR b,1*/
                                if (temp&1) flags|=C_FLAG;
                                else        flags&=~C_FLAG;
                                temp>>=1;
                                if (flags&C_FLAG) temp|=0x80;
                                seteab(temp);
//                                setznp8(temp);
                                if ((temp^(temp>>1))&0x40) flags|=V_FLAG;
                                else                       flags&=~V_FLAG;
                                cycles-=((mod==3)?2:23);
                                break;
                                case 0x10: /*RCL b,1*/
                                temp2=flags&C_FLAG;
                                if (temp&0x80) flags|=C_FLAG;
                                else           flags&=~C_FLAG;
                                temp<<=1;
                                if (temp2) temp|=1;
                                seteab(temp);
//                                setznp8(temp);
                                if ((flags&C_FLAG)^(temp>>7)) flags|=V_FLAG;
                                else                          flags&=~V_FLAG;
                                cycles-=((mod==3)?2:23);
                                break;
                                case 0x18: /*RCR b,1*/
                                temp2=flags&C_FLAG;
                                if (temp&1) flags|=C_FLAG;
                                else        flags&=~C_FLAG;
                                temp>>=1;
                                if (temp2) temp|=0x80;
                                seteab(temp);
//                                setznp8(temp);
                                if ((temp^(temp>>1))&0x40) flags|=V_FLAG;
                                else                       flags&=~V_FLAG;
                                cycles-=((mod==3)?2:23);
                                break;
                                case 0x20: case 0x30: /*SHL b,1*/
                                if (temp&0x80) flags|=C_FLAG;
                                else           flags&=~C_FLAG;
                                if ((temp^(temp<<1))&0x80) flags|=V_FLAG;
                                else                       flags&=~V_FLAG;
                                temp<<=1;
                                seteab(temp);
                                setznp8(temp);
                                cycles-=((mod==3)?2:23);
                                flags|=A_FLAG;
                                break;
                                case 0x28: /*SHR b,1*/
                                if (temp&1) flags|=C_FLAG;
                                else        flags&=~C_FLAG;
                                if (temp&0x80) flags|=V_FLAG;
                                else           flags&=~V_FLAG;
                                temp>>=1;
                                seteab(temp);
                                setznp8(temp);
                                cycles-=((mod==3)?2:23);
                                flags|=A_FLAG;
                                break;
                                case 0x38: /*SAR b,1*/
                                if (temp&1) flags|=C_FLAG;
                                else        flags&=~C_FLAG;
                                temp>>=1;
                                if (temp&0x40) temp|=0x80;
                                seteab(temp);
                                setznp8(temp);
                                cycles-=((mod==3)?2:23);
                                flags|=A_FLAG;
                                flags&=~V_FLAG;
                                break;

//                                default:
//                                printf("Bad D0 opcode %02X\n",rmdat&0x38);
//                                dumpregs();
//                                exit(-1);
                        }
                        break;

                        case 0xD1:
                        fetchea();
                        tempw=geteaw();
                        switch (rmdat&0x38)
                        {
                                case 0x00: /*ROL w,1*/
                                if (tempw&0x8000) flags|=C_FLAG;
                                else              flags&=~C_FLAG;
                                tempw<<=1;
                                if (flags&C_FLAG) tempw|=1;
                                seteaw(tempw);
//                                setznp16(tempw);
                                if ((flags&C_FLAG)^(tempw>>15)) flags|=V_FLAG;
                                else                            flags&=~V_FLAG;
                                cycles-=((mod==3)?2:23);
                                break;
                                case 0x08: /*ROR w,1*/
                                if (tempw&1) flags|=C_FLAG;
                                else         flags&=~C_FLAG;
                                tempw>>=1;
                                if (flags&C_FLAG) tempw|=0x8000;
                                seteaw(tempw);
//                                setznp16(tempw);
                                if ((tempw^(tempw>>1))&0x4000) flags|=V_FLAG;
                                else                           flags&=~V_FLAG;
                                cycles-=((mod==3)?2:23);
                                break;
                                case 0x10: /*RCL w,1*/
                                temp2=flags&C_FLAG;
                                if (tempw&0x8000) flags|=C_FLAG;
                                else              flags&=~C_FLAG;
                                tempw<<=1;
                                if (temp2) tempw|=1;
                                seteaw(tempw);
                                if ((flags&C_FLAG)^(tempw>>15)) flags|=V_FLAG;
                                else                            flags&=~V_FLAG;
                                cycles-=((mod==3)?2:23);
                                break;
                                case 0x18: /*RCR w,1*/
                                temp2=flags&C_FLAG;
                                if (tempw&1) flags|=C_FLAG;
                                else         flags&=~C_FLAG;
                                tempw>>=1;
                                if (temp2) tempw|=0x8000;
                                seteaw(tempw);
//                                setznp16(tempw);
                                if ((tempw^(tempw>>1))&0x4000) flags|=V_FLAG;
                                else                           flags&=~V_FLAG;
                                cycles-=((mod==3)?2:23);
                                break;
                                case 0x20: case 0x30: /*SHL w,1*/
                                if (tempw&0x8000) flags|=C_FLAG;
                                else              flags&=~C_FLAG;
                                if ((tempw^(tempw<<1))&0x8000) flags|=V_FLAG;
                                else                           flags&=~V_FLAG;
                                tempw<<=1;
                                seteaw(tempw);
                                setznp16(tempw);
                                cycles-=((mod==3)?2:23);
                                flags|=A_FLAG;
                                break;
                                case 0x28: /*SHR w,1*/
                                if (tempw&1) flags|=C_FLAG;
                                else         flags&=~C_FLAG;
                                if (tempw&0x8000) flags|=V_FLAG;
                                else              flags&=~V_FLAG;
                                tempw>>=1;
                                seteaw(tempw);
                                setznp16(tempw);
                                cycles-=((mod==3)?2:23);
                                flags|=A_FLAG;
                                break;

                                case 0x38: /*SAR w,1*/
                                if (tempw&1) flags|=C_FLAG;
                                else         flags&=~C_FLAG;
                                tempw>>=1;
                                if (tempw&0x4000) tempw|=0x8000;
                                seteaw(tempw);
                                setznp16(tempw);
                                cycles-=((mod==3)?2:23);
                                flags|=A_FLAG;
                                flags&=~V_FLAG;
                                break;

//                                default:
//                                printf("Bad D1 opcode %02X\n",rmdat&0x38);
//                                dumpregs();
//                                exit(-1);
                        }
                        break;

                        case 0xD2:
                        fetchea();
                        temp=geteab();
                        c=CL;
//                        cycles-=c;
                        if (!c) break;
//                        if (c>7) printf("Shiftb %i %02X\n",rmdat&0x38,c);
                        switch (rmdat&0x38)
                        {
                                case 0x00: /*ROL b,CL*/
                                while (c>0)
                                {
                                        temp2=(temp&0x80)?1:0;
                                        temp=(temp<<1)|temp2;
                                        c--;
                                        cycles-=4;
                                }
                                if (temp2) flags|=C_FLAG;
                                else       flags&=~C_FLAG;
                                seteab(temp);
//                                setznp8(temp);
                                if ((flags&C_FLAG)^(temp>>7)) flags|=V_FLAG;
                                else                          flags&=~V_FLAG;
                                cycles-=((mod==3)?8:28);
                                break;
                                case 0x08: /*ROR b,CL*/
                                while (c>0)
                                {
                                        temp2=temp&1;
                                        temp>>=1;
                                        if (temp2) temp|=0x80;
                                        c--;
                                        cycles-=4;
                                }
                                if (temp2) flags|=C_FLAG;
                                else       flags&=~C_FLAG;
                                seteab(temp);
                                if ((temp^(temp>>1))&0x40) flags|=V_FLAG;
                                else                       flags&=~V_FLAG;
                                cycles-=((mod==3)?8:28);
                                break;
                                case 0x10: /*RCL b,CL*/
//                                printf("RCL %i %02X %02X\n",c,CL,temp);
                                while (c>0)
                                {
                                        templ=flags&C_FLAG;
                                        temp2=temp&0x80;
                                        temp<<=1;
                                        if (temp2) flags|=C_FLAG;
                                        else       flags&=~C_FLAG;
                                        if (templ) temp|=1;
                                        c--;
                                        cycles-=4;
                                }
//                                printf("Now %02X\n",temp);
                                seteab(temp);
                                if ((flags&C_FLAG)^(temp>>7)) flags|=V_FLAG;
                                else                          flags&=~V_FLAG;
                                cycles-=((mod==3)?8:28);
                                break;
                                case 0x18: /*RCR b,CL*/
                                while (c>0)
                                {
                                        templ=flags&C_FLAG;
                                        temp2=temp&1;
                                        temp>>=1;
                                        if (temp2) flags|=C_FLAG;
                                        else       flags&=~C_FLAG;
                                        if (templ) temp|=0x80;
                                        c--;
                                        cycles-=4;
                                }
//                                if (temp2) flags|=C_FLAG;
//                                else       flags&=~C_FLAG;
                                seteab(temp);
                                if ((temp^(temp>>1))&0x40) flags|=V_FLAG;
                                else                       flags&=~V_FLAG;
                                cycles-=((mod==3)?8:28);
                                break;
                                case 0x20: case 0x30: /*SHL b,CL*/
                                if ((temp<<(c-1))&0x80) flags|=C_FLAG;
                                else                    flags&=~C_FLAG;
                                temp<<=c;
                                seteab(temp);
                                setznp8(temp);
                                cycles-=(c*4);
                                cycles-=((mod==3)?8:28);
                                flags|=A_FLAG;
                                break;
                                case 0x28: /*SHR b,CL*/
                                if ((temp>>(c-1))&1) flags|=C_FLAG;
                                else                 flags&=~C_FLAG;
                                temp>>=c;
                                seteab(temp);
                                setznp8(temp);
                                cycles-=(c*4);
                                cycles-=((mod==3)?8:28);
                                flags|=A_FLAG;
                                break;
                                case 0x38: /*SAR b,CL*/
                                if ((temp>>(c-1))&1) flags|=C_FLAG;
                                else                 flags&=~C_FLAG;
                                while (c>0)
                                {
                                        temp>>=1;
                                        if (temp&0x40) temp|=0x80;
                                        c--;
                                        cycles-=4;
                                }
                                seteab(temp);
                                setznp8(temp);
                                cycles-=((mod==3)?8:28);
                                flags|=A_FLAG;
                                break;

//                                default:
//                                printf("Bad D2 opcode %02X\n",rmdat&0x38);
//                                dumpregs();
//                                exit(-1);
                        }
                        break;

                        case 0xD3:
                        fetchea();
                        tempw=geteaw();
                        c=CL;
//                      cycles-=c;
                        if (!c) break;
//                        if (c>15) printf("Shiftw %i %02X\n",rmdat&0x38,c);
                        switch (rmdat&0x38)
                        {
                                case 0x00: /*ROL w,CL*/
                                while (c>0)
                                {
                                        temp=(tempw&0x8000)?1:0;
                                        tempw=(tempw<<1)|temp;
                                        c--;
                                        cycles-=4;
                                }
                                if (temp) flags|=C_FLAG;
                                else      flags&=~C_FLAG;
                                seteaw(tempw);
                                if ((flags&C_FLAG)^(tempw>>15)) flags|=V_FLAG;
                                else                            flags&=~V_FLAG;
                                cycles-=((mod==3)?8:28);
                                break;
                                case 0x08: /*ROR w,CL*/
                                while (c>0)
                                {
                                        tempw2=(tempw&1)?0x8000:0;
                                        tempw=(tempw>>1)|tempw2;
                                        c--;
                                        cycles-=4;
                                }
                                if (tempw2) flags|=C_FLAG;
                                else        flags&=~C_FLAG;
                                seteaw(tempw);
                                if ((tempw^(tempw>>1))&0x4000) flags|=V_FLAG;
                                else                           flags&=~V_FLAG;
                                cycles-=((mod==3)?8:28);
                                break;
                                case 0x10: /*RCL w,CL*/
                                while (c>0)
                                {
                                        templ=flags&C_FLAG;
                                        if (tempw&0x8000) flags|=C_FLAG;
                                        else              flags&=~C_FLAG;
                                        tempw=(tempw<<1)|templ;
                                        c--;
                                        cycles-=4;
                                }
                                if (temp) flags|=C_FLAG;
                                else      flags&=~C_FLAG;
                                seteaw(tempw);
                                if ((flags&C_FLAG)^(tempw>>15)) flags|=V_FLAG;
                                else                            flags&=~V_FLAG;
                                cycles-=((mod==3)?8:28);
                                break;
                                case 0x18: /*RCR w,CL*/
                                while (c>0)
                                {
                                        templ=flags&C_FLAG;
                                        tempw2=(templ&1)?0x8000:0;
                                        if (tempw&1) flags|=C_FLAG;
                                        else         flags&=~C_FLAG;
                                        tempw=(tempw>>1)|tempw2;
                                        c--;
                                        cycles-=4;
                                }
                                if (tempw2) flags|=C_FLAG;
                                else        flags&=~C_FLAG;
                                seteaw(tempw);
                                if ((tempw^(tempw>>1))&0x4000) flags|=V_FLAG;
                                else                           flags&=~V_FLAG;
                                cycles-=((mod==3)?8:28);
                                break;

                                case 0x20: case 0x30: /*SHL w,CL*/
                                if (c>16)
                                {
                                        tempw=0;
                                        flags&=~C_FLAG;
                                }
                                else
                                {
                                        if ((tempw<<(c-1))&0x8000) flags|=C_FLAG;
                                        else                       flags&=~C_FLAG;
                                        tempw<<=c;
                                }
                                seteaw(tempw);
                                setznp16(tempw);
                                cycles-=(c*4);
                                cycles-=((mod==3)?8:28);
                                flags|=A_FLAG;
                                break;

                                case 0x28:            /*SHR w,CL*/
                                if ((tempw>>(c-1))&1) flags|=C_FLAG;
                                else                  flags&=~C_FLAG;
                                tempw>>=c;
                                seteaw(tempw);
                                setznp16(tempw);
                                cycles-=(c*4);
                                cycles-=((mod==3)?8:28);
                                flags|=A_FLAG;
                                break;

                                case 0x38:            /*SAR w,CL*/
                                tempw2=tempw&0x8000;
                                if ((tempw>>(c-1))&1) flags|=C_FLAG;
                                else                  flags&=~C_FLAG;
                                while (c>0)
                                {
                                        tempw=(tempw>>1)|tempw2;
                                        c--;
                                        cycles-=4;
                                }
                                seteaw(tempw);
                                setznp16(tempw);
                                cycles-=((mod==3)?8:28);
                                flags|=A_FLAG;
                                break;

//                                default:
//                                printf("Bad D3 opcode %02X\n",rmdat&0x38);
//                                dumpregs();
//                                exit(-1);
                        }
                        break;

                        case 0xD4: /*AAM*/
                        tempws=FETCH();
                        AH=AL/tempws;
                        AL%=tempws;
                        setznp16(AX);
                        cycles-=83;
                        break;
                        case 0xD5: /*AAD*/
                        tempws=FETCH();
                        AL=(AH*tempws)+AL;
                        AH=0;
                        setznp16(AX);
                        cycles-=60;
                        break;
                        case 0xD7: /*XLAT*/
                        addr=BX+AL;
                        AL=readmemb(ds+addr);
                        cycles-=11;
                        break;
                        case 0xD9: case 0xDA: case 0xDB: case 0xDD: /*ESCAPE*/
                        case 0xDC: case 0xDE: case 0xDF: case 0xD8:
                        fetchea();
                        geteab();
                        break;

                        case 0xE0: /*LOOPNE*/
                        offset=(int8_t)FETCH();
                        CX--;
                        if (CX && !(flags&Z_FLAG)) { pc+=offset; cycles-=12; FETCHCLEAR(); }
                        cycles-=6;
                        break;
                        case 0xE1: /*LOOPE*/
                        offset=(int8_t)FETCH();
                        CX--;
                        if (CX && (flags&Z_FLAG)) { pc+=offset; cycles-=12; FETCHCLEAR(); }
                        cycles-=6;
                        break;
                        case 0xE2: /*LOOP*/
//                        printf("LOOP start\n");
                        offset=(int8_t)FETCH();
                        CX--;
                        if (CX) { pc+=offset; cycles-=12; FETCHCLEAR(); }
                        cycles-=5;
//                        printf("LOOP end!\n");
                        break;
                        case 0xE3: /*JCXZ*/
                        offset=(int8_t)FETCH();
                        if (!CX) { pc+=offset; cycles-=12; FETCHCLEAR(); }
                        cycles-=6;
                        break;

                        case 0xE4: /*IN AL*/
                        temp=FETCH();
                        AL=inb(temp);
                        cycles-=14;
                        break;
                        case 0xE5: /*IN AX*/
                        temp=FETCH();
                        AL=inb(temp);
                        AH=inb(temp+1);
                        cycles-=14;
                        break;
                        case 0xE6: /*OUT AL*/
                        temp=FETCH();
                        outb(temp,AL);
                        cycles-=14;
                        break;
                        case 0xE7: /*OUT AX*/
                        temp=FETCH();
                        outb(temp,AL);
                        outb(temp+1,AH);
                        cycles-=14;
                        break;

                        case 0xE8: /*CALL rel 16*/
                        tempw=getword();
                        if (ssegs) ss=oldss;
//                        writememb(ss+((SP-1)&0xFFFF),pc>>8);
                        writememw(ss,((SP-2)&0xFFFF),pc);
                        SP-=2;
                        pc+=tempw;
                        cycles-=23;
                        FETCHCLEAR();
                        break;
                        case 0xE9: /*JMP rel 16*/
//                        printf("PC was %04X\n",pc);
                        pc+=getword();
//                        printf("PC now %04X\n",pc);
                        cycles-=15;
                        FETCHCLEAR();
                        break;
                        case 0xEA: /*JMP far*/
                        addr=getword();
                        tempw=getword();
                        pc=addr;
//                        printf("EA\n");
                        loadcs(tempw);
//                        cs=loadcs(CS);
//                        cs=CS<<4;
                        cycles-=15;
                        FETCHCLEAR();
                        break;
                        case 0xEB: /*JMP rel*/
                        offset=(int8_t)FETCH();
                        pc+=offset;
                        cycles-=15;
                        FETCHCLEAR();
                        break;
                        case 0xEC: /*IN AL,DX*/
                        AL=inb(DX);
                        cycles-=12;
                        break;
                        case 0xED: /*IN AX,DX*/
                        AL=inb(DX);
                        AH=inb(DX+1);
                        cycles-=12;
                        break;
                        case 0xEE: /*OUT DX,AL*/
                        outb(DX,AL);
                        cycles-=12;
                        break;
                        case 0xEF: /*OUT DX,AX*/
                        outb(DX,AL);
                        outb(DX+1,AH);
                        cycles-=12;
                        break;

                        case 0xF0: /*LOCK*/
                        cycles-=4;
                        break;

                        case 0xF2: /*REPNE*/
                        rep(0);
                        break;
                        case 0xF3: /*REPE*/
                        rep(1);
                        break;

                        case 0xF4: /*HLT*/
//                        printf("IN HLT!!!! %04X:%04X %08X %08X %08X\n",oldcs,oldpc,old8,old82,old83);
/*                        if (!(flags & I_FLAG))
                        {
                                pclog("HLT\n");
                                dumpregs();
                                exit(-1);
                        }*/
                        inhlt=1;
                        pc--;
                        FETCHCLEAR();
                        cycles-=2;
                        break;
                        case 0xF5: /*CMC*/
                        flags^=C_FLAG;
                        cycles-=2;
                        break;

                        case 0xF6:
                        fetchea();
                        temp=geteab();
                        switch (rmdat&0x38)
                        {
                                case 0x00: /*TEST b,#8*/
                                temp2=FETCH();
                                temp&=temp2;
                                setznp8(temp);
                                flags&=~(C_FLAG|V_FLAG|A_FLAG);
                                cycles-=((mod==3)?5:11);
                                break;
                                case 0x10: /*NOT b*/
                                temp=~temp;
                                seteab(temp);
                                cycles-=((mod==3)?3:24);
                                break;
                                case 0x18: /*NEG b*/
                                setsub8(0,temp);
                                temp=0-temp;
                                seteab(temp);
                                cycles-=((mod==3)?3:24);
                                break;
                                case 0x20: /*MUL AL,b*/
                                setznp8(AL);
                                AX=AL*temp;
                                if (AX) flags&=~Z_FLAG;
                                else    flags|=Z_FLAG;
                                if (AH) flags|=(C_FLAG|V_FLAG);
                                else    flags&=~(C_FLAG|V_FLAG);
                                cycles-=70;
                                break;
                                case 0x28: /*IMUL AL,b*/
                                setznp8(AL);
                                tempws=(int)((int8_t)AL)*(int)((int8_t)temp);
                                AX=tempws&0xFFFF;
                                if (AX) flags&=~Z_FLAG;
                                else    flags|=Z_FLAG;
                                if (AH) flags|=(C_FLAG|V_FLAG);
                                else    flags&=~(C_FLAG|V_FLAG);
                                cycles-=80;
                                break;
                                case 0x30: /*DIV AL,b*/
                                tempw=AX;
                                if (temp)
                                {
                                        tempw2=tempw%temp;
/*                                        if (!tempw)
                                        {
                                                writememw((ss+SP)-2,flags|0xF000);
                                                writememw((ss+SP)-4,cs>>4);
                                                writememw((ss+SP)-6,pc);
                                                SP-=6;
                                                flags&=~I_FLAG;
                                                pc=readmemw(0);
                                                cs=readmemw(2)<<4;
                                                printf("Div by zero %04X:%04X\n",cs>>4,pc);
//                                                dumpregs();
//                                                exit(-1);
                                        }
                                        else
                                        {*/
                                                AH=tempw2;
                                                tempw/=temp;
                                                AL=tempw&0xFF;
//                                        }
                                }
                                else
                                {
                                        printf("DIVb BY 0 %04X:%04X\n",cs>>4,pc);
                                        writememw(ss,(SP-2)&0xFFFF,flags|0xF000);
                                        writememw(ss,(SP-4)&0xFFFF,CS);
                                        writememw(ss,(SP-6)&0xFFFF,pc);
                                        SP-=6;
                                        flags&=~I_FLAG;
                                        flags&=~T_FLAG;
                                        pc=readmemw(0,0);
//                        printf("F6 30\n");
                                        loadcs(readmemw(0,2));
                                        FETCHCLEAR();
//                                                cs=loadcs(CS);
//                                                cs=CS<<4;
//                                        printf("Div by zero %04X:%04X %02X %02X\n",cs>>4,pc,0xf6,0x30);
//                                        dumpregs();
//                                        exit(-1);
                                }
                                cycles-=80;
                                break;
                                case 0x38: /*IDIV AL,b*/
                                tempws=(int)AX;
                                if (temp)
                                {
                                        tempw2=tempws%(int)((int8_t)temp);
/*                                        if (!tempw)
                                        {
                                                writememw((ss+SP)-2,flags|0xF000);
                                                writememw((ss+SP)-4,cs>>4);
                                                writememw((ss+SP)-6,pc);
                                                SP-=6;
                                                flags&=~I_FLAG;
                                                pc=readmemw(0);
                                                cs=readmemw(2)<<4;
                                                printf("Div by zero %04X:%04X\n",cs>>4,pc);
                                        }
                                        else
                                        {*/
                                                AH=tempw2&0xFF;
                                                tempws/=(int)((int8_t)temp);
                                                AL=tempws&0xFF;
//                                        }
                                }
                                else
                                {
                                        printf("IDIVb BY 0 %04X:%04X\n",cs>>4,pc);
                                        writememw(ss,(SP-2)&0xFFFF,flags|0xF000);
                                        writememw(ss,(SP-4)&0xFFFF,CS);
                                        writememw(ss,(SP-6)&0xFFFF,pc);
                                        SP-=6;
                                        flags&=~I_FLAG;
                                        flags&=~T_FLAG;
                                        pc=readmemw(0,0);
//                        printf("F6 38\n");
                                        loadcs(readmemw(0,2));
                                        FETCHCLEAR();
//                                                cs=loadcs(CS);
//                                                cs=CS<<4;
//                                        printf("Div by zero %04X:%04X %02X %02X\n",cs>>4,pc,0xf6,0x38);
                                }
                                cycles-=101;
                                break;

//                                default:
//                                printf("Bad F6 opcode %02X\n",rmdat&0x38);
//                                dumpregs();
//                                exit(-1);
                        }
                        break;

                        case 0xF7:
                        fetchea();
                        tempw=geteaw();
                        switch (rmdat&0x38)
                        {
                                case 0x00: /*TEST w*/
                                tempw2=getword();
                                setznp16(tempw&tempw2);
                                flags&=~(C_FLAG|V_FLAG|A_FLAG);
                                cycles-=((mod==3)?5:11);
                                break;
                                case 0x10: /*NOT w*/
                                seteaw(~tempw);
                                cycles-=((mod==3)?3:24);
                                break;
                                case 0x18: /*NEG w*/
                                setsub16(0,tempw);
                                tempw=0-tempw;
                                seteaw(tempw);
                                cycles-=((mod==3)?3:24);
                                break;
                                case 0x20: /*MUL AX,w*/
                                setznp16(AX);
                                templ=AX*tempw;
//                                if (output) printf("%04X*%04X=%08X\n",AX,tempw,templ);
                                AX=templ&0xFFFF;
                                DX=templ>>16;
                                if (AX|DX) flags&=~Z_FLAG;
                                else       flags|=Z_FLAG;
                                if (DX)    flags|=(C_FLAG|V_FLAG);
                                else       flags&=~(C_FLAG|V_FLAG);
                                cycles-=118;
                                break;
                                case 0x28: /*IMUL AX,w*/
                                setznp16(AX);
//                                printf("IMUL %i %i ",(int)((int16_t)AX),(int)((int16_t)tempw));
                                tempws=(int)((int16_t)AX)*(int)((int16_t)tempw);
                                if ((tempws>>15) && ((tempws>>15)!=-1)) flags|=(C_FLAG|V_FLAG);
                                else                                    flags&=~(C_FLAG|V_FLAG);
//                                printf("%i ",tempws);
                                AX=tempws&0xFFFF;
                                tempws=(uint16_t)(tempws>>16);
                                DX=tempws&0xFFFF;
//                                printf("%04X %04X\n",AX,DX);
//                                dumpregs();
//                                exit(-1);
                                if (AX|DX) flags&=~Z_FLAG;
                                else       flags|=Z_FLAG;
                                cycles-=128;
                                break;
                                case 0x30: /*DIV AX,w*/
                                templ=(DX<<16)|AX;
//                                printf("DIV %08X/%04X\n",templ,tempw);
                                if (tempw)
                                {
                                        tempw2=templ%tempw;
                                        DX=tempw2;
                                        templ/=tempw;
                                        AX=templ&0xFFFF;
                                }
                                else
                                {
                                        printf("DIVw BY 0 %04X:%04X\n",cs>>4,pc);
                                        writememw(ss,(SP-2)&0xFFFF,flags|0xF000);
                                        writememw(ss,(SP-4)&0xFFFF,CS);
                                        writememw(ss,(SP-6)&0xFFFF,pc);
                                        SP-=6;
                                        flags&=~I_FLAG;
                                        flags&=~T_FLAG;
                                        pc=readmemw(0,0);
//                        printf("F7 30\n");
                                        loadcs(readmemw(0,2));
                                        FETCHCLEAR();
                                }
                                cycles-=144;
                                break;
                                case 0x38: /*IDIV AX,w*/
                                tempws=(int)((DX<<16)|AX);
//                                printf("IDIV %i %i ",tempws,tempw);
                                if (tempw)
                                {
                                        tempw2=tempws%(int)((int16_t)tempw);
//                                        printf("%04X ",tempw2);
                                                DX=tempw2;
                                                tempws/=(int)((int16_t)tempw);
                                                AX=tempws&0xFFFF;
                                }
                                else
                                {
                                        printf("IDIVw BY 0 %04X:%04X\n",cs>>4,pc);
                                        writememw(ss,(SP-2)&0xFFFF,flags|0xF000);
                                        writememw(ss,(SP-4)&0xFFFF,CS);
                                        writememw(ss,(SP-6)&0xFFFF,pc);
                                        SP-=6;
                                        flags&=~I_FLAG;
                                        flags&=~T_FLAG;
                                        pc=readmemw(0,0);
//                        printf("F7 38\n");
                                        loadcs(readmemw(0,2));
                                        FETCHCLEAR();
                                }
                                cycles-=165;
                                break;

//                                default:
//                                printf("Bad F7 opcode %02X\n",rmdat&0x38);
//                                dumpregs();
//                                exit(-1);
                        }
                        break;

                        case 0xF8: /*CLC*/
                        flags&=~C_FLAG;
                        cycles-=2;
                        break;
                        case 0xF9: /*STC*/
//                        printf("STC %04X\n",pc);
                        flags|=C_FLAG;
                        cycles-=2;
                        break;
                        case 0xFA: /*CLI*/
                        flags&=~I_FLAG;
//                        printf("CLI at %04X:%04X\n",cs>>4,pc);
                        cycles-=3;
                        break;
                        case 0xFB: /*STI*/
                        flags|=I_FLAG;
//                        printf("STI at %04X:%04X\n",cs>>4,pc);
                        cycles-=2;
                        break;
                        case 0xFC: /*CLD*/
                        flags&=~D_FLAG;
                        cycles-=2;
                        break;
                        case 0xFD: /*STD*/
                        flags|=D_FLAG;
                        cycles-=2;
                        break;

                        case 0xFE: /*INC/DEC b*/
                        fetchea();
                        temp=geteab();
                        flags&=~V_FLAG;
                        if (rmdat&0x38)
                        {
                                setsub8nc(temp,1);
                                temp2=temp-1;
                                if ((temp&0x80) && !(temp2&0x80)) flags|=V_FLAG;
                        }
                        else
                        {
                                setadd8nc(temp,1);
                                temp2=temp+1;
                                if ((temp2&0x80) && !(temp&0x80)) flags|=V_FLAG;
                        }
//                        setznp8(temp2);
                        seteab(temp2);
                        cycles-=((mod==3)?3:23);
                        break;

                        case 0xFF:
                        fetchea();
                        switch (rmdat&0x38)
                        {
                                case 0x00: /*INC w*/
                                tempw=geteaw();
                                setadd16nc(tempw,1);
//                                setznp16(tempw+1);
                                seteaw(tempw+1);
                                cycles-=((mod==3)?3:23);
                                break;
                                case 0x08: /*DEC w*/
                                tempw=geteaw();
//                                setsub16(tempw,1);
                                setsub16nc(tempw,1);
//                                setznp16(tempw-1);
                                seteaw(tempw-1);
//                                if (output) printf("DEC - %04X\n",tempw);
                                cycles-=((mod==3)?3:23);
                                break;
                                case 0x10: /*CALL*/
                                tempw=geteaw();
                                if (ssegs) ss=oldss;
                                writememw(ss,(SP-2)&0xFFFF,pc);
                                SP-=2;
                                pc=tempw;
//                        printf("FF 10\n");
                                cycles-=((mod==3)?20:29);
                                FETCHCLEAR();
                                break;
                                case 0x18: /*CALL far*/
                                tempw=readmemw(easeg,eaaddr);
                                tempw2=readmemw(easeg,(eaaddr+2)&0xFFFF); //geteaw2();
                                tempw3=CS;
                                tempw4=pc;
                                if (ssegs) ss=oldss;
                                pc=tempw;
//                        printf("FF 18\n");
                                loadcs(tempw2);
                                writememw(ss,(SP-2)&0xFFFF,tempw3);
                                writememw(ss,((SP-4)&0xFFFF),tempw4);
                                SP-=4;
                                cycles-=53;
                                FETCHCLEAR();
                                break;
                                case 0x20: /*JMP*/
                                pc=geteaw();
//                        printf("FF 20\n");
                                cycles-=((mod==3)?11:18);
                                FETCHCLEAR();
                                break;
                                case 0x28: /*JMP far*/
                                pc=readmemw(easeg,eaaddr); //geteaw();
//                        printf("FF 28\n");
                                loadcs(readmemw(easeg,(eaaddr+2)&0xFFFF)); //geteaw2();
//                                cs=loadcs(CS);
//                                cs=CS<<4;
                                cycles-=24;
                                FETCHCLEAR();
                                break;
                                case 0x30: /*PUSH w*/
                                tempw=geteaw();
//                                if (output) printf("PUSH %04X %i %02X %04X %04X %02X %02X\n",tempw,rm,rmdat,easeg,eaaddr,ram[0x22340+0x5638],ram[0x22340+0x5639]);
                                if (ssegs) ss=oldss;
                                writememw(ss,((SP-2)&0xFFFF),tempw);
                                SP-=2;
                                cycles-=((mod==3)?15:24);
                                break;

//                                default:
//                                printf("Bad FF opcode %02X\n",rmdat&0x38);
//                                dumpregs();
//                                exit(-1);
                        }
                        break;

                        default:
                        FETCH();
                        cycles-=8;
                        break;

/*                        printf("Bad opcode %02X at %04X:%04X from %04X:%04X %08X\n",opcode,cs>>4,pc,old8>>16,old8&0xFFFF,old82);
                        dumpregs();
                        exit(-1);*/
                }
                pc&=0xFFFF;

/*                if ((CS & 0xf000) == 0xa000)
                {
                        dumpregs();
                        exit(-1);
                }*/
//                output = 3;
/*                if (CS == 0xf000)
                {
                        dumpregs();
                        exit(-1);
                }
                output = 3;*/
                if (ssegs)
                {
                        ds=oldds;
                        ss=oldss;
                        ssegs=0;
                }
                
//                output = 3;
               // if (instime) printf("%i %i %i %i\n",cycdiff,cycles,memcycs,fetchclocks);
                FETCHADD(((cycdiff-cycles)-memcycs)-fetchclocks);
                if ((cycdiff-cycles)<memcycs) cycles-=(memcycs-(cycdiff-cycles));
                if (romset==ROM_IBMPC)
                {
                        if ((cs+pc)==0xFE4A7) /*You didn't seriously think I was going to emulate the cassette, did you?*/
                        {
                                CX=1;
                                BX=0x500;
                        }
                }
                memcycs=0;

                insc++;
//                output=(CS==0xEB9);
                clockhardware();


                if (trap && (flags&T_FLAG) && !noint)
                {
//                        printf("TRAP!!! %04X:%04X\n",CS,pc);
                        writememw(ss,(SP-2)&0xFFFF,flags|0xF000);
                        writememw(ss,(SP-4)&0xFFFF,CS);
                        writememw(ss,(SP-6)&0xFFFF,pc);
                        SP-=6;
                        addr=1<<2;
                        flags&=~I_FLAG;
                        flags&=~T_FLAG;
                        pc=readmemw(0,addr);
                        loadcs(readmemw(0,addr+2));
                        FETCHCLEAR();
                }
                else if (nmi && nmi_enable)
                {
//                        output = 3;
                        writememw(ss,(SP-2)&0xFFFF,flags|0xF000);
                        writememw(ss,(SP-4)&0xFFFF,CS);
                        writememw(ss,(SP-6)&0xFFFF,pc);
                        SP-=6;
                        addr=2<<2;
                        flags&=~I_FLAG;
                        flags&=~T_FLAG;
                        pc=readmemw(0,addr);
                        loadcs(readmemw(0,addr+2));
                        FETCHCLEAR();
                        nmi_enable = 0;
                }
                else if (takeint && !ssegs && !noint)
                {
                        temp=picinterrupt();
                        if (temp!=0xFF)
                        {
                                if (inhlt) pc++;
                                writememw(ss,(SP-2)&0xFFFF,flags|0xF000);
                                writememw(ss,(SP-4)&0xFFFF,CS);
                                writememw(ss,(SP-6)&0xFFFF,pc);
                                SP-=6;
                                addr=temp<<2;
                                flags&=~I_FLAG;
                                flags&=~T_FLAG;
                                pc=readmemw(0,addr);
//                        printf("INT INT INT\n");
                                loadcs(readmemw(0,addr+2));
                                FETCHCLEAR();
//                                printf("INTERRUPT\n");
                        }
                }
                takeint = (flags&I_FLAG) && (pic.pend&~pic.mask);

                if (noint) noint=0;
                ins++;
/*                if (timetolive)
                {
                        timetolive--;
                        if (!timetolive) exit(-1); //output=0;
                }*/
        }
}

