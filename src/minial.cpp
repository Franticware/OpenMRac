/* Naive implementation of a subset of OpenAL used by OpenMRac, featuring low sound quality and no optimization.
 * This should serve as a starting point on platforms where usable OpenAL implementation is not available.
 * To use this, add -DUSE_MINIAL to CFLAGS and remove -lopenal from LFLAGS.
 */

#ifdef USE_MINIAL

#include "minial.h"
#include <map>
#include <vector>
#include <algorithm>
#include <cmath>
#include <SDL/SDL.h>

#define MA_FREQ 22050
#define MA_SAMPLES 1024

struct ALCdevice
{
    ALuint dummy;
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
    std::vector<Sint16> samples;
};

static ALCdevice alcDevice;
static ALCcontext alcContext;

static std::map<ALuint, MA_Source>* sourceMap = /*nullptr*/0;
static std::map<ALuint, MA_Buffer>* bufferMap = /*nullptr*/0;
static std::vector<float>* floatBuff = /*nullptr*/0;

static void ma_callback(void *userdata, Uint8 *stream, int len)
{
    (void)userdata;
    Sint16* stream16 = (Sint16*)stream;
    int len16 = len >> 1;
    floatBuff->resize(len16);
    std::fill(floatBuff->begin(), floatBuff->end(), 0.f);
    for (auto& p : *sourceMap)
    {
        MA_Source& src = p.second;
        if (src.playing && src.gain > 0.f)
        {
            if (src.buffer != 0)
            {
                MA_Buffer& buff = (*bufferMap)[src.buffer];
                if (!buff.samples.empty())
                {
                    for (int i = 0; i != len16; ++i)
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
                        (*floatBuff)[i] += buff.samples[src.pos] * src.gain;
                        src.pos += src.pitch;
                    }
                }
            }
        }
    }

    for (int i = 0; i < len16; ++i)
    {
        int32_t temp = std::floor((*floatBuff)[i] + 0.5f);
        if (temp > 32767) temp = 32767;
        else if (temp < -32768) temp = -32768;
        stream16[i] = temp;
        //stream16[i] = (rand() % 256) - 128; // white noise
    }
}

////////////////

ALCboolean alcIsExtensionPresent(ALCdevice *device, const ALCchar *extname)
{
    (void)device;
    (void)extname;
    return 1;
}

const ALCchar* alcGetString(ALCdevice *device, ALCenum param)
{
    (void)device;
    (void)param;
    static const ALCchar* ret = "\0\0";
    return ret;
}

ALCdevice* alcOpenDevice(const ALCchar *devicename)
{
    (void)devicename;
    sourceMap = new std::map<ALuint, MA_Source>;
    bufferMap = new std::map<ALuint, MA_Buffer>;
    floatBuff = new std::vector<float>;
    floatBuff->resize(MA_SAMPLES);
    SDL_InitSubSystem(SDL_INIT_AUDIO);
    SDL_AudioSpec as;
    as.freq = MA_FREQ;
    as.format = AUDIO_S16;
    as.channels = 1;
    as.samples = MA_SAMPLES;
    as.callback = ma_callback;
    as.userdata = /*nullptr*/0;
    SDL_OpenAudio(&as, /*nullptr*/0);
    SDL_PauseAudio(0);
    return &alcDevice;
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
    SDL_CloseAudio();
    delete sourceMap;
    delete bufferMap;
    delete floatBuff;
    return 1;
}

////////////////

static ALuint sourceCounter = 1;
static ALuint bufferCounter = 1;

template<class T> void generateStuff(ALsizei n, ALuint* stuff, std::map<ALuint, T>& m, ALuint& counter)
{
    SDL_LockAudio();
    for (ALsizei i = 0; i != n; ++i)
    {
        T stuffObj;
        stuff[i] = counter;
        m[counter] = stuffObj;
        ++counter;
    }
    SDL_UnlockAudio();
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
    SDL_LockAudio();
    for (ALsizei i = 0; i != n; ++i)
    {
        m.erase(stuff[i]);
    }
    SDL_UnlockAudio();
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
    return;
}

void alBufferData(ALuint buffer, ALenum format, const ALvoid *data, ALsizei size, ALsizei freq)
{
    if (buffer == 0 || format != AL_FORMAT_MONO16 || freq != MA_FREQ) return;
    SDL_LockAudio();
    MA_Buffer& buff = (*bufferMap)[buffer];
    buff.samples.resize(size >> 1);
    std::copy((Sint16*)data, ((Sint16*)data) + buff.samples.size(), buff.samples.begin());
    SDL_UnlockAudio();
}

void alSourcef(ALuint source, ALenum param, ALfloat value)
{
    if (source == 0) return;
    SDL_LockAudio();
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
    SDL_UnlockAudio();
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
    SDL_LockAudio();
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
    SDL_UnlockAudio();
}

void alSourcePlay(ALuint source)
{
    if (source == 0) return;
    SDL_LockAudio();
    MA_Source& src = (*sourceMap)[source];
    src.playing = true;
    SDL_UnlockAudio();
}

void alSourceStop(ALuint source)
{
    if (source == 0) return;
    SDL_LockAudio();
    MA_Source& src = (*sourceMap)[source];
    src.playing = false;
    SDL_UnlockAudio();
}

void alSourceRewind(ALuint source)
{
    if (source == 0) return;
    SDL_LockAudio();
    MA_Source& src = (*sourceMap)[source];
    src.pos = 0;
    SDL_UnlockAudio();
}

#endif // USE_MINIAL
