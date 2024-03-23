/* Copyright (c) 2022, Vojtěch Salajka. All rights reserved. Use of this source code is governed by a BSD-style license
 * that can be found in the LICENSE file. */

/*
 * Naive implementation of a subset of OpenAL used by OpenMRac, featuring low sound quality and no optimization.
 * This should serve as a starting point on platforms where usable OpenAL implementation is not available.
 * To use this, add -DUSE_MINIAL to CFLAGS and remove -lopenal from LFLAGS.
 */

#include "minial_gus.h"
#include "minial_sb.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <map>
#include <vector>

struct ALCdevice
{
    char deviceName[256] = {0};
};

struct ALCcontext
{
    MinialInterface* minialInterface;
};

static MinialInterface* minialInterface = 0;

void MA_periodicStream(void)
{
    if (minialInterface)
    {
        minialInterface->MA_periodicStream();
    }
}

////////////////

ALCboolean alcIsExtensionPresent(ALCdevice* device, const ALCchar* extname)
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

const ALCchar* alcGetString(ALCdevice* device, ALCenum param)
{
    if (device == 0 && (param == ALC_ALL_DEVICES_SPECIFIER || param == ALC_DEVICE_SPECIFIER))
    {
        static const ALCchar* ret = "\0\0";
        return ret;
    }
    return 0;
}

ALCdevice* alcOpenDevice(const ALCchar* devicename)
{
    ALCdevice* alcDevice = new ALCdevice;
    if (devicename)
    {
        strncpy(alcDevice->deviceName, devicename, 255);
    }
    return alcDevice;
}

ALCcontext* alcCreateContext(const ALCdevice* device, const ALCint* attrlist)
{
    if (device == 0)
    {
        return 0;
    }

    ALCint freq = 22050;
    ALCint monoSources = 32;

    if (attrlist)
    {
        for (int i = 0; attrlist[i * 2] || attrlist[i * 2 + 1]; ++i)
        {
            switch (attrlist[i * 2])
            {
            case ALC_FREQUENCY:
                freq = attrlist[i * 2 + 1];
                break;
            case ALC_MONO_SOURCES:
                monoSources = attrlist[i * 2 + 1];
                break;
            default:
                break;
            }
        }
    }

    ALCcontext* alcContext = new ALCcontext;
    alcContext->minialInterface = 0;

    if (device == 0 || strcmp(device->deviceName, "default") == 0 || strcmp(device->deviceName, "sb") == 0)
    {
        alcContext->minialInterface = new MinialSB(freq);
    }
    else if (strcmp(device->deviceName, "none") == 0)
    {
        alcContext->minialInterface = 0;
    }
    else if (strcmp(device->deviceName, "gus") == 0)
    {
        alcContext->minialInterface = new MinialGUS(monoSources);
    }
    else
    {
        delete alcContext;
        return 0;
    }

    if (alcContext->minialInterface)
    {
        if (!alcContext->minialInterface->valid())
        {
            printf("a%d\n", __LINE__);
            delete alcContext->minialInterface;
            delete alcContext;
            return 0;
        }
    }

    return alcContext;
}

ALCboolean alcMakeContextCurrent(ALCcontext* context)
{
    if (context)
    {
        minialInterface = context->minialInterface;
    }
    return 1;
}

void alcDestroyContext(ALCcontext* context)
{
    if (context)
    {
        if (context->minialInterface)
        {
            if (minialInterface == context->minialInterface)
            {
                minialInterface = 0;
            }
            delete context->minialInterface;
        }
        delete context;
    }
    return;
}

ALCboolean alcCloseDevice(ALCdevice* device)
{
    if (device)
    {
        delete device;
    }
    return 1;
}

////////////////

void alGenSources(ALsizei n, ALuint* sources)
{
    if (minialInterface)
        minialInterface->GenSources(n, sources);
}

void alGenBuffers(ALsizei n, ALuint* buffers)
{
    if (minialInterface)
        minialInterface->GenBuffers(n, buffers);
}

void alDeleteSources(ALsizei n, const ALuint* sources)
{
    if (minialInterface)
        minialInterface->DeleteSources(n, sources);
}

void alDeleteBuffers(ALsizei n, const ALuint* buffers)
{
    if (minialInterface)
        minialInterface->DeleteBuffers(n, buffers);
}

void alListenerfv(ALenum param, const ALfloat* values)
{
    if (minialInterface)
        minialInterface->Listenerfv(param, values);
    return;
}

void alBufferData(ALuint buffer, ALenum format, const ALvoid* data, ALsizei size, ALsizei freq)
{
    if (minialInterface)
        minialInterface->BufferData(buffer, format, data, size, freq);
}

void alSourcef(ALuint source, ALenum param, ALfloat value)
{
    if (minialInterface)
        minialInterface->Sourcef(source, param, value);
}

void alSourcefv(ALuint source, ALenum param, const ALfloat* values)
{
    if (minialInterface)
        minialInterface->Sourcefv(source, param, values);
    return;
}

void alSourcei(ALuint source, ALenum param, ALint value)
{
    if (minialInterface)
        minialInterface->Sourcei(source, param, value);
}

void alSourcePlay(ALuint source)
{
    if (minialInterface)
        minialInterface->SourcePlay(source);
}

void alSourcePause(ALuint source)
{
    if (minialInterface)
        minialInterface->SourcePause(source);
}

void alSourceStop(ALuint source)
{
    if (minialInterface)
        minialInterface->SourceStop(source);
}

void alSourceRewind(ALuint source)
{
    if (minialInterface)
        minialInterface->SourceRewind(source);
}

ALint alGetInteger(ALenum param)
{
    if (minialInterface)
    {
        return minialInterface->GetInteger(param);
    }
    return -1;
}

void alGetSourcef(ALuint source, ALenum param, ALfloat* value)
{
    if (minialInterface)
    {
        return minialInterface->GetSourcef(source, param, value);
    }
}

ALenum alGetError(void)
{
    if (minialInterface)
    {
        return minialInterface->GetError();
    }
    else
    {
        return AL_NO_ERROR;
    }
}

void alListenerf(ALenum param, ALfloat value)
{
    if (minialInterface)
    {
        return minialInterface->Listenerf(param, value);
    }
}

void alGetSourcei(ALuint source,  ALenum param, ALint *value)
{
    if (minialInterface)
    {
        return minialInterface->GetSourcei(source, param, value);
    }
}

void alGetListenerf(ALenum param, ALfloat *value)
{
    if (minialInterface)
    {
        return minialInterface->GetListenerf(param, value);
    }
}
