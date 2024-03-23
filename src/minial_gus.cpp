#include "minial_gus.h"

#include <cmath>
#include <cstdio>

#include "gus.h"

#define GUS_ALLOC_DEBUG_LOG 0

MinialGUS::MinialGUS(ALint monoSources)
{
    m_gusVoices = monoSources;
    uint32_t port = GUSFind();
    if (!port)
    {
        m_valid = false;
    }
    else
    {
        m_valid = true;
        uint32_t gusMemSize = GUSFindMem();
        memKb = gusMemSize >> 10;

        for (int i = 0; i != memKb / 256; ++i)
        {
            freeMap[262144 * i] = 262144;
        }

        GUSReset(m_gusVoices);
    }
}

MinialGUS::~MinialGUS()
{
    if (m_valid)
    {
        GUSReset(m_gusVoices);
    }
}

template <class T> void generateStuff(ALsizei n, ALuint* stuff, std::map<ALuint, T>& m, ALuint& counter)
{
    for (ALsizei i = 0; i != n; ++i)
    {
        T stuffObj;
        stuff[i] = counter;
        m[counter] = stuffObj;
        ++counter;
    }
}

template <class T> void deleteStuff(ALsizei n, const ALuint* stuff, std::map<ALuint, T>& m)
{
    for (ALsizei i = 0; i != n; ++i)
    {
        m.erase(stuff[i]);
    }
}

void MinialGUS::MA_periodicStream(void)
{
    // do nothing
}

uint32_t MinialGUS::gusAlloc(uint32_t len)
{
    for (auto fm : freeMap)
    {
        uint32_t oldLen = fm.second;
        if (oldLen >= len)
        {
            uint32_t oldStart = fm.first;
            freeMap.erase(oldStart);
            freeMap[oldStart + len] = oldLen - len;
            allocMap[oldStart] = len;
            return oldStart;
        }
    }
    return GUS_INVALID_ALLOC; // allocation failed
}

void MinialGUS::gusFree(uint32_t pos)
{
    if (pos == GUS_INVALID_ALLOC)
    {
        return;
    }
    auto it = allocMap.find(pos);
    if (it == allocMap.end())
    {
        return;
    }
    freeMap[it->first] = it->second;
    allocMap.erase(pos);
    gusDefrag();
}

void MinialGUS::gusDefrag(void)
{
    for (;;)
    {
        uint32_t thisAddr = 0;
        uint32_t nextAddr = 0;
        uint32_t newLen = 0;
        bool found = false;
        for (auto it : freeMap)
        {
            nextAddr = it.first + it.second;
            auto it2 = freeMap.find(nextAddr);
            if (nextAddr % 262144 != 0 && it2 != freeMap.end())
            {
                found = true;
                thisAddr = it.first;
                newLen = it.second + it2->second;
                break;
            }
        }
        if (!found)
        {
            break;
        }
        freeMap[thisAddr] = newLen;
        freeMap.erase(nextAddr);
    }
#if GUS_ALLOC_DEBUG_LOG
    printf("defrag[");
    bool first = true;
    for (auto it : freeMap)
    {
        if (!first)
        {
            printf("|");
        }
        printf("%x %x", it.first, it.first + it.second);
        first = false;
    }
    printf("]\n");
#endif
}

void MinialGUS::gusSetVolume(uint8_t voice, ALfloat listenerGain, ALfloat sourceGain)
{
    if (sourceGain < 0.f)
        sourceGain = 0.f;
    else if (sourceGain > 1.f)
        sourceGain = 1.f;
    GUSSetVolume(voice, std::floor(listenerGain * sourceGain * 511.0 + 0.5));
}

void MinialGUS::GenSources(ALsizei n, ALuint* sources)
{
    ALsizei freeVoices = 0;
    for (int i = 0; i != m_gusVoices; ++i)
    {
        if (!sourceMap[i].alloc)
        {
            ++freeVoices;
        }
    }
    if (freeVoices < n)
    {
        m_error = AL_OUT_OF_MEMORY;
        return;
    }
    ALsizei n1 = 0;
    for (ALuint i = 0; i != (ALuint)m_gusVoices; ++i)
    {
        if (!sourceMap[i].alloc)
        {
            sourceMap[i] = MA_GUS_Source();
            sourceMap[i].alloc = true;
            sources[n1++] = i + 1;
            if (n1 >= n)
            {
                return;
            }
        }
    }
}
void MinialGUS::GenBuffers(ALsizei n, ALuint* buffers)
{
    generateStuff(n, buffers, bufferMap, bufferCounter);
}
void MinialGUS::DeleteSources(ALsizei n, const ALuint* sources)
{
    for (ALsizei i = 0; i != n; ++i)
    {
        if (sources[i] > 0 && sources[i] <= (ALuint)m_gusVoices)
        {
            sourceMap[sources[i] - 1].alloc = false;
            if (sourceMap[sources[i] - 1].buffer)
            {
                auto it = bufferMap.find(sourceMap[sources[i] - 1].buffer);
                if (it != bufferMap.end())
                {
                    MA_GUS_Buffer& buf = it->second;
                    --buf.refcount;
                }
            }
        }
    }
}
void MinialGUS::DeleteBuffers(ALsizei n, const ALuint* buffers)
{
    for (ALsizei i = 0; i != n; ++i)
    {
        if (bufferMap[buffers[i]].refcount != 0)
        {
            m_error = AL_INVALID_OPERATION;
            return;
        }
    }
    for (ALsizei i = 0; i != n; ++i)
    {
        gusFree(bufferMap[buffers[i]].addr);
    }
    deleteStuff(n, buffers, bufferMap);
}
void MinialGUS::Listenerfv(ALenum param, const ALfloat* values)
{
    (void)param;
    (void)values;
    // listener parameters have no effect
    return;
}
void MinialGUS::BufferData(ALuint buffer, ALenum format, const ALvoid* data, ALsizei size, ALsizei freq)
{
    size >>= 1;

    if (buffer == 0 || format != AL_FORMAT_MONO16 || size == 0)
        return; // only 16-bit mono audio is currently supported
    MA_GUS_Buffer& buff = bufferMap[buffer];
    buff.pitch = float(freq) / float(m_freq);

    gusFree(buff.addr);
    buff.addr = gusAlloc(size);
#if GUS_ALLOC_DEBUG_LOG
    printf("alloc[");
    bool first = true;
    for (auto it : freeMap)
    {
        if (!first)
        {
            printf("|");
        }
        printf("%x %x", it.first, it.first + it.second);
        first = false;
    }
    printf("]\n");
#endif
    if (buff.addr != GUS_INVALID_ALLOC)
    {
        buff.len = size;
        for (int i = 0; i != size; ++i)
        {
            GUSPoke(buff.addr + i, ((int16_t*)data)[i] / 256);
        }
    }
    else
    {
        m_error = AL_OUT_OF_MEMORY;
    }
}
void MinialGUS::Sourcef(ALuint source, ALenum param, ALfloat value)
{
    if (source == 0)
    {
        m_error = AL_INVALID_NAME;
        return;
    }
    MA_GUS_Source& src = sourceMap[source - 1];
    uint8_t voiceI = source - 1;
    ALuint buffer = src.buffer;
    switch (param)
    {
    case AL_PITCH:
        src.pitch = value;
        if (voiceI < m_gusVoices && buffer)
        {
            MA_GUS_Buffer& buf = (bufferMap)[buffer];
            GUSSetFreq(voiceI, std::floor(src.pitch * buf.pitch * m_freq + 0.5));
        }
        break;
    case AL_GAIN:
        src.gain = value;
        if (voiceI < m_gusVoices && src.playing)
        {
            gusSetVolume(voiceI, m_listenerGain, src.gain);
        }
        break;
    case AL_SAMPLE_OFFSET:
        src.pos = std::floor(value + 0.5);
        if (voiceI < m_gusVoices && src.playing && buffer)
        {
            MA_GUS_Buffer& buf = (bufferMap)[buffer];
            if (buf.addr != GUS_INVALID_ALLOC)
            {
                uint32_t pos = src.pos;
                bool play = true;
                if (pos > buf.len)
                {
                    if (!src.looping)
                    {
                        play = false;
                    }
                    pos = 0;
                }
                if (play)
                {
                    GUSPlayVoice(voiceI, (src.looping ? GUS_LOOP_ENABLE : 0), buf.addr + pos, buf.addr,
                                 buf.addr + buf.len);
                }
            }
        }
        break;
    }
}
void MinialGUS::Sourcefv(ALuint source, ALenum param, const ALfloat* values)
{
    (void)source;
    (void)param;
    (void)values;
    return;
}

void MinialGUS::Sourcei(ALuint source, ALenum param, ALint value)
{
    if (source == 0)
        return;
    MA_GUS_Source& src = sourceMap[source - 1];
    uint8_t voiceI = source - 1;
    switch (param)
    {
    case AL_LOOPING:
        src.looping = !!value;
        if (voiceI < m_gusVoices && src.playing)
        {
            GUSVoiceControl(voiceI, (src.looping ? GUS_LOOP_ENABLE : 0));
        }
        break;
    case AL_BUFFER:
        if (src.state == AL_PLAYING && GUSGetVoiceStatus(voiceI) & (GUS_VOICE_STOP | GUS_VOICE_STOPPED))
        {
            src.state = AL_STOPPED;
        }
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
                    MA_GUS_Buffer& buf = it->second;
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
                MA_GUS_Buffer& buf = it->second;
                ++buf.refcount;
            }
            src.buffer = value;
        }
        break;
    }
}
void MinialGUS::SourcePlay(ALuint source)
{
    if (source == 0)
        return;
    MA_GUS_Source& src = sourceMap[source - 1];
    src.playing = true;
    src.state = AL_PLAYING;
    ALuint buffer = src.buffer;
    uint8_t voiceI = source - 1;
    if (voiceI < m_gusVoices && buffer)
    {
        MA_GUS_Buffer& buf = (bufferMap)[buffer];
        if (buf.addr != GUS_INVALID_ALLOC)
        {
            uint32_t pos = src.pos;
            bool play = true;
            if (pos > buf.len)
            {
                if (!src.looping)
                {
                    play = false;
                }
                pos = 0;
            }
            if (play)
            {
                gusSetVolume(voiceI, m_listenerGain, src.gain);
                GUSSetBalance(voiceI, 7);
                GUSSetFreq(voiceI, std::floor(src.pitch * buf.pitch * m_freq + 0.5));
                GUSPlayVoice(voiceI, (src.looping ? GUS_LOOP_ENABLE : 0), buf.addr + pos, buf.addr, buf.addr + buf.len);
            }
        }
    }
}

void MinialGUS::SourcePause(ALuint source)
{
    if (source == 0)
        return;
    MA_GUS_Source& src = sourceMap[source - 1];
    src.playing = false;
    src.state = AL_PAUSED;
    uint8_t voiceI = source - 1;
    if (voiceI < m_gusVoices)
    {
        ALfloat pos;
        GetSourcef(source, AL_SAMPLE_OFFSET, &pos);
        src.pos = pos;
        GUSVoiceControl(voiceI, GUS_VOICE_STOP);
    }
}

void MinialGUS::SourceStop(ALuint source)
{
    if (source == 0)
        return;
    MA_GUS_Source& src = sourceMap[source - 1];
    src.playing = false;
    src.state = AL_STOPPED;
    uint8_t voiceI = source - 1;
    if (voiceI < m_gusVoices)
    {
        src.pos = 0;
        GUSVoiceControl(voiceI, GUS_VOICE_STOP);
    }
}
void MinialGUS::SourceRewind(ALuint source)
{
    if (source == 0)
        return;
    MA_GUS_Source& src = sourceMap[source - 1];
    src.playing = false;
    src.state = AL_INITIAL;
    uint8_t voiceI = source - 1;
    if (voiceI < m_gusVoices)
    {
        src.pos = 0;
        GUSVoiceControl(voiceI, GUS_VOICE_STOP);
    }
}

ALint MinialGUS::GetInteger(ALenum param)
{
    if (param == AL_EXT_GUS_RAM_KB)
    {
        return memKb;
    }
    return -1;
}

void MinialGUS::GetSourcef(ALuint source, ALenum param, ALfloat* value)
{
    if (source == 0)
    {
        m_error = AL_INVALID_NAME;
        return;
    }
    MA_GUS_Source& src = sourceMap[source - 1];
    switch (param)
    {
    case AL_PITCH:
        *value = src.pitch;
        break;
    case AL_GAIN:
        *value = src.gain;
        break;
    case AL_SAMPLE_OFFSET: {
        uint8_t voiceI = source - 1;
        uint32_t pos = GUSVoicePos(voiceI) - bufferMap[src.buffer].addr;
        *value = pos;
        break;
    }
    default:
        m_error = AL_INVALID_ENUM;
        break;
    }
}

void MinialGUS::Listenerf(ALenum param, ALfloat value)
{
    switch (param)
    {
    case AL_GAIN:
        m_listenerGain = value;
        for (int i = 0; i != m_gusVoices; ++i)
        {
            if (sourceMap[i].alloc)
            {
                gusSetVolume(i, m_listenerGain, sourceMap[i].gain);
            }
        }
        break;
    default:
        m_error = AL_INVALID_ENUM;
        break;
    }
}

void MinialGUS::GetSourcei(ALuint source, ALenum param, ALint* value)
{
    if (source == 0)
    {
        m_error = AL_INVALID_NAME;
        return;
    }
    MA_GUS_Source& src = sourceMap[source - 1];
    switch (param)
    {
    case AL_LOOPING:
        *value = src.looping ? AL_TRUE : AL_FALSE;
        break;
    case AL_BUFFER:
        *value = src.buffer;
        break;
    case AL_SOURCE_STATE: {
        uint8_t voiceI = source - 1;
        if (src.state == AL_PLAYING && GUSGetVoiceStatus(voiceI) & (GUS_VOICE_STOP | GUS_VOICE_STOPPED))
        {
            src.state = AL_STOPPED;
        }
        *value = src.state;
        break;
    }
    default:
        m_error = AL_INVALID_ENUM;
        break;
    }
}

void MinialGUS::GetListenerf(ALenum param, ALfloat* value)
{
    switch (param)
    {
    case AL_GAIN:
        *value = m_listenerGain;
        break;
    default:
        m_error = AL_INVALID_ENUM;
        break;
    }
}
