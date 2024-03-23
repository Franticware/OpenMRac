#ifndef HLIDAC_SOUNDMNG_H
#define HLIDAC_SOUNDMNG_H

#include <cstring>
#include "minial.h"

inline void swapArrayLE16(unsigned char* , unsigned int )
{
    // do nothing
}

inline void tweakLoop(unsigned char* data, unsigned int size)
{
    short* shortData = reinterpret_cast<short*>(data);
    unsigned int count = size / 2;
    float first = shortData[1];
    float last = shortData[count - 2];
    shortData[0] = first + (last - first) * 0.3;
    shortData[count - 1] = last + (first - last) * 0.3;
}

inline bool smallSampleRam()
{
    ALint ramKb = alGetInteger(AL_EXT_GUS_RAM_KB);
    if (ramKb == -1) return false;
    return ramKb < 512;
}

typedef ALuint ALbuffer;
typedef ALuint ALsource;

class Sound_game_static {
public:
    static constexpr int hitStreamCount = 5;
    static constexpr int hitSampleCount = 2;
    Sound_game_static() {
        for (int i = 0; i != 4; ++i)
        {
            p_skid_stream[i] = 0;
            p_engine0_sample[i] = 0;
            p_engine1_sample[i] = 0;
            p_engine_stream[i] = 0;
        }
        for (int i = 0; i != hitStreamCount; ++i)
            p_hit_stream[i] = 0;
        p_skid_sample = 0;
        p_hit_sample[0] = 0;
        p_hit_sample[1] = 0;
    }
    ~Sound_game_static() {
        for (int i = 0; i != 4; ++i)
        {
            p_skid_stream[i] = 0;
            p_engine0_sample[i] = 0;
            p_engine1_sample[i] = 0;
            p_engine_stream[i] = 0;
        }
        for (int i = 0; i != hitStreamCount; ++i)
            p_hit_stream[i] = 0;
        p_skid_sample = 0;
        p_hit_sample[0] = 0;
        p_hit_sample[1] = 0;
    }

    void init();
    void load(unsigned int i, ALbuffer p_engine0_sample, ALbuffer p_engine1_sample);

    ALsource p_skid_stream[4];
    ALsource p_engine_stream[4];
    ALbuffer p_engine0_sample[4];
    ALbuffer p_engine1_sample[4];
    ALsource p_hit_stream[hitStreamCount];

    ALbuffer p_skid_sample;
    ALbuffer p_hit_sample[hitSampleCount];

    //ALsource p_laprecord_stream;
    //ALbuffer p_laprecord_sample;

    //void playLaprecord(float);

    void playSoundTest(float);
};

class Sound_car {
public:
    Sound_car() : p_skid_stream(0), p_engine_stream(0), p_engine0_sample(0), p_engine1_sample(0), p_engine0_state(0), p_engine1_state(0), p_skid_state(0), p_time(0),
        p_T(0), p_running_pitch(0), p_pan(0), p_player(0), p_brake_volume(0), p_engine_on(0) { }
    ~Sound_car() {
    }
    void init(ALsource stream_engine, ALbuffer sample_idle, ALbuffer sample_running, float running_pitch, ALsource stream_skid, int player, int players_n);
    void frame(float deltaT, int engine_state/*0 - nultý, 1 - první, 2 - první potichu*/, float engine_pitch, const float velocity[2]);
    void stop();
    ALsource p_skid_stream;
    ALsource p_engine_stream;
    ALbuffer p_engine0_sample;
    ALbuffer p_engine1_sample;
    ALfloat p_engine0_offset = 0;
    ALfloat p_engine1_offset = 0;
    int p_engine0_state; // 0 - nehraje, 1 - hraje
    int p_engine1_state; // 0 - nehraje, 1 - hraje
    int p_skid_state; // 0 - nehraje, 1 - hraje
    float p_time; // aktualizace každých 100 ms
    float p_T; // 100 ms
    float p_running_pitch; // tón základního zvuku běžícího motoru
    int p_pan; // -1 vlevo, 0 střed, 1 vpravo
    int p_player;
    float p_brake_volume;
    int p_engine_on;
};

class Sound_crash {
public:
    Sound_crash() { p_hit_streams = 0; p_hit_samples = 0; p_currentStream = 0; }
    ~Sound_crash() {
    }
    void init(const ALsource* stream_hit, const ALbuffer* sample_hit); // load zvuků
    void play(float c_j); // přehraje zvuk nárazu
    const ALsource* p_hit_streams;
    const ALbuffer* p_hit_samples;
    ALuint p_currentStream;
};

#endif
