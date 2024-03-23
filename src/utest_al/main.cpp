#include "tests.h"

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;
    printf("minial unit test suite\n");

    ALCdevice* aldevice = alcOpenDevice(argc > 1 ? argv[1] : 0); // NULL parameter = open default device
    ALCcontext* alcontext = NULL;
    if (aldevice != NULL)
    {

        alcontext = alcCreateContext(aldevice, attribs); // create context
        if (alcontext != NULL)
        {
            alcMakeContextCurrent(alcontext); // set active context
        }
        else
        {
            fprintf(stderr, "%s", "Error: Can't create OpenAL context\n");
        }
    }
    else
    {
        if (argc > 1)
            fprintf(stderr, "Error: Can't open OpenAL device \"%s\"\n", argv[1]);
        else
            fprintf(stderr, "Error: Can't open default OpenAL device");
    }

    ALfloat listenerPos[] = {0.0, 0.0, 0.0};
    ALfloat listenerVel[] = {0.0, 0.0, 0.0};
    ALfloat listenerOri[] = {0.0, 0.0, -1.0, 0.0, 1.0, 0.0};
    alListenerfv(AL_POSITION, listenerPos);
    alListenerfv(AL_VELOCITY, listenerVel);
    alListenerfv(AL_ORIENTATION, listenerOri);
    alListenerf(AL_GAIN, 1.0);

    // unit test suite start
    int errCount = 0;
    if (true)
    {
        ALuint sampleA, sampleB;
        std::vector<uint8_t> wavData;
        if (loadWav("monti.wav", wavData))
        {
            printf("error loading wav data\n");
        }

        ALuint buffers[2];
        alGenBuffers(2, buffers);
        utAlErrCheck(__LINE__);

        sampleA = buffers[0];
        sampleB = buffers[1];
        alBufferData(sampleA, AL_FORMAT_MONO16, wavData.data(), wavData.size(), 22050);
        utAlErrCheck(__LINE__);

        if (loadWav("handel.wav", wavData))
        {
            printf("error loading wav data\n");
        }

        alBufferData(sampleB, AL_FORMAT_MONO16, wavData.data(), wavData.size(), 22050);
        utAlErrCheck(__LINE__);

        errCount += testBufferSwitch(sampleA, sampleB);
        errCount += testGain(sampleA);

        alDeleteBuffers(2, buffers);
        utAlErrCheck(__LINE__);
    }

    errCount += testBufferRefs();

    // unit test suite end

    if (alcontext != NULL)
        alcDestroyContext(alcontext);
    if (aldevice != NULL)
        alcCloseDevice(aldevice);

    printf("error count = %d\n", errCount);

    return !!errCount;
}
