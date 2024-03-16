#ifndef GUS_H
#define GUS_H

#include <cstdint>

/* Read a value from GUS memory */
uint8_t GUSPeek(uint32_t loc);

/* Write a value into GUS memory */
void GUSPoke(uint32_t loc, uint8_t b);

/* Returns TRUE if there is a GUS at I/O address BASE */
bool GUSProbe(void);

/* Search all possible I/O addresses for the GUS */
uint32_t GUSFind(void);

/* Returns how much RAM is available on the GUS */
uint32_t GUSFindMem(void);

/* An incomplete routine to initialise the GUS for output. */
void GUSReset(uint8_t voices = 14);

/* Set the volume of channel voi to vol, a 16bit logarithmic scale
   volume value -  0 is off, $ffff is full volume, $e0000 is half volume, etc */
void GUSSetVolume(uint8_t voi, uint16_t vol);

/* set the pan position (from 0 to 15, 0 being left, 15 right and 7 middle) */
void GUSSetBalance(uint8_t v, uint8_t b);

/* frequency at which the sample should be played */
void GUSSetFreq(uint8_t v, uint32_t f);

/* This routine tells the GUS to play a sample commencing at vBegin,
   starting at location vStart, and stopping at vEnd */
void GUSPlayVoice(uint8_t v, uint8_t mode, uint32_t vBegin, uint32_t vStart, uint32_t vEnd);

/* This routine will return the memory location that the channel v is currently playing. If the GUS has reached the end
 * of the sample, then the returned value will be vEnd. If you want to see what BYTE value is currently being played
 * (for visual output of the sample's waveform), then you simply PEEK the location pointed to by this routine. */
uint32_t GUSVoicePos(uint8_t v);

/* stop the playback on channel v */
/*void GUSStopVoice(uint8_t v);*/

void GUSVoiceControl(uint8_t v, uint8_t b);

#define GUS_VOICE_STOPPED 1
#define GUS_VOICE_STOP 2
#define GUS_VOICE_16BIT 4
#define GUS_LOOP_ENABLE 8
#define GUS_BIDIRECTIONAL_LOOP 16
#define GUS_WAVETABLE_IRQ 32
#define GUS_DIRECTION 64
#define GUS_IRQ_PENDING 128

#endif // GUS_H
