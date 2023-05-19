/* Copyright (c) 2022, Vojtěch Salajka. All rights reserved. Use of this source code is governed by a BSD-style license that can be found in the LICENSE file. */

/*
 * Naive implementation of a subset of OpenAL used by OpenMRac, featuring low sound quality and no optimization.
 * This should serve as a starting point on platforms where usable OpenAL implementation is not available.
 * To use this, add -DUSE_MINIAL to CFLAGS and remove -lopenal from LFLAGS.
 */

#ifdef USE_MINIAL

#include "minial.h"
#include <map>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <SDL2/SDL.h>

#define MA_INTERP_NONE 0 // awful
#define MA_INTERP_LINEAR 1 // fair
#define MA_INTERP_CUBIC_SPLINE 2 // good
#define MA_INTERP_HIGH_QUALITY 3 // not implemented

#define MA_INTERP MA_INTERP_CUBIC_SPLINE
//#define MA_INTERP MA_INTERP_LINEAR
//#define MA_INTERP MA_INTERP_NONE

#define MA_DEFAULT_FREQ 48000

int MA_lowLatency = 1;
int MA_frequency = MA_DEFAULT_FREQ;

#define MA_SAMPLES_LOW_LATENCY 256
#define MA_SAMPLES_NORMAL_LATENCY 1024

//#define STREAM_TO_FILE 1

#if STREAM_TO_FILE
#define STREAM_FILE "minial.raw"
FILE* minialStreamFile = nullptr;
#endif

struct ALCdevice
{
    SDL_AudioDeviceID id;
};

struct ALCcontext
{
    ALuint dummy;
};

struct MA_Source
{
    MA_Source()
    {
        buffer = 0;
        pitch = 1;
        gain = 1;
        pos = 0;
        looping = false;
        playing = false;
    }
    ALuint buffer;
    ALfloat pitch;
    ALfloat gain;
    ALfloat pos;
    bool looping;
    bool playing;
};

struct MA_Buffer
{
    std::vector<float> samples;
    float pitch = 1.f;
    inline float getSample(int i, bool loop)
    {
        if (samples.empty())
        {
            return 0.f;
        }
        if (i >= 0 && i < int(samples.size()))
        {
            return samples[i];
        }
        else
        {
            if (loop)
            {
                if (i < 0)
                {
                    while (i < 0)
                    {
                        i += samples.size();
                    }
                    if (i >= int(samples.size()))
                    {
                        return 0.f;
                    }
                    else
                    {
                        return samples[i];
                    }
                }
                else
                {
                    while (i >= int(samples.size()))
                    {
                        i -= int(samples.size());
                    }
                    if (i < 0)
                    {
                        return 0.f;
                    }
                    else
                    {
                        return samples[i];
                    }
                }
            }
            else
            {
                return 0.f;
            }
        }
    }
};

static ALCdevice alcDevice;
static ALCcontext alcContext;

static std::map<ALuint, MA_Source> sourceMap;
static std::map<ALuint, MA_Buffer> bufferMap;
static int maObtainedFreq = MA_DEFAULT_FREQ;

static void ma_callback(void *userdata, Uint8 *stream, int len)
{
    (void)userdata;
    int count = len >> 2;
    float* floatBuff = (float*)stream;
    std::fill(floatBuff, floatBuff + count, 0.f);
    if (sourceMap.empty()) return;
    for (auto& p : sourceMap)
    {
        MA_Source& src = p.second;
        if (src.playing)
        {
            if (src.buffer != 0)
            {
                MA_Buffer& buff = (bufferMap)[src.buffer];
                if (!buff.samples.empty())
                {
                    const float pitch = src.pitch * buff.pitch;
                    for (int i = 0; i != count; ++i)
                    {
                        while (src.pos >= buff.samples.size())
                        {
                            if (src.looping)
                            {
                                src.pos -= buff.samples.size();
                            }
                            else
                            {
                                src.pos = 0;
                                src.playing = false;
                            }
                        }
                        if (!src.playing) break;
#if MA_INTERP == MA_INTERP_NONE
                        const float output = buff.getSample(std::floor(src.pos), src.looping);
#endif
#if MA_INTERP == MA_INTERP_LINEAR
                        int ipos0 = std::floor(src.pos);
                        int ipos1 = ipos0 + 1;
                        float smp0 = buff.getSample(ipos0, src.looping);
                        float smp1 = buff.getSample(ipos1, src.looping);
                        const float output = (smp0 + (smp1 - smp0) * (src.pos - ipos0));
#endif
#if MA_INTERP == MA_INTERP_CUBIC_SPLINE
                        const int ipos_0 = std::floor(src.pos);
                        const int ipos_n1 = ipos_0 - 1;
                        const int ipos_1 = ipos_0 + 1;
                        const int ipos_2 = ipos_0 + 2;
                        const float sn1 = buff.getSample(ipos_n1, src.looping);
                        const float s0 = buff.getSample(ipos_0, src.looping);
                        const float s1 = buff.getSample(ipos_1, src.looping);
                        const float s2 = buff.getSample(ipos_2, src.looping);

                        const float pf = src.pos - ipos_0;
                        const float pf2 = pf * pf;

                        const float output = s0 + pf * 0.5f * (
                                    (2.f * pf2 - 3.f * pf - 1.f) * (s0 - s1) +
                                    (pf2 - 2.f * pf + 1.f) * (s0 - sn1) +
                                    (pf2 - pf) * (s2 - s1)
                                    );
#endif
#if MA_INTERP == MA_INTERP_HIGH_QUALITY
                        const float output = 0.f; // High quality audio filter is currently not implemented
#endif
                        floatBuff[i] += output * src.gain;
                        src.pos += pitch;
                    }
                }
            }
        }
    }

#if STREAM_TO_FILE
    if (minialStreamFile) fwrite(floatBuff, sizeof(float), count, minialStreamFile);
#endif
}

////////////////

static bool isSdlAudioInit = false;

static void sdlAudioInit(void)
{
#if STREAM_TO_FILE
    minialStreamFile = fopen(STREAM_FILE, "wb");
#endif
    if (!isSdlAudioInit)
    {
        SDL_InitSubSystem(SDL_INIT_AUDIO);
        isSdlAudioInit = true;
    }
}

ALCboolean alcIsExtensionPresent(ALCdevice *device, const ALCchar *extname)
{
    if (device == 0)
    {
        if (strcmp(extname, "ALC_ENUMERATE_ALL_EXT") == 0)
        {
            return 1;
        }
        if (strcmp(extname, "ALC_ENUMERATION_EXT") == 0)
        {
            return 1;
        }
    }
    return 0;
}

const ALCchar* alcGetString(ALCdevice *device, ALCenum param)
{
    sdlAudioInit();
    if (device == 0 && (param == ALC_ALL_DEVICES_SPECIFIER || param == ALC_DEVICE_SPECIFIER))
    {
        static std::vector<char> deviceList;
        deviceList.clear();
        const int count = SDL_GetNumAudioDevices(0);
        for (int i = 0; i != count; ++i)
        {
            const char* device = SDL_GetAudioDeviceName(i, 0);
            if (device)
            {
                for (size_t j = 0; j != strlen(device); ++j)
                {
                    deviceList.push_back(device[j]);
                }
                deviceList.push_back(0);
            }
        }
        deviceList.push_back(0);
        deviceList.push_back(0);
        return deviceList.data();
    }
    return 0;
}

ALCdevice* alcOpenDevice(const ALCchar *devicename)
{
    sdlAudioInit();
    (void)devicename;
    sourceMap.clear();
    bufferMap.clear();
    SDL_AudioSpec as;
    as.freq = MA_frequency;
    as.format = AUDIO_F32SYS;
    as.channels = 1;
    as.samples = (as.freq < 30000) ? 256 : 512;
    if (MA_lowLatency == 0) as.samples *= 4;
    as.callback = ma_callback;
    as.userdata = nullptr;
    SDL_AudioSpec obtained;
    SDL_AudioDeviceID id = SDL_OpenAudioDevice(devicename, 0, &as, &obtained, SDL_AUDIO_ALLOW_SAMPLES_CHANGE | SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);

    fprintf(stderr, "Audio driver: '%s'\n", SDL_GetCurrentAudioDriver()); fflush(stderr);

    if (id > 0)
    {
        maObtainedFreq = obtained.freq;
        alcDevice.id = id;
        SDL_PauseAudioDevice(alcDevice.id, 0);
        return &alcDevice;
    }
    else
    {
        fprintf(stderr, "MiniAL: %s\n", SDL_GetError()); fflush(stderr);
        return 0;
    }
}

ALCcontext* alcCreateContext(ALCdevice *device, const ALCint *attrlist)
{
    (void)device;
    (void)attrlist;
    return &alcContext;
}

ALCboolean alcMakeContextCurrent(ALCcontext *context)
{
    (void)context;
    return 1;
}

void alcDestroyContext(ALCcontext *context)
{
#if STREAM_TO_FILE
    if (minialStreamFile) fclose(minialStreamFile);
#endif
    (void)context;
    return;
}

ALCboolean alcCloseDevice(ALCdevice *device)
{
    (void)device;
    SDL_CloseAudioDevice(device->id);
    sourceMap.clear();
    bufferMap.clear();
    return 1;
}

////////////////

static ALuint sourceCounter = 1;
static ALuint bufferCounter = 1;

template<class T> void generateStuff(ALsizei n, ALuint* stuff, std::map<ALuint, T>& m, ALuint& counter)
{
    SDL_LockAudioDevice(alcDevice.id);
    for (ALsizei i = 0; i != n; ++i)
    {
        T stuffObj;
        stuff[i] = counter;
        m[counter] = stuffObj;
        ++counter;
    }
    SDL_UnlockAudioDevice(alcDevice.id);
}

void alGenSources(ALsizei n, ALuint *sources)
{
    generateStuff(n, sources, sourceMap, sourceCounter);
}

void alGenBuffers(ALsizei n, ALuint *buffers)
{
    generateStuff(n, buffers, bufferMap, bufferCounter);
}

template<class T> void deleteStuff(ALsizei n, const ALuint* stuff, std::map<ALuint, T> m)
{
    if (m.empty())
        return;
    SDL_LockAudioDevice(alcDevice.id);
    for (ALsizei i = 0; i != n; ++i)
    {
        m.erase(stuff[i]);
    }
    SDL_UnlockAudioDevice(alcDevice.id);
}

void alDeleteSources(ALsizei n, const ALuint *sources)
{
    deleteStuff(n, sources, sourceMap);
}

void alDeleteBuffers(ALsizei n, const ALuint *buffers)
{
    deleteStuff(n, buffers, bufferMap);
}

void alListenerfv(ALenum param, const ALfloat *values)
{
    (void)param;
    (void)values;
    // listener parameters have no effect
    return;
}

void alBufferData(ALuint buffer, ALenum format, const ALvoid *data, ALsizei size, ALsizei freq)
{
    if (buffer == 0 || bufferMap.empty() || format != AL_FORMAT_MONO16) return; // only 16-bit mono audio is currently supported
    SDL_LockAudioDevice(alcDevice.id);
    MA_Buffer& buff = (bufferMap)[buffer];
    buff.pitch = float(freq)/float(maObtainedFreq);
    buff.samples.resize(size >> 1);
    for (size_t i = 0; i != buff.samples.size(); ++i)
    {
        buff.samples[i] = ((Sint16*)data)[i] / 32768.f; // convert sample from s16 to float
    }
    SDL_UnlockAudioDevice(alcDevice.id);
}

void alSourcef(ALuint source, ALenum param, ALfloat value)
{
    if (source == 0 || sourceMap.empty()) return;
    SDL_LockAudioDevice(alcDevice.id);
    MA_Source& src = (sourceMap)[source];
    switch (param)
    {
    case AL_PITCH:
        if (value < 0.f) value = 0.f;
        src.pitch = value;
        break;
    case AL_GAIN:
        if (value < 0.f) value = 0.f;
        if (value > 1.f) value = 1.f;
        src.gain = value;
        break;
    case AL_SAMPLE_OFFSET:
        if (value < 0.f) value = 0.f;
        src.pos = value;
        break;
    }
    SDL_UnlockAudioDevice(alcDevice.id);
}

void alSourcefv(ALuint source, ALenum param, const ALfloat *values)
{
    (void)source;
    (void)param;
    (void)values;
    return;
}

void alSourcei(ALuint source, ALenum param, ALint value)
{
    if (source == 0 || sourceMap.empty()) return;
    SDL_LockAudioDevice(alcDevice.id);
    MA_Source& src = (sourceMap)[source];
    switch (param)
    {
    case AL_LOOPING:
        src.looping = !!value;
        break;
    case AL_BUFFER:
        src.buffer = value;
        break;
    }
    SDL_UnlockAudioDevice(alcDevice.id);
}

void alSourcePlay(ALuint source)
{
    if (source == 0 || sourceMap.empty()) return;
    SDL_LockAudioDevice(alcDevice.id);
    MA_Source& src = (sourceMap)[source];
    src.playing = true;
    SDL_UnlockAudioDevice(alcDevice.id);
}

void alSourceStop(ALuint source)
{
    if (source == 0 || sourceMap.empty()) return;
    SDL_LockAudioDevice(alcDevice.id);
    MA_Source& src = (sourceMap)[source];
    src.playing = false;
    SDL_UnlockAudioDevice(alcDevice.id);
}

void alSourceRewind(ALuint source)
{
    if (source == 0 || sourceMap.empty()) return;
    SDL_LockAudioDevice(alcDevice.id);
    MA_Source& src = (sourceMap)[source];
    src.pos = 0;
    SDL_UnlockAudioDevice(alcDevice.id);
}

#endif // USE_MINIAL
