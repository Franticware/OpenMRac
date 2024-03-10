#ifndef _SB_H_
#define _SB_H_

#include <dpmi.h>
#include <go32.h>

/*
 * Offsets relative to base I/O address.
 */
#define SB_LEFT_FM_STATUS   0x00 /* Pro only */
#define SB_LEFT_FM_ADDRESS  0x00 /* Pro only */
#define SB_LEFT_FM_DATA     0x01 /* Pro only */
#define SB_RIGHT_FM_STATUS  0x02 /* Pro only */
#define SB_RIGHT_FM_ADDRESS 0x02 /* Pro only */
#define SB_RIGHT_FM_DATA    0x03 /* Pro only */
#define SB_MIXER_ADDRESS    0x04 /* Pro only */
#define SB_MIXER_DATA       0x05 /* Pro only */
#define SB_DSP_RESET        0x06
#define SB_FM_STATUS        0x08
#define SB_FM_ADDRESS       0x08
#define SB_FM_DATA          0x09
#define SB_DSP_READ_DATA    0x0A
#define SB_DSP_WRITE_DATA   0x0C
#define SB_DSP_WRITE_STATUS 0x0C
#define SB_DSP_DATA_AVAIL   0x0E
#define SB_CD_ROM_DATA      0x10 /* Pro only */
#define SB_CD_ROM_STATUS    0x11 /* Pro only */
#define SB_CD_ROM_RESET     0x12 /* Pro only */
#define SB_CD_ROM_ENABLE    0x13 /* Pro only */

#define SB_ADLIB_FM_STATUS  0x388
#define SB_ADLIB_FM_ADDRESS 0x388
#define SB_ADLIB_FM_DATA    0x389

/* Defines for 8237 DMA Controller IO addresses */
#define SB_DMA         0
#define SB_CH0_BASE    (SB_DMA + 0)
#define SB_CH0_COUNT   (SB_DMA + 1)
#define SB_CH1_BASE    (SB_DMA + 2)
#define SB_CH1_COUNT   (SB_DMA + 3)
#define SB_CH2_BASE    (SB_DMA + 4)
#define SB_CH2_COUNT   (SB_DMA + 5)
#define SB_CH3_BASE    (SB_DMA + 6)
#define SB_CH3_COUNT   (SB_DMA + 7)
#define SB_DMA_STATUS  (SB_DMA + 8)
#define SB_DMA_CMD     (SB_DMA + 8)
#define SB_DMA_REQUEST (SB_DMA + 9)
#define SB_DMA_MASK    (SB_DMA + 10)
#define SB_DMA_MODE    (SB_DMA + 11)
#define SB_DMA_FF      (SB_DMA + 12)
#define SB_DMA_TMP     (SB_DMA + 13)
#define SB_DMA_CLEAR   (SB_DMA + 13)
#define SB_DMA_CLRMSK  (SB_DMA + 14)
#define SB_DMA_WRMSK   (SB_DMA + 15)
#define SB_DMAPAGE     0x80

/* Types of Soundblaster Cards */
#define SB_TYPE_15  1
#define SB_TYPE_PRO 2
#define SB_TYPE_20  3

/* DSP Commands */
#define SB_DIRECT_8_BIT_DAC   0x10
#define SB_DMA_8_BIT_DAC      0x14
#define SB_DMA_2_BIT_DAC      0x16
#define SB_DMA_2_BIT_REF_DAC  0x17
#define SB_DIRECT_ADC         0x20
#define SB_DMA_ADC            0x24
#define SB_MIDI_READ_POLL     0x30
#define SB_MIDI_READ_IRQ      0x31
#define SB_MIDI_WRITE_POLL    0x38
#define SB_TIME_CONSTANT      0x40
#define SB_DMA_4_BIT_DAC      0x74
#define SB_DMA_4_BIT_REF_DAC  0x75
#define SB_DMA_26_BIT_DAC     0x76
#define SB_DMA_26_BIT_REF_DAC 0x77
#define SB_HALT_DMA           0xD0
#define SB_CONTINUE_DMA       0xD4
#define SB_SPEAKER_ON         0xD1
#define SB_SPEAKER_OFF        0xD3
#define SB_DSP_ID             0xE0
#define SB_DSP_VER            0xE1
#define SB_MDAC1              0x61
#define SB_MDAC2              0x62
#define SB_MDAC3              0x63
#define SB_MDAC4              0x64
#define SB_MDAC5              0x65
#define SB_MDAC6              0x66
#define SB_MDAC7              0x67

#define SB_SET_BLOCKSIZE      0x48
#define SB_HIGH_DMA_8_BIT_DAC 0x91
#define SB_HIGH_DMA_8_BIT_ADC 0x99


/* DMA chunk size, in bytes.
 * This parameter determines how big our DMA buffers are.  We play
 * the sample by piecing together chunks that are this big.  This
 * means that we don't have to copy the entire sample down into
 * conventional memory before playing it.  (A nice side effect of
 * this is that we can play samples that are longer than 64K.)
 *
 * Setting this is tricky.  If it's too small, we'll get lots
 * of interrupts, and slower machines might not be able to keep
 * up.  Furthermore, the smaller this is, the more grainy the
 * sound will come out.
 *
 * On the other hand, if we make it too big there will be a noticeable
 * delay between a call to sb_play and when the sound actually starts
 * playing, which is unacceptable for things like games where sound
 * effects should be "instantaneous".
 *
 */
//#define DMA_CHUNK (1024)
#define DMA_CHUNK (512)


/* Card parameters */
extern unsigned int sb_ioaddr;
extern unsigned int sb_irq;
extern unsigned int sb_dmachan;
extern unsigned int sb_type;

#define sb_writedac(x)                                          \
    {                                                           \
        while (inportb(sb_ioaddr + SB_DSP_WRITE_STATUS) & 0x80) \
            ;                                                   \
        outportb(sb_ioaddr + SB_DSP_WRITE_DATA, (x));           \
    }

#define sb_writemixer(x, y)                          \
    {                                                \
        outportb(sb_ioaddr + SB_MIXER_ADDRESS, (x)); \
        outportb(sb_ioaddr + SB_MIXER_DATA, (y));    \
    }

void sb_intr_play(_go32_dpmi_registers* reg);
void sb_fill_buffer(register unsigned n);
void sb_play_buffer(register unsigned n);
void sb_play(unsigned char* data, unsigned long length);

void sb_set_sample_rate(unsigned int rate);
void sb_voice(int state);
void sb_getparams();
int sb_initcard();
int sb_init_buffers();
int sb_init();
int sb_read_dac(int Base);

void sb_install_interrupts(void (*sb_intr)(_go32_dpmi_registers*));
void sb_install_rm_interrupt(void (*sb_intr)(_go32_dpmi_registers*));
void sb_cleanup_rm_interrupt();
void sb_install_pm_interrupt(void (*sb_intr)(_go32_dpmi_registers*));
void sb_cleanup_pm_interrupt();
void sb_cleanup_ints();
int sb_cleanup();

int sb_read_counter(void);
void sb_reset(void);
void kbclear(void);

void StreamStart(int Rate);
unsigned char* StreamBuf(size_t* len);
void StreamReady();
void StreamStop();

#endif
