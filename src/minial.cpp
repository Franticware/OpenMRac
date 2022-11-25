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
#include <SDL2/SDL.h>

#define MA_FILTER_NONE 0
#define MA_FILTER_LINEAR 1 // recommended
#define MA_FILTER_HIGH_QUALITY 2

#define MA_FILTER MA_FILTER_LINEAR
#define MA_FREQ 22050
#define MA_SAMPLES 256

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
};

static ALCdevice alcDevice;
static ALCcontext alcContext;

static std::map<ALuint, MA_Source>* sourceMap = /*nullptr*/0;
static std::map<ALuint, MA_Buffer>* bufferMap = /*nullptr*/0;
static ALsizei maObtainedFreq = MA_FREQ;

static void ma_callback(void *userdata, Uint8 *stream, int len)
{
    (void)userdata;
    int count = len >> 2;
    float* floatBuff = (float*)stream;
    std::fill(floatBuff, floatBuff + count, 0.f);
    for (auto& p : *sourceMap)
    {
        MA_Source& src = p.second;
        if (src.playing)
        {
            if (src.buffer != 0)
            {
                MA_Buffer& buff = (*bufferMap)[src.buffer];
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
#if MA_FILTER == MA_FILTER_NONE
                        floatBuff[i] += buff.samples[src.pos] * src.gain;
#endif
#if MA_FILTER == MA_FILTER_LINEAR
                        uint32_t ipos0 = src.pos;
                        uint32_t ipos1 = ipos0 + 1;
                        float smp0 = buff.samples[ipos0];
                        float smp1 = 0;
                        if (ipos1 >= buff.samples.size())
                        {
                            if (src.looping)
                            {
                                smp1 = buff.samples[0];
                            }
                        }
                        else
                        {
                            smp1 = buff.samples[ipos1];
                        }
                        floatBuff[i] += (smp0 + (smp1 - smp0) * (src.pos - ipos0)) * src.gain;
#endif
#if MA_FILTER == MA_FILTER_HIGH_QUALITY
#error "Hight quality audio filter is currently not implemented"
#endif
                        src.pos += pitch;
                    }
                }
            }
        }
    }
}

////////////////

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
    SDL_InitSubSystem(SDL_INIT_AUDIO);
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
    (void)devicename;
    sourceMap = new std::map<ALuint, MA_Source>;
    bufferMap = new std::map<ALuint, MA_Buffer>;
    SDL_AudioSpec as;
    as.freq = MA_FREQ;
    as.format = AUDIO_F32SYS;
    as.channels = 1;
    as.samples = MA_SAMPLES;
    as.callback = ma_callback;
    as.userdata = /*nullptr*/0;
    SDL_AudioSpec obtained;
    SDL_AudioDeviceID id = SDL_OpenAudioDevice(devicename, 0, &as, &obtained, SDL_AUDIO_ALLOW_SAMPLES_CHANGE | SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);
    if (id > 0)
    {
        maObtainedFreq = obtained.freq;
        alcDevice.id = id;
        SDL_PauseAudioDevice(alcDevice.id, 0);
        return &alcDevice;
    }
    else
    {
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
    (void)context;
    return;
}

ALCboolean alcCloseDevice(ALCdevice *device)
{
    (void)device;
    SDL_CloseAudioDevice(device->id);
    delete sourceMap;
    delete bufferMap;
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
    generateStuff(n, sources, *sourceMap, sourceCounter);
}

void alGenBuffers(ALsizei n, ALuint *buffers)
{
    generateStuff(n, buffers, *bufferMap, bufferCounter);
}

template<class T> void deleteStuff(ALsizei n, const ALuint* stuff, std::map<ALuint, T>& m)
{
    SDL_LockAudioDevice(alcDevice.id);
    for (ALsizei i = 0; i != n; ++i)
    {
        m.erase(stuff[i]);
    }
    SDL_UnlockAudioDevice(alcDevice.id);
}

void alDeleteSources(ALsizei n, const ALuint *sources)
{
    deleteStuff(n, sources, *sourceMap);
}

void alDeleteBuffers(ALsizei n, const ALuint *buffers)
{
    deleteStuff(n, buffers, *bufferMap);
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
    if (buffer == 0 || format != AL_FORMAT_MONO16) return; // only 16-bit mono audio is currently supported
    SDL_LockAudioDevice(alcDevice.id);
    MA_Buffer& buff = (*bufferMap)[buffer];
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
    if (source == 0) return;
    SDL_LockAudioDevice(alcDevice.id);
    MA_Source& src = (*sourceMap)[source];
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
    if (source == 0) return;
    SDL_LockAudioDevice(alcDevice.id);
    MA_Source& src = (*sourceMap)[source];
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
    if (source == 0) return;
    SDL_LockAudioDevice(alcDevice.id);
    MA_Source& src = (*sourceMap)[source];
    src.playing = true;
    SDL_UnlockAudioDevice(alcDevice.id);
}

void alSourceStop(ALuint source)
{
    if (source == 0) return;
    SDL_LockAudioDevice(alcDevice.id);
    MA_Source& src = (*sourceMap)[source];
    src.playing = false;
    SDL_UnlockAudioDevice(alcDevice.id);
}

void alSourceRewind(ALuint source)
{
    if (source == 0) return;
    SDL_LockAudioDevice(alcDevice.id);
    MA_Source& src = (*sourceMap)[source];
    src.pos = 0;
    SDL_UnlockAudioDevice(alcDevice.id);
}

#endif // USE_MINIAL
