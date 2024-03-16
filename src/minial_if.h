#ifndef MINIAL_IF_H
#define MINIAL_IF_H

#include "minial.h"

class MinialInterface
{
public:
    virtual ~MinialInterface()
    {
    }
    virtual void GenSources(ALsizei n, ALuint* sources) = 0;
    virtual void GenBuffers(ALsizei n, ALuint* buffers) = 0;
    virtual void DeleteSources(ALsizei n, const ALuint* sources) = 0;
    virtual void DeleteBuffers(ALsizei n, const ALuint* buffers) = 0;
    virtual void Listenerfv(ALenum param, const ALfloat* values) = 0;
    virtual void BufferData(ALuint buffer, ALenum format, const ALvoid* data, ALsizei size, ALsizei freq) = 0;
    virtual void Sourcef(ALuint source, ALenum param, ALfloat value) = 0;
    virtual void Sourcefv(ALuint source, ALenum param, const ALfloat* values) = 0;
    virtual void Sourcei(ALuint source, ALenum param, ALint value) = 0;
    virtual void SourcePlay(ALuint source) = 0;
    virtual void SourceStop(ALuint source) = 0;
    virtual void SourceRewind(ALuint source) = 0;
    virtual ALint GetInteger(ALenum param) = 0;

    virtual void MA_periodicStream(void) = 0;

    bool valid() const { return m_valid; }

protected:
    bool m_valid;
};

#endif // MINIAL_IF_H
