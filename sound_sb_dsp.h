typedef struct sb_dsp_t
{
        int sb_type;

        int sb_8_length,  sb_8_format,  sb_8_autoinit,  sb_8_pause,  sb_8_enable,  sb_8_autolen,  sb_8_output;
        int sb_8_dmanum;
        int sb_16_length, sb_16_format, sb_16_autoinit, sb_16_pause, sb_16_enable, sb_16_autolen, sb_16_output;
        int sb_pausetime;

        uint8_t sb_read_data[256];
        int sb_read_wp, sb_read_rp;
        int sb_speaker;

        int sb_data_stat;

        int sb_irqnum;

        uint8_t sbe2;
        int sbe2count;

        uint8_t sb_data[8];

        int sb_freq;
        
        int writebusy; /*Needed for Amnesia*/

        int16_t sbdat;
        int sbdat2;
        int16_t sbdatl, sbdatr;

        uint8_t sbref;
        int8_t sbstep;

        int sbdacpos;

        int sbleftright;

        int sbreset;
        uint8_t sbreaddat;
        uint8_t sb_command;
        uint8_t sb_test;
        int sb_timei, sb_timeo;

        int sb_irq8, sb_irq16;

        uint8_t sb_asp_regs[256];
        
        int sbenable, sb_enable_i;
        
        int sbcount, sb_count_i;
        
        int sblatcho, sblatchi;
        
        uint16_t sb_addr;
        
        int stereo;
} sb_dsp_t;

void sb_dsp_init(sb_dsp_t *dsp, int type);

void sb_dsp_setirq(sb_dsp_t *dsp, int irq);
void sb_dsp_setdma8(sb_dsp_t *dsp, int dma);
void sb_dsp_setaddr(sb_dsp_t *dsp, uint16_t addr);

void sb_dsp_speed_changed(sb_dsp_t *dsp);

void sb_dsp_poll(sb_dsp_t *dsp, int16_t *l, int16_t *r);

void sb_dsp_set_stereo(sb_dsp_t *dsp, int stereo);

int sb_dsp_add_status_info(char *s, int max_len, sb_dsp_t *dsp);
