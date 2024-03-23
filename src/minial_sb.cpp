#include "minial_sb.h"

#include "sb.h"
#include <cmath>

MinialSB::MinialSB(ALCint freq)
{
    m_freq = freq;
    floatBuff.resize(DMA_CHUNK);

    if (sb_init())
    {
        m_valid = true;
        StreamStart(m_freq);
    }
    else
    {
        m_valid = false;
    }
}

MinialSB::~MinialSB()
{
    if (m_valid)
    {
        StreamStop();
        sb_cleanup();
    }
}

#define MA_FILTER_NONE 0
#define MA_FILTER_LINEAR 1 // recommended
#define MA_FILTER_HIGH_QUALITY 2

#define MA_FILTER MA_FILTER_LINEAR

template <class T> void generateStuff(ALsizei n, ALuint* stuff, std::map<ALuint, T>& m, ALuint& counter)
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

template <class T> void deleteStuff(ALsizei n, const ALuint* stuff, std::map<ALuint, T>& m)
{
    SDL_LockAudio();
    for (ALsizei i = 0; i != n; ++i)
    {
        m.erase(stuff[i]);
    }
    SDL_UnlockAudio();
}

void MinialSB::ma_callback(void* userdata, Uint8* stream, int len)
{
    (void)userdata;
    floatBuff.resize(len);
    std::fill(floatBuff.begin(), floatBuff.end(), 0.f);

    for (auto& p : sourceMap)
    {
        MA_SB_Source& src = p.second;
        if (src.playing)
        {
            if (src.buffer != 0)
            {
                MA_SB_Buffer& buff = (bufferMap)[src.buffer];
                if (!buff.samples.empty())
                {
                    const float pitch = src.pitch * buff.pitch;
                    for (int i = 0; i != len; ++i)
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
                                src.state = AL_STOPPED;
                            }
                        }
                        if (!src.playing)
                            break;
#if MA_FILTER == MA_FILTER_NONE
                        (*floatBuff)[i] += buff.samples[src.pos] * src.gain;
#endif
#if MA_FILTER == MA_FILTER_LINEAR
                        if (src.gain > 0.f && m_listenerGain > 0.f)
                        {
                            uint32_t ipos0 = src.pos;
                            uint32_t ipos1 = ipos0 + 1;
                            Sint16 smp0 = buff.samples[ipos0];
                            Sint16 smp1 = 0;
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
                            (floatBuff)[i] +=
                                (float(smp0) + (float(smp1) - float(smp0)) * (src.pos - ipos0)) * src.gain;
                        }
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

    for (int i = 0; i < len; ++i)
    {
        int32_t temp = std::floor((floatBuff)[i] * m_listenerGain * 3.90625e-3 + 128.5f);
        if (temp > 255)
            temp = 255;
        else if (temp < 0)
            temp = 0;
        stream[i] = temp;
    }
}

void MinialSB::MA_periodicStream(void)
{
    size_t len;
    unsigned char* stream = StreamBuf(&len);
    if (stream)
    {
        ma_callback(0, stream, len);
        StreamReady();
    }
}

void MinialSB::GenSources(ALsizei n, ALuint* sources)
{
    generateStuff(n, sources, sourceMap, sourceCounter);
}

void MinialSB::GenBuffers(ALsizei n, ALuint* buffers)
{
    generateStuff(n, buffers, bufferMap, bufferCounter);
}

void MinialSB::DeleteSources(ALsizei n, const ALuint* sources)
{
    for (ALsizei i = 0; i != n; ++i)
    {
        ALuint buf = sourceMap[sources[i]].buffer;
        if (buf)
        {
            auto it = bufferMap.find(buf);
            if (it != bufferMap.end())
            {
                MA_SB_Buffer& buf = it->second;
                --buf.refcount;
            }
        }
    }
    deleteStuff(n, sources, sourceMap);
}

void MinialSB::DeleteBuffers(ALsizei n, const ALuint* buffers)
{
    for (ALsizei i = 0; i != n; ++i)
    {
        if (bufferMap[buffers[i]].refcount != 0)
        {
            m_error = AL_INVALID_OPERATION;
            return;
        }
    }
    deleteStuff(n, buffers, bufferMap);
}

void MinialSB::Listenerfv(ALenum param, const ALfloat* values)
{
    (void)param;
    (void)values;
    // listener parameters have no effect
    return;
}

void MinialSB::BufferData(ALuint buffer, ALenum format, const ALvoid* data, ALsizei size, ALsizei freq)
{
    if (buffer == 0 || format != AL_FORMAT_MONO16)
        return; // only 16-bit mono audio is currently supported
    SDL_LockAudio();
    MA_SB_Buffer& buff = (bufferMap)[buffer];
    buff.pitch = float(freq) / float(m_freq);
    buff.samples.resize(size >> 1);
    std::copy((Sint16*)data, ((Sint16*)data) + buff.samples.size(), buff.samples.begin());
    SDL_UnlockAudio();
}

void MinialSB::Sourcef(ALuint source, ALenum param, ALfloat value)
{
    if (source == 0 || sourceMap.find(source) == sourceMap.end())
    {
        m_error = AL_INVALID_NAME;
        return;
    }
    SDL_LockAudio();
    MA_SB_Source& src = (sourceMap)[source];
    switch (param)
    {
    case AL_PITCH:
        if (value < 0.f)
            value = 0.f;
        src.pitch = value;
        break;
    case AL_GAIN:
        src.gainOrig = value;
        if (value < 0.f)
            value = 0.f;
        if (value > 1.f)
            value = 1.f;
        src.gain = value;
        break;
    case AL_SAMPLE_OFFSET:
        if (value < 0.f)
            value = 0.f;
        src.pos = value;
        break;
    default:
        m_error = AL_INVALID_ENUM;
        break;
    }
    SDL_UnlockAudio();
}

void MinialSB::Sourcefv(ALuint source, ALenum param, const ALfloat* values)
{
    if (source == 0 || sourceMap.find(source) == sourceMap.end())
    {
        m_error = AL_INVALID_NAME;
        return;
    }
    (void)param;
    (void)values;
    return;
}

void MinialSB::Sourcei(ALuint source, ALenum param, ALint value)
{
    if (source == 0 || sourceMap.find(source) == sourceMap.end())
    {
        m_error = AL_INVALID_NAME;
        return;
    }
    SDL_LockAudio();
    MA_SB_Source& src = (sourceMap)[source];
    switch (param)
    {
    case AL_LOOPING:
        src.looping = !!value;
        break;
    case AL_BUFFER:
        if (src.state != AL_INITIAL && src.state != AL_STOPPED)
        {
            m_error = AL_INVALID_OPERATION;
        }
        else
        {
            if (src.buffer)
            {
                auto it = bufferMap.find(src.buffer);
                if (it != bufferMap.end())
                {
                    MA_SB_Buffer& buf = it->second;
                    --buf.refcount;
                }
            }
            if (value)
            {
                auto it = bufferMap.find(value);
                if (it == bufferMap.end())
                {
                    m_error = AL_INVALID_NAME;
                    return;
                }
                MA_SB_Buffer& buf = it->second;
                ++buf.refcount;
            }
            src.buffer = value;
        }
        break;
    default:
        m_error = AL_INVALID_ENUM;
        break;
    }
    SDL_UnlockAudio();
}

void MinialSB::SourcePlay(ALuint source)
{
    if (source == 0 || sourceMap.find(source) == sourceMap.end())
    {
        m_error = AL_INVALID_NAME;
        return;
    }
    SDL_LockAudio();
    MA_SB_Source& src = (sourceMap)[source];
    src.playing = true;
    src.state = AL_PLAYING;
    SDL_UnlockAudio();
}

void MinialSB::SourcePause(ALuint source)
{
    if (source == 0 || sourceMap.find(source) == sourceMap.end())
    {
        m_error = AL_INVALID_NAME;
        return;
    }
    SDL_LockAudio();
    MA_SB_Source& src = (sourceMap)[source];
    src.playing = false;
    src.state = AL_PAUSED;
    SDL_UnlockAudio();
}

void MinialSB::SourceStop(ALuint source)
{
    if (source == 0 || sourceMap.find(source) == sourceMap.end())
    {
        m_error = AL_INVALID_NAME;
        return;
    }
    SDL_LockAudio();
    MA_SB_Source& src = (sourceMap)[source];
    src.playing = false;
    src.state = AL_STOPPED;
    src.pos = 0;
    SDL_UnlockAudio();
}
void MinialSB::SourceRewind(ALuint source)
{
    if (source == 0 || sourceMap.find(source) == sourceMap.end())
    {
        m_error = AL_INVALID_NAME;
        return;
    }
    SDL_LockAudio();
    MA_SB_Source& src = (sourceMap)[source];
    src.state = AL_INITIAL;
    src.pos = 0;
    SDL_UnlockAudio();
}

ALint MinialSB::GetInteger(ALenum param)
{
    (void)param;
    return -1;
}

void MinialSB::GetSourcef(ALuint source, ALenum param, ALfloat* value)
{
    if (source == 0 || sourceMap.find(source) == sourceMap.end())
    {
        m_error = AL_INVALID_NAME;
        return;
    }
    SDL_LockAudio();
    MA_SB_Source& src = (sourceMap)[source];
    switch (param)
    {
    case AL_PITCH:
        *value = src.pitch;
        break;
    case AL_GAIN:
        *value = src.gainOrig;
        break;
    case AL_SAMPLE_OFFSET:
        *value = src.pos;
        break;
    default:
        m_error = AL_INVALID_ENUM;
        break;
    }
    SDL_UnlockAudio();
}

void MinialSB::Listenerf(ALenum param, ALfloat value)
{
    SDL_LockAudio();
    switch (param)
    {
    case AL_GAIN:
        m_listenerGain = value;
        break;
    default:
        m_error = AL_INVALID_ENUM;
        break;
    }
    SDL_UnlockAudio();
}

void MinialSB::GetSourcei(ALuint source, ALenum param, ALint* value)
{
    if (source == 0 || sourceMap.find(source) == sourceMap.end())
    {
        m_error = AL_INVALID_NAME;
        return;
    }
    SDL_LockAudio();
    MA_SB_Source& src = (sourceMap)[source];
    switch (param)
    {
    case AL_LOOPING:
        *value = src.looping ? AL_TRUE : AL_FALSE;
        break;
    case AL_BUFFER:
        *value = src.buffer;
        break;
    case AL_SOURCE_STATE:
        *value = src.state;
        break;
    default:
        m_error = AL_INVALID_ENUM;
        break;
    }
    SDL_UnlockAudio();
}

void MinialSB::GetListenerf(ALenum param, ALfloat* value)
{
    SDL_LockAudio();
    switch (param)
    {
    case AL_GAIN:
        *value = m_listenerGain;
        break;
    default:
        m_error = AL_INVALID_ENUM;
        break;
    }
    SDL_UnlockAudio();
}
