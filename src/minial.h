#ifndef MINIAL_H
#define MINIAL_H

#include <cstdint>

struct ALCcontext;
struct ALCdevice;

typedef unsigned int ALuint;
typedef char ALCboolean;
typedef char ALCchar;
typedef int ALCenum;
typedef int ALCint;

#define ALC_ALL_DEVICES_SPECIFIER                0x1013
#define ALC_DEVICE_SPECIFIER                     0x1005

ALCboolean alcIsExtensionPresent(ALCdevice *device, const ALCchar *extname);
const ALCchar* alcGetString(ALCdevice *device, ALCenum param);
ALCdevice* alcOpenDevice(const ALCchar *devicename);
ALCcontext* alcCreateContext(ALCdevice *device, const ALCint *attrlist);
ALCboolean  alcMakeContextCurrent(ALCcontext *context);
void alcDestroyContext(ALCcontext *context);
ALCboolean alcCloseDevice(ALCdevice *device);

typedef float ALfloat;
typedef int ALenum;
typedef int ALsizei;
typedef unsigned int ALuint;
typedef void ALvoid;
typedef int ALint;

#define AL_POSITION                              0x1004
#define AL_VELOCITY                              0x1006
#define AL_ORIENTATION                           0x100F
#define AL_FORMAT_MONO16                         0x1101
#define AL_PITCH                                 0x1003
#define AL_GAIN                                  0x100A
#define AL_LOOPING                               0x1007
#define AL_BUFFER                                0x1009
#define AL_SAMPLE_OFFSET                         0x1025

void alListenerfv(ALenum param, const ALfloat *values);
void alDeleteSources(ALsizei n, const ALuint *sources);
void alDeleteBuffers(ALsizei n, const ALuint *buffers);
void alGenBuffers(ALsizei n, ALuint *buffers);
void alBufferData(ALuint buffer, ALenum format, const ALvoid *data, ALsizei size, ALsizei freq);
void alGenSources(ALsizei n, ALuint *sources);
void alSourcef(ALuint source, ALenum param, ALfloat value);
void alSourcefv(ALuint source, ALenum param, const ALfloat *values);
void alSourcei(ALuint source, ALenum param, ALint value);
void alSourcePlay(ALuint source);
void alSourceStop(ALuint source);
void alSourceRewind(ALuint source);

#endif // MINIAL_H
