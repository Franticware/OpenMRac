#ifndef MINIAL_SB_H
#define MINIAL_SB_H

#include "minial_if.h"
#include "minisdl.h"

#include <map>
#include <vector>

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
    std::vector<Uint16> samples;
    float pitch = 1.f;
};

class MinialSB : public MinialInterface
{
public:
    MinialSB(ALCint freq);
    virtual ~MinialSB();
    virtual void GenSources(ALsizei n, ALuint *sources) override;
    virtual void GenBuffers(ALsizei n, ALuint *buffers) override;
    virtual void DeleteSources(ALsizei n, const ALuint *sources) override;
    virtual void DeleteBuffers(ALsizei n, const ALuint *buffers) override;
    virtual void Listenerfv(ALenum param, const ALfloat *values) override;
    virtual void BufferData(ALuint buffer, ALenum format, const ALvoid *data, ALsizei size, ALsizei freq) override;
    virtual void Sourcef(ALuint source, ALenum param, ALfloat value) override;
    virtual void Sourcefv(ALuint source, ALenum param, const ALfloat *values) override;
    virtual void Sourcei(ALuint source, ALenum param, ALint value) override;
    virtual void SourcePlay(ALuint source) override;
    virtual void SourceStop(ALuint source) override;
    virtual void SourceRewind(ALuint source) override;

    virtual void MA_periodicStream(void) override;

private:
    void ma_callback(void *userdata, Uint8 *stream, int len);

    int m_freq;

    ALuint sourceCounter = 1;
    ALuint bufferCounter = 1;

    std::map<ALuint, MA_Source> sourceMap;
    std::map<ALuint, MA_Buffer> bufferMap;
    std::vector<float> floatBuff;

};

#endif // MINIAL_SB_H
