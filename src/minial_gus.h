#ifndef MINIAL_GUS_H
#define MINIAL_GUS_H

#include "minial_if.h"

#include <map>
#include <vector>

struct MA_GUS_Source
{
    ALuint buffer{0};
    ALfloat pitch{1.0};
    ALfloat gain{1.0};
    uint32_t pos{0};
    bool looping{false};
    bool playing{false};
    uint8_t gusVoice{32};
};

#define GUS_INVALID_ALLOC (0xffffffff)

struct MA_GUS_Buffer
{
    ALfloat pitch{1.0};
    uint32_t addr{GUS_INVALID_ALLOC};
    uint32_t len{0};
};

class MinialGUS : public MinialInterface
{
public:
    MinialGUS();
    virtual ~MinialGUS();
    virtual void GenSources(ALsizei n, ALuint* sources) override;
    virtual void GenBuffers(ALsizei n, ALuint* buffers) override;
    virtual void DeleteSources(ALsizei n, const ALuint* sources) override;
    virtual void DeleteBuffers(ALsizei n, const ALuint* buffers) override;
    virtual void Listenerfv(ALenum param, const ALfloat* values) override;
    virtual void BufferData(ALuint buffer, ALenum format, const ALvoid* data, ALsizei size, ALsizei freq) override;
    virtual void Sourcef(ALuint source, ALenum param, ALfloat value) override;
    virtual void Sourcefv(ALuint source, ALenum param, const ALfloat* values) override;
    virtual void Sourcei(ALuint source, ALenum param, ALint value) override;
    virtual void SourcePlay(ALuint source) override;
    virtual void SourceStop(ALuint source) override;
    virtual void SourceRewind(ALuint source) override;
    virtual ALint GetInteger(ALenum param) override;

    virtual void MA_periodicStream(void) override;

private:
    std::map<uint32_t, uint32_t> freeMap;

    uint32_t gusAlloc(uint32_t len);
    void gusFree(uint32_t pos);

    int memKb = 0;
    const int m_freq = 44100;
    //const int m_gusVoices = 14;
    const int m_gusVoices = 24;

    int gusVoiceMap[32] = {0};

    ALuint sourceCounter = 1;
    ALuint bufferCounter = 1;

    std::map<ALuint, MA_GUS_Source> sourceMap;
    std::map<ALuint, MA_GUS_Buffer> bufferMap;
};

#endif // MINIAL_GUS_H
