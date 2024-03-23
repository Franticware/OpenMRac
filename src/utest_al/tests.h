#ifndef TESTS_H
#define TESTS_H

#include "platform.h"
#include <cmath>
#include <cstdio>
#include <vector>

constexpr float fMargin = 0.001;

inline int loadWav(const char* fileName, std::vector<uint8_t>& data)
{
    constexpr int ok = 0;
    constexpr int err = 1;
    FILE* fin = fopen(fileName, "rb");
    if (fin)
    {
        data.clear();
        // skip wav header
        fseek(fin, 44, SEEK_SET);
        uint8_t b = 0;
        while (fread(&b, 1, 1, fin))
        {
            data.push_back(b);
        }
        fclose(fin);
        return ok;
    }
    return err;
}

inline void utPrint(const char* text, ALint a, ALint b)
{
    printf("%s = %d (0x%x), %d (0x%x) expected\n", text, a, a, b, b);
    fflush(stdout);
}

inline void utPrint(const char* text, ALint a)
{
    printf("%s = %d (0x%x), expected different value\n", text, a, a);
    fflush(stdout);
}

inline void utPrint(const char* text, ALfloat a, ALfloat b)
{
    printf("%s = %f, %f expected\n", text, a, b);
    fflush(stdout);
}

template <class T> void utEquals(int& errCount, const T& a, const T& b, const char* text, int line = 0)
{
    if (a != b)
    {
        ++errCount;
        if (line)
            printf("%d: ", line);
        utPrint(text, a, b);
    }
}

template <class T> void utNequals(int& errCount, const T& a, const T& b, const char* text, int line = 0)
{
    if (a == b)
    {
        ++errCount;
        if (line)
            printf("%d: ", line);
        utPrint(text, a);
    }
}

template <class T> void utEquals(int& errCount, const T& a, const T& b, const T& tol, const char* text, int line = 0)
{
    if (std::abs(double(a) - double(b)) > tol)
    {
        ++errCount;
        if (line)
            printf("%d: ", line);
        utPrint(text, a, b);
    }
}

inline void utAlErrCheck(int line = 0, ALenum expected = AL_NO_ERROR)
{
    ALenum err = alGetError();
    if (err != expected)
    {
        if (line)
            printf("%d: ", line);
        printf("AL error: 0x%x\n", err);
        fflush(stdout);
    }
}

inline int testBufferSwitch(ALuint sampleA, ALuint sampleB)
{
    printf("%s\n", __FUNCTION__);
    fflush(stdout);

    alListenerf(AL_GAIN, 1.0);

    int errCount = 0;
    ALuint source;
    alGenSources(1, &source);
    utAlErrCheck();
    const ALfloat sourcePos[] = {0.0, 0.0, 0.0};
    const ALfloat sourceVel[] = {0.0, 0.0, 0.0};
    alSourcef(0, AL_PITCH, 1.0);
    utAlErrCheck(__LINE__, AL_INVALID_NAME);
    // alSourcef(source, AL_GAIN, 1.0);
    alSourcefv(source, AL_POSITION, sourcePos);
    utAlErrCheck();
    alSourcefv(source, AL_VELOCITY, sourceVel);
    utAlErrCheck();
    // alSourcei(source, AL_LOOPING, 0);
    alSourcei(source, AL_BUFFER, sampleA);
    utAlErrCheck();

    ALint sourceState = -1;

    alGetSourcei(source, AL_SOURCE_STATE, &sourceState);
    utAlErrCheck();
    utEquals(errCount, sourceState, AL_INITIAL, "state");

    alSourcePlay(source);
    utAlErrCheck();

    delay_s(1);

    ALfloat offset = -1.f;
    alGetSourcef(source, AL_SAMPLE_OFFSET, &offset);
    utAlErrCheck();

    utEquals(errCount, offset, 21936.f, 2000.f, "offset", __LINE__);

    alSourceRewind(source);
    utAlErrCheck();

    alGetSourcei(source, AL_SOURCE_STATE, &sourceState);
    utAlErrCheck();
    utEquals(errCount, sourceState, AL_INITIAL, "state", __LINE__);

    alSourcePlay(source);
    utAlErrCheck();

    alGetSourcei(source, AL_SOURCE_STATE, &sourceState);
    utAlErrCheck();
    utEquals(errCount, sourceState, AL_PLAYING, "state", __LINE__);

    alSourcei(source, AL_BUFFER, sampleB);
    utAlErrCheck(__LINE__, AL_INVALID_OPERATION);

    delay_s(7);

    alGetSourcei(source, AL_SOURCE_STATE, &sourceState);
    utAlErrCheck();
    utEquals(errCount, sourceState, AL_STOPPED, "state", __LINE__);

    alSourcei(source, AL_BUFFER, sampleB);
    utAlErrCheck(__LINE__);

    alSourceRewind(source);
    utAlErrCheck();

    alGetSourcei(source, AL_SOURCE_STATE, &sourceState);
    utAlErrCheck();
    utEquals(errCount, sourceState, AL_INITIAL, "state", __LINE__);

    alSourcei(source, AL_BUFFER, sampleB);
    utAlErrCheck(__LINE__);

    alSourcePlay(source);
    utAlErrCheck(__LINE__);

    delay_s(2);

    alSourcePause(source);
    utAlErrCheck(__LINE__);

    delay_s(1);

    alGetSourcei(source, AL_SOURCE_STATE, &sourceState);
    utAlErrCheck();
    utEquals(errCount, sourceState, AL_PAUSED, "state", __LINE__);

    alSourcePlay(source);
    utAlErrCheck(__LINE__);

    delay_s(2);

    alDeleteSources(1, &source);
    utAlErrCheck(__LINE__);

    return errCount;
}

inline int testGain(ALuint sampleA)
{
    printf("%s\n", __FUNCTION__);
    fflush(stdout);

    alListenerf(AL_GAIN, 1.0);
    utAlErrCheck();

    int errCount = 0;
    ALuint source;
    alGenSources(1, &source);
    utAlErrCheck();
    const ALfloat sourcePos[] = {0.0, 0.0, 0.0};
    const ALfloat sourceVel[] = {0.0, 0.0, 0.0};
    alSourcef(source, AL_PITCH, 1.0);
    utAlErrCheck();

    alSourcei(source, AL_LOOPING, AL_TRUE);
    utAlErrCheck();

    alSourcefv(source, AL_POSITION, sourcePos);
    utAlErrCheck();
    alSourcefv(source, AL_VELOCITY, sourceVel);
    utAlErrCheck();

    alSourcei(source, AL_BUFFER, sampleA);
    utAlErrCheck();

    alSourcePlay(source);
    utAlErrCheck();

    alListenerf(AL_GAIN, 0.25);
    utAlErrCheck();
    alSourcef(source, AL_GAIN, 1);
    utAlErrCheck();

    float gain = -1;
    alGetListenerf(AL_GAIN, &gain);
    utAlErrCheck();
    utEquals(errCount, gain, 0.25f, "listener gain");

    delay_s(2);

    alListenerf(AL_GAIN, 0.25);
    utAlErrCheck();
    alSourcef(source, AL_GAIN, 8);
    utAlErrCheck();

    gain = -1;
    alGetSourcef(source, AL_GAIN, &gain);
    utAlErrCheck();
    utEquals(errCount, gain, 8.f, "source gain");

    delay_s(2);

    alListenerf(AL_GAIN, 8);
    utAlErrCheck();
    alSourcef(source, AL_GAIN, 0.25);
    utAlErrCheck();
    gain = -1;
    alGetListenerf(AL_GAIN, &gain);
    utAlErrCheck();
    utEquals(errCount, gain, 8.f, "listener gain");

    delay_s(4);

    alSourcei(source, AL_LOOPING, AL_FALSE);
    utAlErrCheck();

    alListenerf(AL_GAIN, 4);
    utAlErrCheck();
    alSourcef(source, AL_GAIN, 1);
    utAlErrCheck();

    for (;;)
    {
        ALint sourceState = -1;
        alGetSourcei(source, AL_SOURCE_STATE, &sourceState);
        utAlErrCheck();
        utNequals(errCount, sourceState, -1, "source state");
        if (sourceState != AL_PLAYING)
        {
            break;
        }
        delay_s(1);
    }

    alDeleteSources(1, &source);
    utAlErrCheck(__LINE__);

    return errCount;
}

int testBufferRefs()
{
    printf("%s\n", __FUNCTION__);
    fflush(stdout);
    int errCount = 0;
    std::vector<uint8_t> wavData;
    if (loadWav("monti.wav", wavData))
    {
        printf("error loading wav data\n");
    }
    ALuint sampleA = 0;
    alGenBuffers(1, &sampleA);
    utAlErrCheck(__LINE__);

    alBufferData(sampleA, AL_FORMAT_MONO16, wavData.data(), wavData.size(), 22050);
    utAlErrCheck(__LINE__);

    ALuint sourceA;
    alGenSources(1, &sourceA);
    utAlErrCheck(__LINE__);

    ALuint sourceB;
    alGenSources(1, &sourceB);
    utAlErrCheck(__LINE__);

    alSourcei(sourceA, AL_BUFFER, sampleA);
    utAlErrCheck(__LINE__);

    alSourcei(sourceB, AL_BUFFER, sampleA);
    utAlErrCheck(__LINE__);

    alDeleteBuffers(1, &sampleA);
    utAlErrCheck(__LINE__, AL_INVALID_OPERATION);

    ALuint sources[2] = {sourceA, sourceB};
    alDeleteSources(2, sources);
    utAlErrCheck(__LINE__);

    alDeleteBuffers(1, &sampleA);
    utAlErrCheck(__LINE__);

    return errCount;
}

#endif // TESTS_H
