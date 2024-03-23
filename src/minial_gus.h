#ifndef MINIAL_GUS_H
#define MINIAL_GUS_H

#include "minial_if.h"

#include <map>
#include <vector>

struct MA_GUS_Source
{
    bool alloc{false};
    ALuint buffer{0};
    ALint state{AL_INITIAL};
    ALfloat pitch{1.0};
    ALfloat gain{1.0};
    uint32_t pos{0};
    bool looping{false};
    bool playing{false};
};

#define GUS_INVALID_ALLOC (0xffffffff)

struct MA_GUS_Buffer
{
    ALfloat pitch{1.0};
    uint32_t addr{GUS_INVALID_ALLOC};
    uint32_t len{0};
    uint32_t refs{0}; // TODO count referring sources
};

class MinialGUS : public MinialInterface
{
public:
    MinialGUS(ALint monoSources);
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
    virtual void SourcePause(ALuint source) override;
    virtual void SourceStop(ALuint source) override;
    virtual void SourceRewind(ALuint source) override;
    virtual ALint GetInteger(ALenum param) override;
    virtual void GetSourcef(ALuint source, ALenum param, ALfloat* value) override;
    virtual void Listenerf(ALenum param, ALfloat value) override;
    virtual void GetSourcei(ALuint source, ALenum param, ALint* value) override;
    virtual void GetListenerf(ALenum param, ALfloat* value) override;

    virtual void MA_periodicStream(void) override;

private:
    std::map<uint32_t, uint32_t> freeMap;

    uint32_t gusAlloc(uint32_t len);
    void gusFree(uint32_t pos);
    void gusSetVolume(uint8_t voice, ALfloat listenerGain, ALfloat sourceGain);

    ALfloat m_listenerGain = 1.f;

    int memKb = 0;
    const int m_freq = 44100;
    int m_gusVoices = 32;
    static constexpr int m_maxGusVoices = 32;

    ALuint sourceCounter = 1;
    ALuint bufferCounter = 1;

    MA_GUS_Source sourceMap[m_maxGusVoices];
    std::map<ALuint, MA_GUS_Buffer> bufferMap;
};

#endif // MINIAL_GUS_H
