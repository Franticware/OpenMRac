/* Copyright (c) 2022, Vojtěch Salajka. All rights reserved. Use of this source code is governed by a BSD-style license that can be found in the LICENSE file. */

#ifndef MINIAL_H
#define MINIAL_H

#include <cstdint>

struct ALCcontext;
struct ALCdevice;

typedef char ALCboolean;
typedef char ALCchar;
typedef int ALCenum;
typedef int ALCint;
typedef unsigned int ALuint;

#define ALC_DEVICE_SPECIFIER                     0x1005
#define ALC_ALL_DEVICES_SPECIFIER                0x1013

ALCboolean alcCloseDevice(ALCdevice *device);
ALCcontext* alcCreateContext(ALCdevice *device, const ALCint *attrlist);
void alcDestroyContext(ALCcontext *context);
const ALCchar* alcGetString(ALCdevice *device, ALCenum param);
ALCboolean alcIsExtensionPresent(ALCdevice *device, const ALCchar *extname);
ALCboolean  alcMakeContextCurrent(ALCcontext *context);
ALCdevice* alcOpenDevice(const ALCchar *devicename);

typedef int ALenum;
typedef float ALfloat;
typedef int ALint;
typedef int ALsizei;
typedef unsigned int ALuint;
typedef void ALvoid;

#define AL_PITCH                                 0x1003
#define AL_POSITION                              0x1004
#define AL_VELOCITY                              0x1006
#define AL_LOOPING                               0x1007
#define AL_BUFFER                                0x1009
#define AL_GAIN                                  0x100A
#define AL_ORIENTATION                           0x100F
#define AL_SAMPLE_OFFSET                         0x1025
#define AL_FORMAT_MONO16                         0x1101

void alBufferData(ALuint buffer, ALenum format, const ALvoid *data, ALsizei size, ALsizei freq);
void alDeleteBuffers(ALsizei n, const ALuint *buffers);
void alDeleteSources(ALsizei n, const ALuint *sources);
void alGenBuffers(ALsizei n, ALuint *buffers);
void alGenSources(ALsizei n, ALuint *sources);
void alListenerfv(ALenum param, const ALfloat *values);
void alSourcef(ALuint source, ALenum param, ALfloat value);
void alSourcefv(ALuint source, ALenum param, const ALfloat *values);
void alSourcei(ALuint source, ALenum param, ALint value);
void alSourcePlay(ALuint source);
void alSourceRewind(ALuint source);
void alSourceStop(ALuint source);

extern int MA_lowLatency;
extern int MA_frequency;

#endif // MINIAL_H
