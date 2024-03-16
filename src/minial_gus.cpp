#include "minial_gus.h"

#include <cmath>
#include <cstdio>

#include "gus.h"

MinialGUS::MinialGUS()
{
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
    /*TODO*/
}

void MinialGUS::GenSources(ALsizei n, ALuint* sources)
{
    generateStuff(n, sources, sourceMap, sourceCounter);

    for (int i = 0; i != n; ++i)
    {
        MA_GUS_Source& src = (sourceMap)[sources[i]];
        bool mapped = false;
        for (int j = 0; j != m_gusVoices; ++j)
        {
            if (gusVoiceMap[j] == 0)
            {
                //printf("gen %d\n", j);
                src.gusVoice = j;
                gusVoiceMap[j] = 1;
                mapped = true;
                break;
            }
        }
        if (!mapped)
        {
            printf("unmapped voice!\n");
        }
    }
}
void MinialGUS::GenBuffers(ALsizei n, ALuint* buffers)
{
    generateStuff(n, buffers, bufferMap, bufferCounter);
}
void MinialGUS::DeleteSources(ALsizei n, const ALuint* sources)
{
    for (int i = 0; i != n; ++i)
    {
        MA_GUS_Source& src = (sourceMap)[sources[i]];
        gusVoiceMap[src.gusVoice] = 0;
    }
    deleteStuff(n, sources, sourceMap);
}
void MinialGUS::DeleteBuffers(ALsizei n, const ALuint* buffers)
{
    for (int i = 0; i != n; ++i)
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

    buff.addr = gusAlloc(size);
    if (buff.addr != GUS_INVALID_ALLOC)
    {
        buff.len = size;
        for (int i = 0; i != size; ++i)
        {
            GUSPoke(buff.addr + i, ((int16_t*)data)[i] / 256);
        }
    }
}
void MinialGUS::Sourcef(ALuint source, ALenum param, ALfloat value)
{
    if (source == 0)
        return;
    MA_GUS_Source& src = (sourceMap)[source];
    ALuint buffer = src.buffer;
    switch (param)
    {
    case AL_PITCH:
        src.pitch = value;
        if (src.gusVoice < m_gusVoices && buffer)
        {
            MA_GUS_Buffer& buf = (bufferMap)[buffer];
            GUSSetFreq(src.gusVoice, std::floor(src.pitch * buf.pitch * m_freq + 0.5));
        }
        break;
    case AL_GAIN:
        src.gain = value;
        if (src.gusVoice < m_gusVoices && src.playing)
        {
            GUSSetVolume(src.gusVoice, std::floor(value * 511.0 + 0.5));
        }
        break;
    case AL_SAMPLE_OFFSET:
        src.pos = std::floor(value + 0.5);
        if (src.gusVoice < m_gusVoices && src.playing && buffer)
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
                    GUSPlayVoice(src.gusVoice, (src.looping ? GUS_LOOP_ENABLE : 0), buf.addr, buf.addr + pos, buf.addr + buf.len);

                    //printf("_vol %f, freq %f, voice %u, begin %u, start %u, end %u\n", std::floor(src.gain * 511.0 + 0.5), std::floor(src.pitch * buf.pitch * m_freq + 0.5), (uint32_t)src.gusVoice, buf.addr, buf.addr + pos, buf.addr + buf.len);
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
    MA_GUS_Source& src = (sourceMap)[source];
    switch (param)
    {
    case AL_LOOPING:
        src.looping = !!value;
        if (src.gusVoice < m_gusVoices && src.playing)
        {
            GUSVoiceControl(src.gusVoice, (src.looping ? GUS_LOOP_ENABLE : 0));
        }
        break;
    case AL_BUFFER:
        src.buffer = value;
        break;
    }
}
void MinialGUS::SourcePlay(ALuint source)
{
    if (source == 0)
        return;
    MA_GUS_Source& src = (sourceMap)[source];
    //printf("play %u %u\n", (uint32_t)src.gusVoice, (uint32_t)src.buffer);
    src.playing = true;
    ALuint buffer = src.buffer;
    if (src.gusVoice < m_gusVoices && buffer)
    {
        //printf("play1\n");
        MA_GUS_Buffer& buf = (bufferMap)[buffer];
        if (buf.addr != GUS_INVALID_ALLOC)
        {
            //printf("play2\n");
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

                GUSSetVolume(src.gusVoice, std::floor(src.gain * 511.0 + 0.5));
                GUSSetBalance(src.gusVoice, 7);
                GUSSetFreq(src.gusVoice, std::floor(src.pitch * buf.pitch * m_freq + 0.5));
                GUSPlayVoice(src.gusVoice, (src.looping ? GUS_LOOP_ENABLE : 0), buf.addr, buf.addr + pos, buf.addr + buf.len);


                //printf("vol %f, freq %f, voice %u, begin %u, start %u, end %u\n", std::floor(src.gain * 511.0 + 0.5), std::floor(src.pitch * buf.pitch * m_freq + 0.5), (uint32_t)src.gusVoice, buf.addr, buf.addr + pos, buf.addr + buf.len);
            }
        }
    }
}
void MinialGUS::SourceStop(ALuint source)
{
    if (source == 0)
        return;
    MA_GUS_Source& src = (sourceMap)[source];
    src.playing = false;
    if (src.gusVoice < m_gusVoices)
    {
        uint32_t pos = GUSVoicePos(src.gusVoice);
        src.pos = pos;
        GUSVoiceControl(src.gusVoice, GUS_VOICE_STOP);
        GUSSetVolume(src.gusVoice, 0);
    }
}
void MinialGUS::SourceRewind(ALuint source)
{
    if (source == 0)
        return;
    Sourcef(source, AL_SAMPLE_OFFSET, 0);
}

ALint MinialGUS::GetInteger(ALenum param)
{
    if (param == AL_EXT_GUS_RAM_KB)
    {
        return memKb;
    }
    return -1;
}
