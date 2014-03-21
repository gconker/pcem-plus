#include "x86_ops.h"

OpFn *x86_opcodes;
OpFn *x86_opcodes_0f;

static inline void PUSH_W(uint16_t val)
{
        if (stack32)
        {
                writememw(ss, ESP - 2, val);              if (abrt) return;
                ESP -= 2;
        }
        else
        {
                writememw(ss, (SP - 2) & 0xFFFF, val);    if (abrt) return;
                SP -= 2;
        }
}

static inline void PUSH_L(uint32_t val)
{
        if (stack32)
        {
                writememl(ss, ESP - 4, val);              if (abrt) return;
                ESP -= 4;
        }
        else
        {
                writememl(ss, (SP - 4) & 0xFFFF, val);    if (abrt) return;
                SP -= 4;
        }
}

static inline uint16_t POP_W()
{
        uint16_t ret;
        if (stack32)
        {
                ret = readmemw(ss, ESP);                        if (abrt) return 0;
                ESP += 2;
        }
        else
        {
                ret = readmemw(ss, SP);                         if (abrt) return 0;
                SP += 2;
        }
        return ret;
}

static inline uint32_t POP_L()
{
        uint32_t ret;
        if (stack32)
        {
                ret = readmeml(ss, ESP);                        if (abrt) return 0;
                ESP += 4;
        }
        else
        {
                ret = readmeml(ss, SP);                         if (abrt) return 0;
                SP += 4;
        }
        return ret;
}

static inline uint16_t POP_W_seg(uint32_t seg)
{
        uint16_t ret;
        if (stack32)
        {
                ret = readmemw(seg, ESP);                       if (abrt) return 0;
                ESP += 2;
        }
        else
        {
                ret = readmemw(seg, SP);                        if (abrt) return 0;
                SP += 2;
        }
        return ret;
}

static inline uint32_t POP_L_seg(uint32_t seg)
{
        uint32_t ret;
        if (stack32)
        {
                ret = readmeml(seg, ESP);                       if (abrt) return 0;
                ESP += 4;
        }
        else
        {
                ret = readmeml(seg, SP);                        if (abrt) return 0;
                SP += 4;
        }
        return ret;
}

#include "x86_ops_arith.h"
#include "x86_ops_atomic.h"
#include "x86_ops_bcd.h"
#include "x86_ops_bit.h"
#include "x86_ops_bitscan.h"
#include "x86_ops_call.h"
#include "x86_ops_flag.h"
#include "x86_ops_fpu.h"
#include "x86_ops_inc_dec.h"
#include "x86_ops_int.h"
#include "x86_ops_io.h"
#include "x86_ops_jump.h"
#include "x86_ops_misc.h"
#include "x86_ops_mmx.h"
#include "x86_ops_mmx_arith.h"
#include "x86_ops_mmx_cmp.h"
#include "x86_ops_mmx_logic.h"
#include "x86_ops_mmx_mov.h"
#include "x86_ops_mmx_pack.h"
#include "x86_ops_mmx_shift.h"
#include "x86_ops_mov.h"
#include "x86_ops_mov_ctrl.h"
#include "x86_ops_mov_seg.h"
#include "x86_ops_movx.h"
#include "x86_ops_msr.h"
#include "x86_ops_mul.h"
#include "x86_ops_pmode.h"
#include "x86_ops_prefix.h"
#include "x86_ops_rep.h"
#include "x86_ops_ret.h"
#include "x86_ops_set.h"
#include "x86_ops_shift.h"
#include "x86_ops_stack.h"
#include "x86_ops_string.h"
#include "x86_ops_xchg.h"

static int ILLEGAL(uint32_t fetchdat)
{
        pc = oldpc;

//        fatal("Illegal instruction %08X\n", fetchdat);
        x86illegal();
        return 0;
}

static int op0F_w_a16(uint32_t fetchdat)
{
        int opcode = fetchdat & 0xff;
        pc++;

        return x86_opcodes_0f[opcode](fetchdat >> 8);
}
static int op0F_l_a16(uint32_t fetchdat)
{
        int opcode = fetchdat & 0xff;
        pc++;
        
        return x86_opcodes_0f[opcode | 0x100](fetchdat >> 8);
}
static int op0F_w_a32(uint32_t fetchdat)
{
        int opcode = fetchdat & 0xff;
        pc++;
        
        return x86_opcodes_0f[opcode | 0x200](fetchdat >> 8);
}
static int op0F_l_a32(uint32_t fetchdat)
{
        int opcode = fetchdat & 0xff;
        pc++;
        
        return x86_opcodes_0f[opcode | 0x300](fetchdat >> 8);
}

void x86_setopcodes(OpFn *opcodes, OpFn *opcodes_0f)
{
        x86_opcodes = opcodes;
        x86_opcodes_0f = opcodes_0f;
}
        
OpFn ops_286_0f[1024] = 
{
        /*16-bit data, 16-bit addr*/
/*      00              01              02              03              04              05              06              07              08              09              0a              0b              0c              0d              0e              0f*/        
/*00*/  op0F00_a16,     op0F01_w_a16,   opLAR_w_a16,    opLSL_w_a16,    ILLEGAL,        opLOADALL,      opCLTS,         ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*10*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*20*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*30*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,

/*40*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*50*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*60*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*70*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,

/*80*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*90*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*a0*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*b0*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,

/*c0*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*d0*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*e0*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*f0*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,

        /*32-bit data, 16-bit addr*/
/*      00              01              02              03              04              05              06              07              08              09              0a              0b              0c              0d              0e              0f*/        
/*00*/  op0F00_a16,     op0F01_w_a16,   opLAR_w_a16,    opLSL_w_a16,    ILLEGAL,        opLOADALL,      opCLTS,         ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*10*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*20*/  opMOV_r_CRx_a16,opMOV_r_DRx_a16,opMOV_CRx_r_a16,opMOV_DRx_r_a16,opMOV_r_TRx_a16,opMOV_TRx_r_a16,ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*30*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,

/*40*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*50*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*60*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*70*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,

/*80*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*90*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*a0*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*b0*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,

/*c0*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*d0*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*e0*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*f0*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,

        /*16-bit data, 32-bit addr*/
/*      00              01              02              03              04              05              06              07              08              09              0a              0b              0c              0d              0e              0f*/        
/*00*/  op0F00_a16,     op0F01_w_a16,   opLAR_w_a16,    opLSL_w_a16,    ILLEGAL,        opLOADALL,      opCLTS,         ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*10*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*20*/  opMOV_r_CRx_a16,opMOV_r_DRx_a16,opMOV_CRx_r_a16,opMOV_DRx_r_a16,opMOV_r_TRx_a16,opMOV_TRx_r_a16,ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*30*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,

/*40*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*50*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*60*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*70*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,

/*80*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*90*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*a0*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*b0*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,

/*c0*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*d0*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*e0*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*f0*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,

        /*32-bit data, 32-bit addr*/
/*      00              01              02              03              04              05              06              07              08              09              0a              0b              0c              0d              0e              0f*/        
/*00*/  op0F00_a16,     op0F01_w_a16,   opLAR_w_a16,    opLSL_w_a16,    ILLEGAL,        opLOADALL,      opCLTS,         ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*10*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*20*/  opMOV_r_CRx_a16,opMOV_r_DRx_a16,opMOV_CRx_r_a16,opMOV_DRx_r_a16,opMOV_r_TRx_a16,opMOV_TRx_r_a16,ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*30*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,

/*40*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*50*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*60*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*70*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,

/*80*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*90*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*a0*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*b0*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,

/*c0*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*d0*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*e0*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*f0*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
};

OpFn ops_386_0f[1024] = 
{
        /*16-bit data, 16-bit addr*/
/*      00              01              02              03              04              05              06              07              08              09              0a              0b              0c              0d              0e              0f*/        
/*00*/  op0F00_a16,     op0F01_w_a16,   opLAR_w_a16,    opLSL_w_a16,    ILLEGAL,        opLOADALL,      opCLTS,         ILLEGAL,        opINVD,         opWBINVD,       ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*10*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*20*/  opMOV_r_CRx_a16,opMOV_r_DRx_a16,opMOV_CRx_r_a16,opMOV_DRx_r_a16,opMOV_r_TRx_a16,opMOV_TRx_r_a16,ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*30*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,

/*40*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*50*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*60*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*70*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,

/*80*/  opJO_w,         opJNO_w,        opJB_w,         opJNB_w,        opJE_w,         opJNE_w,        opJBE_w,        opJNBE_w,       opJS_w,         opJNS_w,        opJP_w,         opJNP_w,        opJL_w,         opJNL_w,        opJLE_w,        opJNLE_w,
/*90*/  opSETO_a16,     opSETNO_a16,    opSETB_a16,     opSETNB_a16,    opSETE_a16,     opSETNE_a16,    opSETBE_a16,    opSETNBE_a16,   opSETS_a16,     opSETNS_a16,    opSETP_a16,     opSETNP_a16,    opSETL_a16,     opSETNL_a16,    opSETLE_a16,    opSETNLE_a16,
/*a0*/  opPUSH_FS_w,    opPOP_FS_w,     opCPUID,        opBT_w_r_a16,   opSHLD_w_i_a16, opSHLD_w_CL_a16,ILLEGAL,        ILLEGAL,        opPUSH_GS_w,    opPOP_GS_w,     ILLEGAL,        opBTS_w_r_a16,  opSHRD_w_i_a16, opSHRD_w_CL_a16,ILLEGAL,        opIMUL_w_w_a16,
/*b0*/  opCMPXCHG_b_a16,opCMPXCHG_w_a16,opLSS_w_a16,    opBTR_w_r_a16,  opLFS_w_a16,    opLGS_w_a16,    opMOVZX_w_b_a16,opMOVZX_w_w_a16,ILLEGAL,        ILLEGAL,        opBA_w_a16,     opBTC_w_r_a16,  opBSF_w_a16,    opBSR_w_a16,    opMOVSX_w_b_a16,ILLEGAL,

/*c0*/  opXADD_b_a16,   opXADD_w_a16,   ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        opBSWAP_EAX,    opBSWAP_ECX,    opBSWAP_EDX,    opBSWAP_EBX,    opBSWAP_ESP,    opBSWAP_EBP,    opBSWAP_ESI,    opBSWAP_EDI,
/*d0*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*e0*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*f0*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,

        /*32-bit data, 16-bit addr*/
/*      00              01              02              03              04              05              06              07              08              09              0a              0b              0c              0d              0e              0f*/        
/*00*/  op0F00_a16,     op0F01_l_a16,   opLAR_l_a16,    opLSL_l_a16,    ILLEGAL,        opLOADALL,      opCLTS,         ILLEGAL,        opINVD,         opWBINVD,       ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*10*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*20*/  opMOV_r_CRx_a16,opMOV_r_DRx_a16,opMOV_CRx_r_a16,opMOV_DRx_r_a16,opMOV_r_TRx_a16,opMOV_TRx_r_a16,ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*30*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,

/*40*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*50*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*60*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*70*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,

/*80*/  opJO_l,         opJNO_l,        opJB_l,         opJNB_l,        opJE_l,         opJNE_l,        opJBE_l,        opJNBE_l,       opJS_l,         opJNS_l,        opJP_l,         opJNP_l,        opJL_l,         opJNL_l,        opJLE_l,        opJNLE_l,
/*90*/  opSETO_a16,     opSETNO_a16,    opSETB_a16,     opSETNB_a16,    opSETE_a16,     opSETNE_a16,    opSETBE_a16,    opSETNBE_a16,   opSETS_a16,     opSETNS_a16,    opSETP_a16,     opSETNP_a16,    opSETL_a16,     opSETNL_a16,    opSETLE_a16,    opSETNLE_a16,
/*a0*/  opPUSH_FS_l,    opPOP_FS_l,     opCPUID,        opBT_l_r_a16,   opSHLD_l_i_a16, opSHLD_l_CL_a16,ILLEGAL,        ILLEGAL,        opPUSH_GS_l,    opPOP_GS_l,     ILLEGAL,        opBTS_l_r_a16,  opSHRD_l_i_a16, opSHRD_l_CL_a16,ILLEGAL,        opIMUL_l_l_a16,
/*b0*/  opCMPXCHG_b_a16,opCMPXCHG_l_a16,opLSS_l_a16,    opBTR_l_r_a16,  opLFS_l_a16,    opLGS_l_a16,    opMOVZX_l_b_a16,opMOVZX_l_w_a16,ILLEGAL,        ILLEGAL,        opBA_l_a16,     opBTC_l_r_a16,  opBSF_l_a16,    opBSR_l_a16,    opMOVSX_l_b_a16,opMOVSX_l_w_a16,

/*c0*/  opXADD_b_a16,   opXADD_l_a16,   ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        opBSWAP_EAX,    opBSWAP_ECX,    opBSWAP_EDX,    opBSWAP_EBX,    opBSWAP_ESP,    opBSWAP_EBP,    opBSWAP_ESI,    opBSWAP_EDI,
/*d0*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*e0*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*f0*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,

        /*16-bit data, 32-bit addr*/
/*      00              01              02              03              04              05              06              07              08              09              0a              0b              0c              0d              0e              0f*/        
/*00*/  op0F00_a32,     op0F01_w_a32,   opLAR_w_a32,    opLSL_w_a32,    ILLEGAL,        opLOADALL,      opCLTS,         ILLEGAL,        opINVD,         opWBINVD,       ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*10*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*20*/  opMOV_r_CRx_a32,opMOV_r_DRx_a32,opMOV_CRx_r_a32,opMOV_DRx_r_a32,opMOV_r_TRx_a32,opMOV_TRx_r_a32,ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*30*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,

/*40*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*50*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*60*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*70*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,

/*80*/  opJO_w,         opJNO_w,        opJB_w,         opJNB_w,        opJE_w,         opJNE_w,        opJBE_w,        opJNBE_w,       opJS_w,         opJNS_w,        opJP_w,         opJNP_w,        opJL_w,         opJNL_w,        opJLE_w,        opJNLE_w,
/*90*/  opSETO_a32,     opSETNO_a32,    opSETB_a32,     opSETNB_a32,    opSETE_a32,     opSETNE_a32,    opSETBE_a32,    opSETNBE_a32,   opSETS_a32,     opSETNS_a32,    opSETP_a32,     opSETNP_a32,    opSETL_a32,     opSETNL_a32,    opSETLE_a32,    opSETNLE_a32,
/*a0*/  opPUSH_FS_w,    opPOP_FS_w,     opCPUID,        opBT_w_r_a32,   opSHLD_w_i_a32, opSHLD_w_CL_a32,ILLEGAL,        ILLEGAL,        opPUSH_GS_w,    opPOP_GS_w,     ILLEGAL,        opBTS_w_r_a32,  opSHRD_w_i_a32, opSHRD_w_CL_a32,ILLEGAL,        opIMUL_w_w_a32,
/*b0*/  opCMPXCHG_b_a32,opCMPXCHG_w_a32,opLSS_w_a32,    opBTR_w_r_a32,  opLFS_w_a32,    opLGS_w_a32,    opMOVZX_w_b_a32,opMOVZX_w_w_a32,ILLEGAL,        ILLEGAL,        opBA_w_a32,     opBTC_w_r_a32,  opBSF_w_a32,    opBSR_w_a32,    opMOVSX_w_b_a32,ILLEGAL,

/*c0*/  opXADD_b_a32,   opXADD_w_a32,   ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        opBSWAP_EAX,    opBSWAP_ECX,    opBSWAP_EDX,    opBSWAP_EBX,    opBSWAP_ESP,    opBSWAP_EBP,    opBSWAP_ESI,    opBSWAP_EDI,
/*d0*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*e0*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*f0*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,

        /*32-bit data, 32-bit addr*/
/*      00              01              02              03              04              05              06              07              08              09              0a              0b              0c              0d              0e              0f*/        
/*00*/  op0F00_a32,     op0F01_l_a32,   opLAR_l_a32,    opLSL_l_a32,    ILLEGAL,        opLOADALL,      opCLTS,         ILLEGAL,        opINVD,         opWBINVD,       ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*10*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*20*/  opMOV_r_CRx_a32,opMOV_r_DRx_a32,opMOV_CRx_r_a32,opMOV_DRx_r_a32,opMOV_r_TRx_a32,opMOV_TRx_r_a32,ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*30*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,

/*40*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*50*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*60*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*70*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,

/*80*/  opJO_l,         opJNO_l,        opJB_l,         opJNB_l,        opJE_l,         opJNE_l,        opJBE_l,        opJNBE_l,       opJS_l,         opJNS_l,        opJP_l,         opJNP_l,        opJL_l,         opJNL_l,        opJLE_l,        opJNLE_l,
/*90*/  opSETO_a32,     opSETNO_a32,    opSETB_a32,     opSETNB_a32,    opSETE_a32,     opSETNE_a32,    opSETBE_a32,    opSETNBE_a32,   opSETS_a32,     opSETNS_a32,    opSETP_a32,     opSETNP_a32,    opSETL_a32,     opSETNL_a32,    opSETLE_a32,    opSETNLE_a32,
/*a0*/  opPUSH_FS_l,    opPOP_FS_l,     opCPUID,        opBT_l_r_a32,   opSHLD_l_i_a32, opSHLD_l_CL_a32,ILLEGAL,        ILLEGAL,        opPUSH_GS_l,    opPOP_GS_l,     ILLEGAL,        opBTS_l_r_a32,  opSHRD_l_i_a32, opSHRD_l_CL_a32,ILLEGAL,        opIMUL_l_l_a32,
/*b0*/  opCMPXCHG_b_a32,opCMPXCHG_l_a32,opLSS_l_a32,    opBTR_l_r_a32,  opLFS_l_a32,    opLGS_l_a32,    opMOVZX_l_b_a32,opMOVZX_l_w_a32,ILLEGAL,        ILLEGAL,        opBA_l_a32,     opBTC_l_r_a32,  opBSF_l_a32,    opBSR_l_a32,    opMOVSX_l_b_a32,opMOVSX_l_w_a32,

/*c0*/  opXADD_b_a32,   opXADD_l_a32,   ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        opBSWAP_EAX,    opBSWAP_ECX,    opBSWAP_EDX,    opBSWAP_EBX,    opBSWAP_ESP,    opBSWAP_EBP,    opBSWAP_ESI,    opBSWAP_EDI,
/*d0*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*e0*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*f0*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
};

OpFn ops_winchip_0f[1024] = 
{
        /*16-bit data, 16-bit addr*/
/*      00              01              02              03              04              05              06              07              08              09              0a              0b              0c              0d              0e              0f*/        
/*00*/  op0F00_a16,     op0F01_w_a16,   opLAR_w_a16,    opLSL_w_a16,    ILLEGAL,        opLOADALL,      opCLTS,         ILLEGAL,        opINVD,         opWBINVD,       ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*10*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*20*/  opMOV_r_CRx_a16,opMOV_r_DRx_a16,opMOV_CRx_r_a16,opMOV_DRx_r_a16,opMOV_r_TRx_a16,opMOV_TRx_r_a16,ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*30*/  opWRMSR,        opRDTSC,        opRDMSR,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,

/*40*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*50*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*60*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*70*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,

/*80*/  opJO_w,         opJNO_w,        opJB_w,         opJNB_w,        opJE_w,         opJNE_w,        opJBE_w,        opJNBE_w,       opJS_w,         opJNS_w,        opJP_w,         opJNP_w,        opJL_w,         opJNL_w,        opJLE_w,        opJNLE_w,
/*90*/  opSETO_a16,     opSETNO_a16,    opSETB_a16,     opSETNB_a16,    opSETE_a16,     opSETNE_a16,    opSETBE_a16,    opSETNBE_a16,   opSETS_a16,     opSETNS_a16,    opSETP_a16,     opSETNP_a16,    opSETL_a16,     opSETNL_a16,    opSETLE_a16,    opSETNLE_a16,
/*a0*/  opPUSH_FS_w,    opPOP_FS_w,     opCPUID,        opBT_w_r_a16,   opSHLD_w_i_a16, opSHLD_w_CL_a16,ILLEGAL,        ILLEGAL,        opPUSH_GS_w,    opPOP_GS_w,     ILLEGAL,        opBTS_w_r_a16,  opSHRD_w_i_a16, opSHRD_w_CL_a16,ILLEGAL,        opIMUL_w_w_a16,
/*b0*/  opCMPXCHG_b_a16,opCMPXCHG_w_a16,opLSS_w_a16,    opBTR_w_r_a16,  opLFS_w_a16,    opLGS_w_a16,    opMOVZX_w_b_a16,opMOVZX_w_w_a16,ILLEGAL,        ILLEGAL,        opBA_w_a16,     opBTC_w_r_a16,  opBSF_w_a16,    opBSR_w_a16,    opMOVSX_w_b_a16,ILLEGAL,

/*c0*/  opXADD_b_a16,   opXADD_w_a16,   ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        opBSWAP_EAX,    opBSWAP_ECX,    opBSWAP_EDX,    opBSWAP_EBX,    opBSWAP_ESP,    opBSWAP_EBP,    opBSWAP_ESI,    opBSWAP_EDI,
/*d0*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*e0*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*f0*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,

        /*32-bit data, 16-bit addr*/
/*      00              01              02              03              04              05              06              07              08              09              0a              0b              0c              0d              0e              0f*/        
/*00*/  op0F00_a16,     op0F01_l_a16,   opLAR_l_a16,    opLSL_l_a16,    ILLEGAL,        opLOADALL,      opCLTS,         ILLEGAL,        opINVD,         opWBINVD,       ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*10*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*20*/  opMOV_r_CRx_a16,opMOV_r_DRx_a16,opMOV_CRx_r_a16,opMOV_DRx_r_a16,opMOV_r_TRx_a16,opMOV_TRx_r_a16,ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*30*/  opWRMSR,        opRDTSC,        opRDMSR,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,

/*40*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*50*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*60*/  opPUNPCKLBW_a16,opPUNPCKLWD_a16,opPUNPCKLDQ_a16,opPACKSSWB_a16, opPCMPGTB_a16,  opPCMPGTW_a16,  opPCMPGTD_a16,  opPACKUSWB_a16, opPUNPCKHBW_a16,opPUNPCKHWD_a16,opPUNPCKHDQ_a16,opPACKSSDW_a16, ILLEGAL,        ILLEGAL,        opMOVD_l_mm_a16,opMOVQ_q_mm_a16,
/*70*/  ILLEGAL,        opPSxxW_imm,    opPSxxD_imm,    opPSxxQ_imm,    opPCMPEQB_a16,  opPCMPEQW_a16,  opPCMPEQD_a16,  opEMMS,         ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        opMOVD_mm_l_a16,opMOVQ_mm_q_a16,

/*80*/  opJO_l,         opJNO_l,        opJB_l,         opJNB_l,        opJE_l,         opJNE_l,        opJBE_l,        opJNBE_l,       opJS_l,         opJNS_l,        opJP_l,         opJNP_l,        opJL_l,         opJNL_l,        opJLE_l,        opJNLE_l,
/*90*/  opSETO_a16,     opSETNO_a16,    opSETB_a16,     opSETNB_a16,    opSETE_a16,     opSETNE_a16,    opSETBE_a16,    opSETNBE_a16,   opSETS_a16,     opSETNS_a16,    opSETP_a16,     opSETNP_a16,    opSETL_a16,     opSETNL_a16,    opSETLE_a16,    opSETNLE_a16,
/*a0*/  opPUSH_FS_l,    opPOP_FS_l,     opCPUID,        opBT_l_r_a16,   opSHLD_l_i_a16, opSHLD_l_CL_a16,ILLEGAL,        ILLEGAL,        opPUSH_GS_l,    opPOP_GS_l,     ILLEGAL,        opBTS_l_r_a16,  opSHRD_l_i_a16, opSHRD_l_CL_a16,ILLEGAL,        opIMUL_l_l_a16,
/*b0*/  opCMPXCHG_b_a16,opCMPXCHG_l_a16,opLSS_l_a16,    opBTR_l_r_a16,  opLFS_l_a16,    opLGS_l_a16,    opMOVZX_l_b_a16,opMOVZX_l_w_a16,ILLEGAL,        ILLEGAL,        opBA_l_a16,     opBTC_l_r_a16,  opBSF_l_a16,    opBSR_l_a16,    opMOVSX_l_b_a16,opMOVSX_l_w_a16,

/*c0*/  opXADD_b_a16,   opXADD_l_a16,   ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        opBSWAP_EAX,    opBSWAP_ECX,    opBSWAP_EDX,    opBSWAP_EBX,    opBSWAP_ESP,    opBSWAP_EBP,    opBSWAP_ESI,    opBSWAP_EDI,
/*d0*/  ILLEGAL,        opPSRLW_a16,    opPSRLD_a16,    opPSRLQ_a16,    ILLEGAL,        opPMULLW_a16,   ILLEGAL,        ILLEGAL,        opPSUBUSB_a16,  opPSUBUSW_a16,  NULL,           opPAND_a16,     opPADDUSB_a16,  opPADDUSW_a16,  NULL,           opPANDN_a16,
/*e0*/  ILLEGAL,        opPSRAW_a16,    opPSRAD_a16,    ILLEGAL,        ILLEGAL,        opPMULHW_a16,   ILLEGAL,        ILLEGAL,        opPSUBSB_a16,   opPSUBSW_a16,   NULL,           opPOR_a16,      opPADDSB_a16,   opPADDSW_a16,   NULL,           opPXOR_a16,
/*f0*/  ILLEGAL,        opPSLLW_a16,    opPSLLD_a16,    opPSLLQ_a16,    ILLEGAL,        opPMADDWD_a16,  ILLEGAL,        ILLEGAL,        opPSUBB_a16,    opPSUBW_a16,    opPSUBD_a16,    ILLEGAL,        opPADDB_a16,    opPADDW_a16,    opPADDD_a16,    ILLEGAL,

        /*16-bit data, 32-bit addr*/
/*      00              01              02              03              04              05              06              07              08              09              0a              0b              0c              0d              0e              0f*/        
/*00*/  op0F00_a32,     op0F01_w_a32,   opLAR_w_a32,    opLSL_w_a32,    ILLEGAL,        opLOADALL,      opCLTS,         ILLEGAL,        opINVD,         opWBINVD,       ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*10*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*20*/  opMOV_r_CRx_a32,opMOV_r_DRx_a32,opMOV_CRx_r_a32,opMOV_DRx_r_a32,opMOV_r_TRx_a32,opMOV_TRx_r_a32,ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*30*/  opWRMSR,        opRDTSC,        opRDMSR,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,

/*40*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*50*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*60*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*70*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,

/*80*/  opJO_w,         opJNO_w,        opJB_w,         opJNB_w,        opJE_w,         opJNE_w,        opJBE_w,        opJNBE_w,       opJS_w,         opJNS_w,        opJP_w,         opJNP_w,        opJL_w,         opJNL_w,        opJLE_w,        opJNLE_w,
/*90*/  opSETO_a32,     opSETNO_a32,    opSETB_a32,     opSETNB_a32,    opSETE_a32,     opSETNE_a32,    opSETBE_a32,    opSETNBE_a32,   opSETS_a32,     opSETNS_a32,    opSETP_a32,     opSETNP_a32,    opSETL_a32,     opSETNL_a32,    opSETLE_a32,    opSETNLE_a32,
/*a0*/  opPUSH_FS_w,    opPOP_FS_w,     opCPUID,        opBT_w_r_a32,   opSHLD_w_i_a32, opSHLD_w_CL_a32,ILLEGAL,        ILLEGAL,        opPUSH_GS_w,    opPOP_GS_w,     ILLEGAL,        opBTS_w_r_a32,  opSHRD_w_i_a32, opSHRD_w_CL_a32,ILLEGAL,        opIMUL_w_w_a32,
/*b0*/  opCMPXCHG_b_a32,opCMPXCHG_w_a32,opLSS_w_a32,    opBTR_w_r_a32,  opLFS_w_a32,    opLGS_w_a32,    opMOVZX_w_b_a32,opMOVZX_w_w_a32,ILLEGAL,        ILLEGAL,        opBA_w_a32,     opBTC_w_r_a32,  opBSF_w_a32,    opBSR_w_a32,    opMOVSX_w_b_a32,ILLEGAL,

/*c0*/  opXADD_b_a32,   opXADD_w_a32,   ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        opBSWAP_EAX,    opBSWAP_ECX,    opBSWAP_EDX,    opBSWAP_EBX,    opBSWAP_ESP,    opBSWAP_EBP,    opBSWAP_ESI,    opBSWAP_EDI,
/*d0*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*e0*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*f0*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,

        /*32-bit data, 32-bit addr*/
/*      00              01              02              03              04              05              06              07              08              09              0a              0b              0c              0d              0e              0f*/        
/*00*/  op0F00_a32,     op0F01_l_a32,   opLAR_l_a32,    opLSL_l_a32,    ILLEGAL,        opLOADALL,      opCLTS,         ILLEGAL,        opINVD,         opWBINVD,       ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*10*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*20*/  opMOV_r_CRx_a32,opMOV_r_DRx_a32,opMOV_CRx_r_a32,opMOV_DRx_r_a32,opMOV_r_TRx_a32,opMOV_TRx_r_a32,ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*30*/  opWRMSR,        opRDTSC,        opRDMSR,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,

/*40*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*50*/  ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,
/*60*/  opPUNPCKLBW_a32,opPUNPCKLWD_a32,opPUNPCKLDQ_a32,opPACKSSWB_a32, opPCMPGTB_a32,  opPCMPGTW_a32,  opPCMPGTD_a32,  opPACKUSWB_a32, opPUNPCKHBW_a32,opPUNPCKHWD_a32,opPUNPCKHDQ_a32,opPACKSSDW_a32, ILLEGAL,        ILLEGAL,        opMOVD_l_mm_a32,opMOVQ_q_mm_a32,
/*70*/  ILLEGAL,        opPSxxW_imm,    opPSxxD_imm,    opPSxxQ_imm,    opPCMPEQB_a32,  opPCMPEQW_a32,  opPCMPEQD_a32,  opEMMS,         ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        opMOVD_mm_l_a32,opMOVQ_mm_q_a32,

/*80*/  opJO_l,         opJNO_l,        opJB_l,         opJNB_l,        opJE_l,         opJNE_l,        opJBE_l,        opJNBE_l,       opJS_l,         opJNS_l,        opJP_l,         opJNP_l,        opJL_l,         opJNL_l,        opJLE_l,        opJNLE_l,
/*90*/  opSETO_a32,     opSETNO_a32,    opSETB_a32,     opSETNB_a32,    opSETE_a32,     opSETNE_a32,    opSETBE_a32,    opSETNBE_a32,   opSETS_a32,     opSETNS_a32,    opSETP_a32,     opSETNP_a32,    opSETL_a32,     opSETNL_a32,    opSETLE_a32,    opSETNLE_a32,
/*a0*/  opPUSH_FS_l,    opPOP_FS_l,     opCPUID,        opBT_l_r_a32,   opSHLD_l_i_a32, opSHLD_l_CL_a32,ILLEGAL,        ILLEGAL,        opPUSH_GS_l,    opPOP_GS_l,     ILLEGAL,        opBTS_l_r_a32,  opSHRD_l_i_a32, opSHRD_l_CL_a32,ILLEGAL,        opIMUL_l_l_a32,
/*b0*/  opCMPXCHG_b_a32,opCMPXCHG_l_a32,opLSS_l_a32,    opBTR_l_r_a32,  opLFS_l_a32,    opLGS_l_a32,    opMOVZX_l_b_a32,opMOVZX_l_w_a32,ILLEGAL,        ILLEGAL,        opBA_l_a32,     opBTC_l_r_a32,  opBSF_l_a32,    opBSR_l_a32,    opMOVSX_l_b_a32,opMOVSX_l_w_a32,

/*c0*/  opXADD_b_a32,   opXADD_l_a32,   ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        opBSWAP_EAX,    opBSWAP_ECX,    opBSWAP_EDX,    opBSWAP_EBX,    opBSWAP_ESP,    opBSWAP_EBP,    opBSWAP_ESI,    opBSWAP_EDI,
/*d0*/  ILLEGAL,        opPSRLW_a32,    opPSRLD_a32,    opPSRLQ_a32,    ILLEGAL,        opPMULLW_a32,   ILLEGAL,        ILLEGAL,        opPSUBUSB_a32,  opPSUBUSW_a32,  NULL,           opPAND_a32,     opPADDUSB_a32,  opPADDUSW_a32,  NULL,           opPANDN_a32,
/*e0*/  ILLEGAL,        opPSRAW_a32,    opPSRAD_a32,    ILLEGAL,        ILLEGAL,        opPMULHW_a32,   ILLEGAL,        ILLEGAL,        opPSUBSB_a32,   opPSUBSW_a32,   NULL,           opPOR_a32,      opPADDSB_a32,   opPADDSW_a32,   NULL,           opPXOR_a32,
/*f0*/  ILLEGAL,        opPSLLW_a32,    opPSLLD_a32,    opPSLLQ_a32,    ILLEGAL,        opPMADDWD_a32,  ILLEGAL,        ILLEGAL,        opPSUBB_a32,    opPSUBW_a32,    opPSUBD_a32,    ILLEGAL,        opPADDB_a32,    opPADDW_a32,    opPADDD_a32,    ILLEGAL,
};

OpFn ops_286[1024] = 
{
        /*16-bit data, 16-bit addr*/
/*      00              01              02              03              04              05              06              07              08              09              0a              0b              0c              0d              0e              0f*/        
/*00*/  opADD_b_rmw_a16,opADD_w_rmw_a16,opADD_b_rm_a16, opADD_w_rm_a16, opADD_AL_imm,   opADD_AX_imm,   opPUSH_ES_w,    opPOP_ES_w,     opOR_b_rmw_a16, opOR_w_rmw_a16, opOR_b_rm_a16,  opOR_w_rm_a16,  opOR_AL_imm,    opOR_AX_imm,    opPUSH_CS_w,    op0F_w_a16,
/*10*/  opADC_b_rmw_a16,opADC_w_rmw_a16,opADC_b_rm_a16, opADC_w_rm_a16, opADC_AL_imm,   opADC_AX_imm,   opPUSH_SS_w,    opPOP_SS_w,     opSBB_b_rmw_a16,opSBB_w_rmw_a16,opSBB_b_rm_a16, opSBB_w_rm_a16, opSBB_AL_imm,   opSBB_AX_imm,   opPUSH_DS_w,    opPOP_DS_w,
/*20*/  opAND_b_rmw_a16,opAND_w_rmw_a16,opAND_b_rm_a16, opAND_w_rm_a16, opAND_AL_imm,   opAND_AX_imm,   op_ES,          opDAA,          opSUB_b_rmw_a16,opSUB_w_rmw_a16,opSUB_b_rm_a16, opSUB_w_rm_a16, opSUB_AL_imm,   opSUB_AX_imm,   op_CS,          opDAS,
/*30*/  opXOR_b_rmw_a16,opXOR_w_rmw_a16,opXOR_b_rm_a16, opXOR_w_rm_a16, opXOR_AL_imm,   opXOR_AX_imm,   op_SS,          opAAA,          opCMP_b_rmw_a16,opCMP_w_rmw_a16,opCMP_b_rm_a16, opCMP_w_rm_a16, opCMP_AL_imm,   opCMP_AX_imm,   op_DS,          opAAS,

/*40*/  opINC_AX,       opINC_CX,       opINC_DX,       opINC_BX,       opINC_SP,       opINC_BP,       opINC_SI,       opINC_DI,       opDEC_AX,       opDEC_CX,       opDEC_DX,       opDEC_BX,       opDEC_SP,       opDEC_BP,       opDEC_SI,       opDEC_DI,  
/*50*/  opPUSH_AX,      opPUSH_CX,      opPUSH_DX,      opPUSH_BX,      opPUSH_SP,      opPUSH_BP,      opPUSH_SI,      opPUSH_DI,      opPOP_AX,       opPOP_CX,       opPOP_DX,       opPOP_BX,       opPOP_SP,       opPOP_BP,       opPOP_SI,       opPOP_DI, 
/*60*/  opPUSHA_w,      opPOPA_w,       opBOUND_w_a16,  opARPL_a16,     ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        opPUSH_imm_w,   opIMUL_w_iw_a16,opPUSH_imm_bw,  opIMUL_w_ib_a16,opINSB_a16,     opINSW_a16,     opOUTSB_a16,    opOUTSW_a16,
/*70*/  opJO,           opJNO,          opJB,           opJNB,          opJE,           opJNE,          opJBE,          opJNBE,         opJS,           opJNS,          opJP,           opJNP,          opJL,           opJNL,          opJLE,          opJNLE,

/*80*/  op80_a16,       op81_w_a16,     op80_a16,       op83_w_a16,     opTEST_b_a16,   opTEST_w_a16,   opXCHG_b_a16,   opXCHG_w_a16,   opMOV_b_r_a16,  opMOV_w_r_a16,  opMOV_r_b_a16,  opMOV_r_w_a16,  opMOV_w_seg_a16,opLEA_w_a16,    opMOV_seg_w_a16,opPOPW_a16,
/*90*/  opNOP,          opXCHG_AX_CX,   opXCHG_AX_DX,   opXCHG_AX_BX,   opXCHG_AX_SP,   opXCHG_AX_BP,   opXCHG_AX_SI,   opXCHG_AX_DI,   opCBW,          opCWD,          opCALL_far_w,   opWAIT,         opPUSHF,        opPOPF_286,     opSAHF,         opLAHF,
/*a0*/  opMOV_AL_a16,   opMOV_AX_a16,   opMOV_a16_AL,   opMOV_a16_AX,   opMOVSB_a16,    opMOVSW_a16,    opCMPSB_a16,    opCMPSW_a16,    opTEST_AL,      opTEST_AX,      opSTOSB_a16,    opSTOSW_a16,    opLODSB_a16,    opLODSW_a16,    opSCASB_a16,    opSCASW_a16,
/*b0*/  opMOV_AL_imm,   opMOV_CL_imm,   opMOV_DL_imm,   opMOV_BL_imm,   opMOV_AH_imm,   opMOV_CH_imm,   opMOV_DH_imm,   opMOV_BH_imm,   opMOV_AX_imm,   opMOV_CX_imm,   opMOV_DX_imm,   opMOV_BX_imm,   opMOV_SP_imm,   opMOV_BP_imm,   opMOV_SI_imm,   opMOV_DI_imm,

/*c0*/  opC0_a16,       opC1_w_a16,     opRET_w_imm,    opRET_w,        opLES_w_a16,    opLDS_w_a16,    opMOV_b_imm_a16,opMOV_w_imm_a16,opENTER_w,      opLEAVE_w,      opRETF_a16_imm, opRETF_a16,     opINT3,         opINT,          opINTO,         opIRET_286,
/*d0*/  opD0_a16,       opD1_w_a16,     opD2_a16,       opD3_w_a16,     opAAM,          opAAD,          opSETALC,       opXLAT_a16,     opESCAPE_d8_a16,opESCAPE_d9_a16,opESCAPE_da_a16,opESCAPE_db_a16,opESCAPE_dc_a16,opESCAPE_dd_a16,opESCAPE_de_a16,opESCAPE_df_a16,
/*e0*/  opLOOPNE_w,     opLOOPE_w,      opLOOP_w,       opJCXZ,         opIN_AL_imm,    opIN_AX_imm,    opOUT_AL_imm,   opOUT_AX_imm,   opCALL_r16,     opJMP_r16,      opJMP_far_a16,  opJMP_r8,       opIN_AL_DX,     opIN_AX_DX,     opOUT_AL_DX,    opOUT_AX_DX,
/*f0*/  opLOCK,         ILLEGAL,        opREPNE,        opREPE,         opHLT,          opCMC,          opF6_a16,       opF7_w_a16,     opCLC,          opSTC,          opCLI,          opSTI,          opCLD,          opSTD,          opINCDEC_b_a16, opFF_w_a16,

        /*32-bit data, 16-bit addr*/
/*      00              01              02              03              04              05              06              07              08              09              0a              0b              0c              0d              0e              0f*/        
/*00*/  opADD_b_rmw_a16,opADD_w_rmw_a16,opADD_b_rm_a16, opADD_w_rm_a16, opADD_AL_imm,   opADD_AX_imm,   opPUSH_ES_w,    opPOP_ES_w,     opOR_b_rmw_a16, opOR_w_rmw_a16, opOR_b_rm_a16,  opOR_w_rm_a16,  opOR_AL_imm,    opOR_AX_imm,    opPUSH_CS_w,    op0F_w_a16,
/*10*/  opADC_b_rmw_a16,opADC_w_rmw_a16,opADC_b_rm_a16, opADC_w_rm_a16, opADC_AL_imm,   opADC_AX_imm,   opPUSH_SS_w,    opPOP_SS_w,     opSBB_b_rmw_a16,opSBB_w_rmw_a16,opSBB_b_rm_a16, opSBB_w_rm_a16, opSBB_AL_imm,   opSBB_AX_imm,   opPUSH_DS_w,    opPOP_DS_w,
/*20*/  opAND_b_rmw_a16,opAND_w_rmw_a16,opAND_b_rm_a16, opAND_w_rm_a16, opAND_AL_imm,   opAND_AX_imm,   op_ES,          opDAA,          opSUB_b_rmw_a16,opSUB_w_rmw_a16,opSUB_b_rm_a16, opSUB_w_rm_a16, opSUB_AL_imm,   opSUB_AX_imm,   op_CS,          opDAS,
/*30*/  opXOR_b_rmw_a16,opXOR_w_rmw_a16,opXOR_b_rm_a16, opXOR_w_rm_a16, opXOR_AL_imm,   opXOR_AX_imm,   op_SS,          opAAA,          opCMP_b_rmw_a16,opCMP_w_rmw_a16,opCMP_b_rm_a16, opCMP_w_rm_a16, opCMP_AL_imm,   opCMP_AX_imm,   op_DS,          opAAS,

/*40*/  opINC_AX,       opINC_CX,       opINC_DX,       opINC_BX,       opINC_SP,       opINC_BP,       opINC_SI,       opINC_DI,       opDEC_AX,       opDEC_CX,       opDEC_DX,       opDEC_BX,       opDEC_SP,       opDEC_BP,       opDEC_SI,       opDEC_DI,  
/*50*/  opPUSH_AX,      opPUSH_CX,      opPUSH_DX,      opPUSH_BX,      opPUSH_SP,      opPUSH_BP,      opPUSH_SI,      opPUSH_DI,      opPOP_AX,       opPOP_CX,       opPOP_DX,       opPOP_BX,       opPOP_SP,       opPOP_BP,       opPOP_SI,       opPOP_DI, 
/*60*/  opPUSHA_w,      opPOPA_w,       opBOUND_w_a16,  opARPL_a16,     ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        opPUSH_imm_w,   opIMUL_w_iw_a16,opPUSH_imm_bw,  opIMUL_w_ib_a16,opINSB_a16,     opINSW_a16,     opOUTSB_a16,    opOUTSW_a16,
/*70*/  opJO,           opJNO,          opJB,           opJNB,          opJE,           opJNE,          opJBE,          opJNBE,         opJS,           opJNS,          opJP,           opJNP,          opJL,           opJNL,          opJLE,          opJNLE,

/*80*/  op80_a16,       op81_w_a16,     op80_a16,       op83_w_a16,     opTEST_b_a16,   opTEST_w_a16,   opXCHG_b_a16,   opXCHG_w_a16,   opMOV_b_r_a16,  opMOV_w_r_a16,  opMOV_r_b_a16,  opMOV_r_w_a16,  opMOV_w_seg_a16,opLEA_w_a16,    opMOV_seg_w_a16,opPOPW_a16,
/*90*/  opNOP,          opXCHG_AX_CX,   opXCHG_AX_DX,   opXCHG_AX_BX,   opXCHG_AX_SP,   opXCHG_AX_BP,   opXCHG_AX_SI,   opXCHG_AX_DI,   opCBW,          opCWD,          opCALL_far_w,   opWAIT,         opPUSHF,        opPOPF_286,     opSAHF,         opLAHF,
/*a0*/  opMOV_AL_a16,   opMOV_AX_a16,   opMOV_a16_AL,   opMOV_a16_AX,   opMOVSB_a16,    opMOVSW_a16,    opCMPSB_a16,    opCMPSW_a16,    opTEST_AL,      opTEST_AX,      opSTOSB_a16,    opSTOSW_a16,    opLODSB_a16,    opLODSW_a16,    opSCASB_a16,    opSCASW_a16,
/*b0*/  opMOV_AL_imm,   opMOV_CL_imm,   opMOV_DL_imm,   opMOV_BL_imm,   opMOV_AH_imm,   opMOV_CH_imm,   opMOV_DH_imm,   opMOV_BH_imm,   opMOV_AX_imm,   opMOV_CX_imm,   opMOV_DX_imm,   opMOV_BX_imm,   opMOV_SP_imm,   opMOV_BP_imm,   opMOV_SI_imm,   opMOV_DI_imm,

/*c0*/  opC0_a16,       opC1_w_a16,     opRET_w_imm,    opRET_w,        opLES_w_a16,    opLDS_w_a16,    opMOV_b_imm_a16,opMOV_w_imm_a16,opENTER_w,      opLEAVE_w,      opRETF_a16_imm, opRETF_a16,     opINT3,         opINT,          opINTO,         opIRET_286,
/*d0*/  opD0_a16,       opD1_w_a16,     opD2_a16,       opD3_w_a16,     opAAM,          opAAD,          opSETALC,       opXLAT_a16,     opESCAPE_d8_a16,opESCAPE_d9_a16,opESCAPE_da_a16,opESCAPE_db_a16,opESCAPE_dc_a16,opESCAPE_dd_a16,opESCAPE_de_a16,opESCAPE_df_a16,
/*e0*/  opLOOPNE_w,     opLOOPE_w,      opLOOP_w,       opJCXZ,         opIN_AL_imm,    opIN_AX_imm,    opOUT_AL_imm,   opOUT_AX_imm,   opCALL_r16,     opJMP_r16,      opJMP_far_a16,  opJMP_r8,       opIN_AL_DX,     opIN_AX_DX,     opOUT_AL_DX,    opOUT_AX_DX,
/*f0*/  opLOCK,         ILLEGAL,        opREPNE,        opREPE,         opHLT,          opCMC,          opF6_a16,       opF7_w_a16,     opCLC,          opSTC,          opCLI,          opSTI,          opCLD,          opSTD,          opINCDEC_b_a16, opFF_w_a16,

        /*16-bit data, 32-bit addr*/
/*      00              01              02              03              04              05              06              07              08              09              0a              0b              0c              0d              0e              0f*/        
/*00*/  opADD_b_rmw_a16,opADD_w_rmw_a16,opADD_b_rm_a16, opADD_w_rm_a16, opADD_AL_imm,   opADD_AX_imm,   opPUSH_ES_w,    opPOP_ES_w,     opOR_b_rmw_a16, opOR_w_rmw_a16, opOR_b_rm_a16,  opOR_w_rm_a16,  opOR_AL_imm,    opOR_AX_imm,    opPUSH_CS_w,    op0F_w_a16,
/*10*/  opADC_b_rmw_a16,opADC_w_rmw_a16,opADC_b_rm_a16, opADC_w_rm_a16, opADC_AL_imm,   opADC_AX_imm,   opPUSH_SS_w,    opPOP_SS_w,     opSBB_b_rmw_a16,opSBB_w_rmw_a16,opSBB_b_rm_a16, opSBB_w_rm_a16, opSBB_AL_imm,   opSBB_AX_imm,   opPUSH_DS_w,    opPOP_DS_w,
/*20*/  opAND_b_rmw_a16,opAND_w_rmw_a16,opAND_b_rm_a16, opAND_w_rm_a16, opAND_AL_imm,   opAND_AX_imm,   op_ES,          opDAA,          opSUB_b_rmw_a16,opSUB_w_rmw_a16,opSUB_b_rm_a16, opSUB_w_rm_a16, opSUB_AL_imm,   opSUB_AX_imm,   op_CS,          opDAS,
/*30*/  opXOR_b_rmw_a16,opXOR_w_rmw_a16,opXOR_b_rm_a16, opXOR_w_rm_a16, opXOR_AL_imm,   opXOR_AX_imm,   op_SS,          opAAA,          opCMP_b_rmw_a16,opCMP_w_rmw_a16,opCMP_b_rm_a16, opCMP_w_rm_a16, opCMP_AL_imm,   opCMP_AX_imm,   op_DS,          opAAS,

/*40*/  opINC_AX,       opINC_CX,       opINC_DX,       opINC_BX,       opINC_SP,       opINC_BP,       opINC_SI,       opINC_DI,       opDEC_AX,       opDEC_CX,       opDEC_DX,       opDEC_BX,       opDEC_SP,       opDEC_BP,       opDEC_SI,       opDEC_DI,  
/*50*/  opPUSH_AX,      opPUSH_CX,      opPUSH_DX,      opPUSH_BX,      opPUSH_SP,      opPUSH_BP,      opPUSH_SI,      opPUSH_DI,      opPOP_AX,       opPOP_CX,       opPOP_DX,       opPOP_BX,       opPOP_SP,       opPOP_BP,       opPOP_SI,       opPOP_DI, 
/*60*/  opPUSHA_w,      opPOPA_w,       opBOUND_w_a16,  opARPL_a16,     ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        opPUSH_imm_w,   opIMUL_w_iw_a16,opPUSH_imm_bw,  opIMUL_w_ib_a16,opINSB_a16,     opINSW_a16,     opOUTSB_a16,    opOUTSW_a16,
/*70*/  opJO,           opJNO,          opJB,           opJNB,          opJE,           opJNE,          opJBE,          opJNBE,         opJS,           opJNS,          opJP,           opJNP,          opJL,           opJNL,          opJLE,          opJNLE,

/*80*/  op80_a16,       op81_w_a16,     op80_a16,       op83_w_a16,     opTEST_b_a16,   opTEST_w_a16,   opXCHG_b_a16,   opXCHG_w_a16,   opMOV_b_r_a16,  opMOV_w_r_a16,  opMOV_r_b_a16,  opMOV_r_w_a16,  opMOV_w_seg_a16,opLEA_w_a16,    opMOV_seg_w_a16,opPOPW_a16,
/*90*/  opNOP,          opXCHG_AX_CX,   opXCHG_AX_DX,   opXCHG_AX_BX,   opXCHG_AX_SP,   opXCHG_AX_BP,   opXCHG_AX_SI,   opXCHG_AX_DI,   opCBW,          opCWD,          opCALL_far_w,   opWAIT,         opPUSHF,        opPOPF_286,     opSAHF,         opLAHF,
/*a0*/  opMOV_AL_a16,   opMOV_AX_a16,   opMOV_a16_AL,   opMOV_a16_AX,   opMOVSB_a16,    opMOVSW_a16,    opCMPSB_a16,    opCMPSW_a16,    opTEST_AL,      opTEST_AX,      opSTOSB_a16,    opSTOSW_a16,    opLODSB_a16,    opLODSW_a16,    opSCASB_a16,    opSCASW_a16,
/*b0*/  opMOV_AL_imm,   opMOV_CL_imm,   opMOV_DL_imm,   opMOV_BL_imm,   opMOV_AH_imm,   opMOV_CH_imm,   opMOV_DH_imm,   opMOV_BH_imm,   opMOV_AX_imm,   opMOV_CX_imm,   opMOV_DX_imm,   opMOV_BX_imm,   opMOV_SP_imm,   opMOV_BP_imm,   opMOV_SI_imm,   opMOV_DI_imm,

/*c0*/  opC0_a16,       opC1_w_a16,     opRET_w_imm,    opRET_w,        opLES_w_a16,    opLDS_w_a16,    opMOV_b_imm_a16,opMOV_w_imm_a16,opENTER_w,      opLEAVE_w,      opRETF_a16_imm, opRETF_a16,     opINT3,         opINT,          opINTO,         opIRET_286,
/*d0*/  opD0_a16,       opD1_w_a16,     opD2_a16,       opD3_w_a16,     opAAM,          opAAD,          opSETALC,       opXLAT_a16,     opESCAPE_d8_a16,opESCAPE_d9_a16,opESCAPE_da_a16,opESCAPE_db_a16,opESCAPE_dc_a16,opESCAPE_dd_a16,opESCAPE_de_a16,opESCAPE_df_a16,
/*e0*/  opLOOPNE_w,     opLOOPE_w,      opLOOP_w,       opJCXZ,         opIN_AL_imm,    opIN_AX_imm,    opOUT_AL_imm,   opOUT_AX_imm,   opCALL_r16,     opJMP_r16,      opJMP_far_a16,  opJMP_r8,       opIN_AL_DX,     opIN_AX_DX,     opOUT_AL_DX,    opOUT_AX_DX,
/*f0*/  opLOCK,         ILLEGAL,        opREPNE,        opREPE,         opHLT,          opCMC,          opF6_a16,       opF7_w_a16,     opCLC,          opSTC,          opCLI,          opSTI,          opCLD,          opSTD,          opINCDEC_b_a16, opFF_w_a16,

        /*32-bit data, 32-bit addr*/
/*      00              01              02              03              04              05              06              07              08              09              0a              0b              0c              0d              0e              0f*/        
/*00*/  opADD_b_rmw_a16,opADD_w_rmw_a16,opADD_b_rm_a16, opADD_w_rm_a16, opADD_AL_imm,   opADD_AX_imm,   opPUSH_ES_w,    opPOP_ES_w,     opOR_b_rmw_a16, opOR_w_rmw_a16, opOR_b_rm_a16,  opOR_w_rm_a16,  opOR_AL_imm,    opOR_AX_imm,    opPUSH_CS_w,    op0F_w_a16,
/*10*/  opADC_b_rmw_a16,opADC_w_rmw_a16,opADC_b_rm_a16, opADC_w_rm_a16, opADC_AL_imm,   opADC_AX_imm,   opPUSH_SS_w,    opPOP_SS_w,     opSBB_b_rmw_a16,opSBB_w_rmw_a16,opSBB_b_rm_a16, opSBB_w_rm_a16, opSBB_AL_imm,   opSBB_AX_imm,   opPUSH_DS_w,    opPOP_DS_w,
/*20*/  opAND_b_rmw_a16,opAND_w_rmw_a16,opAND_b_rm_a16, opAND_w_rm_a16, opAND_AL_imm,   opAND_AX_imm,   op_ES,          opDAA,          opSUB_b_rmw_a16,opSUB_w_rmw_a16,opSUB_b_rm_a16, opSUB_w_rm_a16, opSUB_AL_imm,   opSUB_AX_imm,   op_CS,          opDAS,
/*30*/  opXOR_b_rmw_a16,opXOR_w_rmw_a16,opXOR_b_rm_a16, opXOR_w_rm_a16, opXOR_AL_imm,   opXOR_AX_imm,   op_SS,          opAAA,          opCMP_b_rmw_a16,opCMP_w_rmw_a16,opCMP_b_rm_a16, opCMP_w_rm_a16, opCMP_AL_imm,   opCMP_AX_imm,   op_DS,          opAAS,

/*40*/  opINC_AX,       opINC_CX,       opINC_DX,       opINC_BX,       opINC_SP,       opINC_BP,       opINC_SI,       opINC_DI,       opDEC_AX,       opDEC_CX,       opDEC_DX,       opDEC_BX,       opDEC_SP,       opDEC_BP,       opDEC_SI,       opDEC_DI,  
/*50*/  opPUSH_AX,      opPUSH_CX,      opPUSH_DX,      opPUSH_BX,      opPUSH_SP,      opPUSH_BP,      opPUSH_SI,      opPUSH_DI,      opPOP_AX,       opPOP_CX,       opPOP_DX,       opPOP_BX,       opPOP_SP,       opPOP_BP,       opPOP_SI,       opPOP_DI, 
/*60*/  opPUSHA_w,      opPOPA_w,       opBOUND_w_a16,  opARPL_a16,     ILLEGAL,        ILLEGAL,        ILLEGAL,        ILLEGAL,        opPUSH_imm_w,   opIMUL_w_iw_a16,opPUSH_imm_bw,  opIMUL_w_ib_a16,opINSB_a16,     opINSW_a16,     opOUTSB_a16,    opOUTSW_a16,
/*70*/  opJO,           opJNO,          opJB,           opJNB,          opJE,           opJNE,          opJBE,          opJNBE,         opJS,           opJNS,          opJP,           opJNP,          opJL,           opJNL,          opJLE,          opJNLE,

/*80*/  op80_a16,       op81_w_a16,     op80_a16,       op83_w_a16,     opTEST_b_a16,   opTEST_w_a16,   opXCHG_b_a16,   opXCHG_w_a16,   opMOV_b_r_a16,  opMOV_w_r_a16,  opMOV_r_b_a16,  opMOV_r_w_a16,  opMOV_w_seg_a16,opLEA_w_a16,    opMOV_seg_w_a16,opPOPW_a16,
/*90*/  opNOP,          opXCHG_AX_CX,   opXCHG_AX_DX,   opXCHG_AX_BX,   opXCHG_AX_SP,   opXCHG_AX_BP,   opXCHG_AX_SI,   opXCHG_AX_DI,   opCBW,          opCWD,          opCALL_far_w,   opWAIT,         opPUSHF,        opPOPF_286,     opSAHF,         opLAHF,
/*a0*/  opMOV_AL_a16,   opMOV_AX_a16,   opMOV_a16_AL,   opMOV_a16_AX,   opMOVSB_a16,    opMOVSW_a16,    opCMPSB_a16,    opCMPSW_a16,    opTEST_AL,      opTEST_AX,      opSTOSB_a16,    opSTOSW_a16,    opLODSB_a16,    opLODSW_a16,    opSCASB_a16,    opSCASW_a16,
/*b0*/  opMOV_AL_imm,   opMOV_CL_imm,   opMOV_DL_imm,   opMOV_BL_imm,   opMOV_AH_imm,   opMOV_CH_imm,   opMOV_DH_imm,   opMOV_BH_imm,   opMOV_AX_imm,   opMOV_CX_imm,   opMOV_DX_imm,   opMOV_BX_imm,   opMOV_SP_imm,   opMOV_BP_imm,   opMOV_SI_imm,   opMOV_DI_imm,

/*c0*/  opC0_a16,       opC1_w_a16,     opRET_w_imm,    opRET_w,        opLES_w_a16,    opLDS_w_a16,    opMOV_b_imm_a16,opMOV_w_imm_a16,opENTER_w,      opLEAVE_w,      opRETF_a16_imm, opRETF_a16,     opINT3,         opINT,          opINTO,         opIRET_286,
/*d0*/  opD0_a16,       opD1_w_a16,     opD2_a16,       opD3_w_a16,     opAAM,          opAAD,          opSETALC,       opXLAT_a16,     opESCAPE_d8_a16,opESCAPE_d9_a16,opESCAPE_da_a16,opESCAPE_db_a16,opESCAPE_dc_a16,opESCAPE_dd_a16,opESCAPE_de_a16,opESCAPE_df_a16,
/*e0*/  opLOOPNE_w,     opLOOPE_w,      opLOOP_w,       opJCXZ,         opIN_AL_imm,    opIN_AX_imm,    opOUT_AL_imm,   opOUT_AX_imm,   opCALL_r16,     opJMP_r16,      opJMP_far_a16,  opJMP_r8,       opIN_AL_DX,     opIN_AX_DX,     opOUT_AL_DX,    opOUT_AX_DX,
/*f0*/  opLOCK,         ILLEGAL,        opREPNE,        opREPE,         opHLT,          opCMC,          opF6_a16,       opF7_w_a16,     opCLC,          opSTC,          opCLI,          opSTI,          opCLD,          opSTD,          opINCDEC_b_a16, opFF_w_a16,
};

OpFn ops_386[1024] = 
{
        /*16-bit data, 16-bit addr*/
/*      00              01              02              03              04              05              06              07              08              09              0a              0b              0c              0d              0e              0f*/        
/*00*/  opADD_b_rmw_a16,opADD_w_rmw_a16,opADD_b_rm_a16, opADD_w_rm_a16, opADD_AL_imm,   opADD_AX_imm,   opPUSH_ES_w,    opPOP_ES_w,     opOR_b_rmw_a16, opOR_w_rmw_a16, opOR_b_rm_a16,  opOR_w_rm_a16,  opOR_AL_imm,    opOR_AX_imm,    opPUSH_CS_w,    op0F_w_a16,
/*10*/  opADC_b_rmw_a16,opADC_w_rmw_a16,opADC_b_rm_a16, opADC_w_rm_a16, opADC_AL_imm,   opADC_AX_imm,   opPUSH_SS_w,    opPOP_SS_w,     opSBB_b_rmw_a16,opSBB_w_rmw_a16,opSBB_b_rm_a16, opSBB_w_rm_a16, opSBB_AL_imm,   opSBB_AX_imm,   opPUSH_DS_w,    opPOP_DS_w,
/*20*/  opAND_b_rmw_a16,opAND_w_rmw_a16,opAND_b_rm_a16, opAND_w_rm_a16, opAND_AL_imm,   opAND_AX_imm,   op_ES,          opDAA,          opSUB_b_rmw_a16,opSUB_w_rmw_a16,opSUB_b_rm_a16, opSUB_w_rm_a16, opSUB_AL_imm,   opSUB_AX_imm,   op_CS,          opDAS,
/*30*/  opXOR_b_rmw_a16,opXOR_w_rmw_a16,opXOR_b_rm_a16, opXOR_w_rm_a16, opXOR_AL_imm,   opXOR_AX_imm,   op_SS,          opAAA,          opCMP_b_rmw_a16,opCMP_w_rmw_a16,opCMP_b_rm_a16, opCMP_w_rm_a16, opCMP_AL_imm,   opCMP_AX_imm,   op_DS,          opAAS,

/*40*/  opINC_AX,       opINC_CX,       opINC_DX,       opINC_BX,       opINC_SP,       opINC_BP,       opINC_SI,       opINC_DI,       opDEC_AX,       opDEC_CX,       opDEC_DX,       opDEC_BX,       opDEC_SP,       opDEC_BP,       opDEC_SI,       opDEC_DI,  
/*50*/  opPUSH_AX,      opPUSH_CX,      opPUSH_DX,      opPUSH_BX,      opPUSH_SP,      opPUSH_BP,      opPUSH_SI,      opPUSH_DI,      opPOP_AX,       opPOP_CX,       opPOP_DX,       opPOP_BX,       opPOP_SP,       opPOP_BP,       opPOP_SI,       opPOP_DI, 
/*60*/  opPUSHA_w,      opPOPA_w,       opBOUND_w_a16,  opARPL_a16,     op_FS,          op_GS,          op_66,          op_67,          opPUSH_imm_w,   opIMUL_w_iw_a16,opPUSH_imm_bw,  opIMUL_w_ib_a16,opINSB_a16,     opINSW_a16,     opOUTSB_a16,    opOUTSW_a16,
/*70*/  opJO,           opJNO,          opJB,           opJNB,          opJE,           opJNE,          opJBE,          opJNBE,         opJS,           opJNS,          opJP,           opJNP,          opJL,           opJNL,          opJLE,          opJNLE,

/*80*/  op80_a16,       op81_w_a16,     op80_a16,       op83_w_a16,     opTEST_b_a16,   opTEST_w_a16,   opXCHG_b_a16,   opXCHG_w_a16,   opMOV_b_r_a16,  opMOV_w_r_a16,  opMOV_r_b_a16,  opMOV_r_w_a16,  opMOV_w_seg_a16,opLEA_w_a16,    opMOV_seg_w_a16,opPOPW_a16,
/*90*/  opNOP,          opXCHG_AX_CX,   opXCHG_AX_DX,   opXCHG_AX_BX,   opXCHG_AX_SP,   opXCHG_AX_BP,   opXCHG_AX_SI,   opXCHG_AX_DI,   opCBW,          opCWD,          opCALL_far_w,   opWAIT,         opPUSHF,        opPOPF,         opSAHF,         opLAHF,
/*a0*/  opMOV_AL_a16,   opMOV_AX_a16,   opMOV_a16_AL,   opMOV_a16_AX,   opMOVSB_a16,    opMOVSW_a16,    opCMPSB_a16,    opCMPSW_a16,    opTEST_AL,      opTEST_AX,      opSTOSB_a16,    opSTOSW_a16,    opLODSB_a16,    opLODSW_a16,    opSCASB_a16,    opSCASW_a16,
/*b0*/  opMOV_AL_imm,   opMOV_CL_imm,   opMOV_DL_imm,   opMOV_BL_imm,   opMOV_AH_imm,   opMOV_CH_imm,   opMOV_DH_imm,   opMOV_BH_imm,   opMOV_AX_imm,   opMOV_CX_imm,   opMOV_DX_imm,   opMOV_BX_imm,   opMOV_SP_imm,   opMOV_BP_imm,   opMOV_SI_imm,   opMOV_DI_imm,

/*c0*/  opC0_a16,       opC1_w_a16,     opRET_w_imm,    opRET_w,        opLES_w_a16,    opLDS_w_a16,    opMOV_b_imm_a16,opMOV_w_imm_a16,opENTER_w,      opLEAVE_w,      opRETF_a16_imm, opRETF_a16,     opINT3,         opINT,          opINTO,         opIRET,
/*d0*/  opD0_a16,       opD1_w_a16,     opD2_a16,       opD3_w_a16,     opAAM,          opAAD,          opSETALC,       opXLAT_a16,     opESCAPE_d8_a16,opESCAPE_d9_a16,opESCAPE_da_a16,opESCAPE_db_a16,opESCAPE_dc_a16,opESCAPE_dd_a16,opESCAPE_de_a16,opESCAPE_df_a16,
/*e0*/  opLOOPNE_w,     opLOOPE_w,      opLOOP_w,       opJCXZ,         opIN_AL_imm,    opIN_AX_imm,    opOUT_AL_imm,   opOUT_AX_imm,   opCALL_r16,     opJMP_r16,      opJMP_far_a16,  opJMP_r8,       opIN_AL_DX,     opIN_AX_DX,     opOUT_AL_DX,    opOUT_AX_DX,
/*f0*/  opLOCK,         ILLEGAL,        opREPNE,        opREPE,         opHLT,          opCMC,          opF6_a16,       opF7_w_a16,     opCLC,          opSTC,          opCLI,          opSTI,          opCLD,          opSTD,          opINCDEC_b_a16, opFF_w_a16,

        /*32-bit data, 16-bit addr*/
/*      00              01              02              03              04              05              06              07              08              09              0a              0b              0c              0d              0e              0f*/
/*00*/  opADD_b_rmw_a16,opADD_l_rmw_a16,opADD_b_rm_a16, opADD_l_rm_a16, opADD_AL_imm,   opADD_EAX_imm,  opPUSH_ES_l,    opPOP_ES_l,     opOR_b_rmw_a16, opOR_l_rmw_a16, opOR_b_rm_a16,  opOR_l_rm_a16,  opOR_AL_imm,    opOR_EAX_imm,   opPUSH_CS_l,    op0F_l_a16,
/*10*/  opADC_b_rmw_a16,opADC_l_rmw_a16,opADC_b_rm_a16, opADC_l_rm_a16, opADC_AL_imm,   opADC_EAX_imm,  opPUSH_SS_l,    opPOP_SS_l,     opSBB_b_rmw_a16,opSBB_l_rmw_a16,opSBB_b_rm_a16, opSBB_l_rm_a16, opSBB_AL_imm,   opSBB_EAX_imm,  opPUSH_DS_l,    opPOP_DS_l,
/*20*/  opAND_b_rmw_a16,opAND_l_rmw_a16,opAND_b_rm_a16, opAND_l_rm_a16, opAND_AL_imm,   opAND_EAX_imm,  op_ES,          opDAA,          opSUB_b_rmw_a16,opSUB_l_rmw_a16,opSUB_b_rm_a16, opSUB_l_rm_a16, opSUB_AL_imm,   opSUB_EAX_imm,  op_CS,          opDAS,
/*30*/  opXOR_b_rmw_a16,opXOR_l_rmw_a16,opXOR_b_rm_a16, opXOR_l_rm_a16, opXOR_AL_imm,   opXOR_EAX_imm,  op_SS,          opAAA,          opCMP_b_rmw_a16,opCMP_l_rmw_a16,opCMP_b_rm_a16, opCMP_l_rm_a16, opCMP_AL_imm,   opCMP_EAX_imm,  op_DS,          opAAS,

/*40*/  opINC_EAX,      opINC_ECX,      opINC_EDX,      opINC_EBX,      opINC_ESP,      opINC_EBP,      opINC_ESI,      opINC_EDI,      opDEC_EAX,      opDEC_ECX,      opDEC_EDX,      opDEC_EBX,      opDEC_ESP,      opDEC_EBP,      opDEC_ESI,      opDEC_EDI,
/*50*/  opPUSH_EAX,     opPUSH_ECX,     opPUSH_EDX,     opPUSH_EBX,     opPUSH_ESP,     opPUSH_EBP,     opPUSH_ESI,     opPUSH_EDI,     opPOP_EAX,      opPOP_ECX,      opPOP_EDX,      opPOP_EBX,      opPOP_ESP,      opPOP_EBP,      opPOP_ESI,      opPOP_EDI, 
/*60*/  opPUSHA_l,      opPOPA_l,       opBOUND_l_a16,  opARPL_a16,     op_FS,          op_GS,          op_66,          op_67,          opPUSH_imm_l,   opIMUL_l_il_a16,opPUSH_imm_bl,  opIMUL_l_ib_a16,opINSB_a16,     opINSL_a16,     opOUTSB_a16,    opOUTSL_a16,
/*70*/  opJO,           opJNO,          opJB,           opJNB,          opJE,           opJNE,          opJBE,          opJNBE,         opJS,           opJNS,          opJP,           opJNP,          opJL,           opJNL,          opJLE,          opJNLE,

/*80*/  op80_a16,       op81_l_a16,     op80_a16,       op83_l_a16,     opTEST_b_a16,   opTEST_l_a16,   opXCHG_b_a16,   opXCHG_l_a16,   opMOV_b_r_a16,  opMOV_l_r_a16,  opMOV_r_b_a16,  opMOV_r_l_a16,  opMOV_l_seg_a16,opLEA_l_a16,    opMOV_seg_w_a16,opPOPL_a16,
/*90*/  opNOP,          opXCHG_EAX_ECX, opXCHG_EAX_EDX, opXCHG_EAX_EBX, opXCHG_EAX_ESP, opXCHG_EAX_EBP, opXCHG_EAX_ESI, opXCHG_EAX_EDI, opCWDE,         opCDQ,          opCALL_far_l,   opWAIT,         opPUSHFD,       opPOPFD,        opSAHF,         opLAHF,
/*a0*/  opMOV_AL_a16,   opMOV_EAX_a16,  opMOV_a16_AL,   opMOV_a16_EAX,  opMOVSB_a16,    opMOVSL_a16,    opCMPSB_a16,    opCMPSL_a16,    opTEST_AL,      opTEST_EAX,     opSTOSB_a16,    opSTOSL_a16,    opLODSB_a16,    opLODSL_a16,    opSCASB_a16,    opSCASL_a16,
/*b0*/  opMOV_AL_imm,   opMOV_CL_imm,   opMOV_DL_imm,   opMOV_BL_imm,   opMOV_AH_imm,   opMOV_CH_imm,   opMOV_DH_imm,   opMOV_BH_imm,   opMOV_EAX_imm,  opMOV_ECX_imm,  opMOV_EDX_imm,  opMOV_EBX_imm,  opMOV_ESP_imm,  opMOV_EBP_imm,  opMOV_ESI_imm,  opMOV_EDI_imm,

/*c0*/  opC0_a16,       opC1_l_a16,     opRET_l_imm,    opRET_l,        opLES_l_a16,    opLDS_l_a16,    opMOV_b_imm_a16,opMOV_l_imm_a16,opENTER_l,      opLEAVE_l,      opRETF_a32_imm, opRETF_a32,     opINT3,         opINT,          opINTO,         opIRETD,
/*d0*/  opD0_a16,       opD1_l_a16,     opD2_a16,       opD3_l_a16,     opAAM,          opAAD,          opSETALC,       opXLAT_a16,     opESCAPE_d8_a16,opESCAPE_d9_a16,opESCAPE_da_a16,opESCAPE_db_a16,opESCAPE_dc_a16,opESCAPE_dd_a16,opESCAPE_de_a16,opESCAPE_df_a16,
/*e0*/  opLOOPNE_w,     opLOOPE_w,      opLOOP_w,       opJCXZ,         opIN_AL_imm,    opIN_EAX_imm,   opOUT_AL_imm,   opOUT_EAX_imm,  opCALL_r32,     opJMP_r32,      opJMP_far_a32,  opJMP_r8,       opIN_AL_DX,     opIN_EAX_DX,    opOUT_AL_DX,    opOUT_EAX_DX,
/*f0*/  opLOCK,         ILLEGAL,        opREPNE,        opREPE,         opHLT,          opCMC,          opF6_a16,       opF7_l_a16,     opCLC,          opSTC,          opCLI,          opSTI,          opCLD,          opSTD,          opINCDEC_b_a16, opFF_l_a16,

        /*16-bit data, 32-bit addr*/
/*      00              01              02              03              04              05              06              07              08              09              0a              0b              0c              0d              0e              0f*/
/*00*/  opADD_b_rmw_a32,opADD_w_rmw_a32,opADD_b_rm_a32, opADD_w_rm_a32, opADD_AL_imm,   opADD_AX_imm,   opPUSH_ES_w,    opPOP_ES_w,     opOR_b_rmw_a32, opOR_w_rmw_a32, opOR_b_rm_a32,  opOR_w_rm_a32,  opOR_AL_imm,    opOR_AX_imm,    opPUSH_CS_w,    op0F_w_a32,
/*10*/  opADC_b_rmw_a32,opADC_w_rmw_a32,opADC_b_rm_a32, opADC_w_rm_a32, opADC_AL_imm,   opADC_AX_imm,   opPUSH_SS_w,    opPOP_SS_w,     opSBB_b_rmw_a32,opSBB_w_rmw_a32,opSBB_b_rm_a32, opSBB_w_rm_a32, opSBB_AL_imm,   opSBB_AX_imm,   opPUSH_DS_w,    opPOP_DS_w,
/*20*/  opAND_b_rmw_a32,opAND_w_rmw_a32,opAND_b_rm_a32, opAND_w_rm_a32, opAND_AL_imm,   opAND_AX_imm,   op_ES,          opDAA,          opSUB_b_rmw_a32,opSUB_w_rmw_a32,opSUB_b_rm_a32, opSUB_w_rm_a32, opSUB_AL_imm,   opSUB_AX_imm,   op_CS,          opDAS,
/*30*/  opXOR_b_rmw_a32,opXOR_w_rmw_a32,opXOR_b_rm_a32, opXOR_w_rm_a32, opXOR_AL_imm,   opXOR_AX_imm,   op_SS,          opAAA,          opCMP_b_rmw_a32,opCMP_w_rmw_a32,opCMP_b_rm_a32, opCMP_w_rm_a32, opCMP_AL_imm,   opCMP_AX_imm,   op_DS,          opAAS,

/*40*/  opINC_AX,       opINC_CX,       opINC_DX,       opINC_BX,       opINC_SP,       opINC_BP,       opINC_SI,       opINC_DI,       opDEC_AX,       opDEC_CX,       opDEC_DX,       opDEC_BX,       opDEC_SP,       opDEC_BP,       opDEC_SI,       opDEC_DI,  
/*50*/  opPUSH_AX,      opPUSH_CX,      opPUSH_DX,      opPUSH_BX,      opPUSH_SP,      opPUSH_BP,      opPUSH_SI,      opPUSH_DI,      opPOP_AX,       opPOP_CX,       opPOP_DX,       opPOP_BX,       opPOP_SP,       opPOP_BP,       opPOP_SI,       opPOP_DI, 
/*60*/  opPUSHA_w,      opPOPA_w,       opBOUND_w_a32,  opARPL_a32,     op_FS,          op_GS,          op_66,          op_67,          opPUSH_imm_w,   opIMUL_w_iw_a32,opPUSH_imm_bw,  opIMUL_w_ib_a32,opINSB_a32,     opINSW_a32,     opOUTSB_a32,    opOUTSW_a32,
/*70*/  opJO,           opJNO,          opJB,           opJNB,          opJE,           opJNE,          opJBE,          opJNBE,         opJS,           opJNS,          opJP,           opJNP,          opJL,           opJNL,          opJLE,          opJNLE,

/*80*/  op80_a32,       op81_w_a32,     op80_a32,       op83_w_a32,     opTEST_b_a32,   opTEST_w_a32,   opXCHG_b_a32,   opXCHG_w_a32,   opMOV_b_r_a32,  opMOV_w_r_a32,  opMOV_r_b_a32,  opMOV_r_w_a32,  opMOV_w_seg_a32,opLEA_w_a32,    opMOV_seg_w_a32,opPOPW_a32,
/*90*/  opNOP,          opXCHG_AX_CX,   opXCHG_AX_DX,   opXCHG_AX_BX,   opXCHG_AX_SP,   opXCHG_AX_BP,   opXCHG_AX_SI,   opXCHG_AX_DI,   opCBW,          opCWD,          opCALL_far_w,   opWAIT,         opPUSHF,        opPOPF,         opSAHF,         opLAHF,
/*a0*/  opMOV_AL_a32,   opMOV_AX_a32,   opMOV_a32_AL,   opMOV_a32_AX,   opMOVSB_a32,    opMOVSW_a32,    opCMPSB_a32,    opCMPSW_a32,    opTEST_AL,      opTEST_AX,      opSTOSB_a32,    opSTOSW_a32,    opLODSB_a32,    opLODSW_a32,    opSCASB_a32,    opSCASW_a32,
/*b0*/  opMOV_AL_imm,   opMOV_CL_imm,   opMOV_DL_imm,   opMOV_BL_imm,   opMOV_AH_imm,   opMOV_CH_imm,   opMOV_DH_imm,   opMOV_BH_imm,   opMOV_AX_imm,   opMOV_CX_imm,   opMOV_DX_imm,   opMOV_BX_imm,   opMOV_SP_imm,   opMOV_BP_imm,   opMOV_SI_imm,   opMOV_DI_imm,

/*c0*/  opC0_a32,       opC1_w_a32,     opRET_w_imm,    opRET_w,        opLES_w_a32,    opLDS_w_a32,    opMOV_b_imm_a32,opMOV_w_imm_a32,opENTER_w,      opLEAVE_w,      opRETF_a16_imm, opRETF_a16,     opINT3,         opINT,          opINTO,         opIRET,
/*d0*/  opD0_a32,       opD1_w_a32,     opD2_a32,       opD3_w_a32,     opAAM,          opAAD,          opSETALC,       opXLAT_a32,     opESCAPE_d8_a32,opESCAPE_d9_a32,opESCAPE_da_a32,opESCAPE_db_a32,opESCAPE_dc_a32,opESCAPE_dd_a32,opESCAPE_de_a32,opESCAPE_df_a32,
/*e0*/  opLOOPNE_l,     opLOOPE_l,      opLOOP_l,       opJECXZ,        opIN_AL_imm,    opIN_AX_imm,    opOUT_AL_imm,   opOUT_AX_imm,   opCALL_r16,     opJMP_r16,      opJMP_far_a16,  opJMP_r8,       opIN_AL_DX,     opIN_AX_DX,     opOUT_AL_DX,    opOUT_AX_DX,
/*f0*/  opLOCK,         ILLEGAL,        opREPNE,        opREPE,         opHLT,          opCMC,          opF6_a32,       opF7_w_a32,     opCLC,          opSTC,          opCLI,          opSTI,          opCLD,          opSTD,          opINCDEC_b_a32, opFF_w_a32,

        /*32-bit data, 32-bit addr*/
/*      00              01              02              03              04              05              06              07              08              09              0a              0b              0c              0d              0e              0f*/
/*00*/  opADD_b_rmw_a32,opADD_l_rmw_a32,opADD_b_rm_a32, opADD_l_rm_a32, opADD_AL_imm,   opADD_EAX_imm,  opPUSH_ES_l,    opPOP_ES_l,     opOR_b_rmw_a32, opOR_l_rmw_a32, opOR_b_rm_a32,  opOR_l_rm_a32,  opOR_AL_imm,    opOR_EAX_imm,   opPUSH_CS_l,    op0F_l_a32,
/*10*/  opADC_b_rmw_a32,opADC_l_rmw_a32,opADC_b_rm_a32, opADC_l_rm_a32, opADC_AL_imm,   opADC_EAX_imm,  opPUSH_SS_l,    opPOP_SS_l,     opSBB_b_rmw_a32,opSBB_l_rmw_a32,opSBB_b_rm_a32, opSBB_l_rm_a32, opSBB_AL_imm,   opSBB_EAX_imm,  opPUSH_DS_l,    opPOP_DS_l,
/*20*/  opAND_b_rmw_a32,opAND_l_rmw_a32,opAND_b_rm_a32, opAND_l_rm_a32, opAND_AL_imm,   opAND_EAX_imm,  op_ES,          opDAA,          opSUB_b_rmw_a32,opSUB_l_rmw_a32,opSUB_b_rm_a32, opSUB_l_rm_a32, opSUB_AL_imm,   opSUB_EAX_imm,  op_CS,          opDAS,
/*30*/  opXOR_b_rmw_a32,opXOR_l_rmw_a32,opXOR_b_rm_a32, opXOR_l_rm_a32, opXOR_AL_imm,   opXOR_EAX_imm,  op_SS,          opAAA,          opCMP_b_rmw_a32,opCMP_l_rmw_a32,opCMP_b_rm_a32, opCMP_l_rm_a32, opCMP_AL_imm,   opCMP_EAX_imm,  op_DS,          opAAS,

/*40*/  opINC_EAX,      opINC_ECX,      opINC_EDX,      opINC_EBX,      opINC_ESP,      opINC_EBP,      opINC_ESI,      opINC_EDI,      opDEC_EAX,      opDEC_ECX,      opDEC_EDX,      opDEC_EBX,      opDEC_ESP,      opDEC_EBP,      opDEC_ESI,      opDEC_EDI,
/*50*/  opPUSH_EAX,     opPUSH_ECX,     opPUSH_EDX,     opPUSH_EBX,     opPUSH_ESP,     opPUSH_EBP,     opPUSH_ESI,     opPUSH_EDI,     opPOP_EAX,      opPOP_ECX,      opPOP_EDX,      opPOP_EBX,      opPOP_ESP,      opPOP_EBP,      opPOP_ESI,      opPOP_EDI, 
/*60*/  opPUSHA_l,      opPOPA_l,       opBOUND_l_a32,  opARPL_a32,     op_FS,          op_GS,          op_66,          op_67,          opPUSH_imm_l,   opIMUL_l_il_a32,opPUSH_imm_bl,  opIMUL_l_ib_a32,opINSB_a32,     opINSL_a32,     opOUTSB_a32,    opOUTSL_a32,
/*70*/  opJO,           opJNO,          opJB,           opJNB,          opJE,           opJNE,          opJBE,          opJNBE,         opJS,           opJNS,          opJP,           opJNP,          opJL,           opJNL,          opJLE,          opJNLE,

/*80*/  op80_a32,       op81_l_a32,     op80_a32,       op83_l_a32,     opTEST_b_a32,   opTEST_l_a32,   opXCHG_b_a32,   opXCHG_l_a32,   opMOV_b_r_a32,  opMOV_l_r_a32,  opMOV_r_b_a32,  opMOV_r_l_a32,  opMOV_l_seg_a32,opLEA_l_a32,    opMOV_seg_w_a32,opPOPL_a32,
/*90*/  opNOP,          opXCHG_EAX_ECX, opXCHG_EAX_EDX, opXCHG_EAX_EBX, opXCHG_EAX_ESP, opXCHG_EAX_EBP, opXCHG_EAX_ESI, opXCHG_EAX_EDI, opCWDE,         opCDQ,          opCALL_far_l,   opWAIT,         opPUSHFD,       opPOPFD,        opSAHF,         opLAHF,
/*a0*/  opMOV_AL_a32,   opMOV_EAX_a32,  opMOV_a32_AL,   opMOV_a32_EAX,  opMOVSB_a32,    opMOVSL_a32,    opCMPSB_a32,    opCMPSL_a32,    opTEST_AL,      opTEST_EAX,     opSTOSB_a32,    opSTOSL_a32,    opLODSB_a32,    opLODSL_a32,    opSCASB_a32,    opSCASL_a32,
/*b0*/  opMOV_AL_imm,   opMOV_CL_imm,   opMOV_DL_imm,   opMOV_BL_imm,   opMOV_AH_imm,   opMOV_CH_imm,   opMOV_DH_imm,   opMOV_BH_imm,   opMOV_EAX_imm,  opMOV_ECX_imm,  opMOV_EDX_imm,  opMOV_EBX_imm,  opMOV_ESP_imm,  opMOV_EBP_imm,  opMOV_ESI_imm,  opMOV_EDI_imm,

/*c0*/  opC0_a32,       opC1_l_a32,     opRET_l_imm,    opRET_l,        opLES_l_a32,    opLDS_l_a32,    opMOV_b_imm_a32,opMOV_l_imm_a32,opENTER_l,      opLEAVE_l,      opRETF_a32_imm, opRETF_a32,     opINT3,         opINT,          opINTO,         opIRETD,
/*d0*/  opD0_a32,       opD1_l_a32,     opD2_a32,       opD3_l_a32,     opAAM,          opAAD,          opSETALC,       opXLAT_a32,     opESCAPE_d8_a32,opESCAPE_d9_a32,opESCAPE_da_a32,opESCAPE_db_a32,opESCAPE_dc_a32,opESCAPE_dd_a32,opESCAPE_de_a32,opESCAPE_df_a32,
/*e0*/  opLOOPNE_l,     opLOOPE_l,      opLOOP_l,       opJECXZ,        opIN_AL_imm,    opIN_EAX_imm,   opOUT_AL_imm,   opOUT_EAX_imm,  opCALL_r32,     opJMP_r32,      opJMP_far_a32,  opJMP_r8,       opIN_AL_DX,     opIN_EAX_DX,    opOUT_AL_DX,    opOUT_EAX_DX,
/*f0*/  opLOCK,         ILLEGAL,        opREPNE,        opREPE,         opHLT,          opCMC,          opF6_a32,       opF7_l_a32,     opCLC,          opSTC,          opCLI,          opSTI,          opCLD,          opSTD,          opINCDEC_b_a32, opFF_l_a32,
};