#ifndef HLIDAC_SOUNDMNG_H
#define HLIDAC_SOUNDMNG_H

#include <cstring>

#ifdef USE_MINIAL
#include "minial.h"
#else
#include <AL/al.h>
#endif

#include <SDL2/SDL_endian.h>

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
inline void swapArrayLE16(unsigned char* , unsigned int )
{
    // do nothing
}
#else
inline void swapArrayLE16(unsigned char* data, unsigned int size)
{
    short* shortData = reinterpret_cast<short*>(data);
    unsigned int count = size / 2;
    for (unsigned int i = 0; i != count; ++i)
    {
        shortData[i] = SDL_SwapLE16(shortData[i]);
    }
}
#endif

inline void tweakLoop(unsigned char* data, unsigned int size)
{
    short* shortData = reinterpret_cast<short*>(data);
    unsigned int count = size / 2;
    float first = shortData[1];
    float last = shortData[count - 2];
    shortData[0] = first + (last - first) * 0.3;
    shortData[count - 1] = last + (first - last) * 0.3;
}

typedef ALuint ALbuffer;
typedef ALuint ALsource;

class Sound_game_static {
public:
    Sound_game_static() {
        for (int i = 0; i != 4; ++i)
        {
            p_skid_stream[i] = 0;
            p_engine0_stream[i] = 0;
            p_engine1_stream[i] = 0;
        }
        for (int i = 0; i != 10; ++i)
            p_hit_stream[i] = 0;
        p_test_stream = 0;
        p_skid_sample = 0;
        p_hit_sample[0] = 0;
        p_hit_sample[1] = 0;
    }

    void init();
    void load(unsigned int i, ALbuffer p_engine0_sample, ALbuffer p_engine1_sample);

    ALsource p_skid_stream[4];
    ALsource p_engine0_stream[4];
    ALsource p_engine1_stream[4];
    ALsource p_hit_stream[10];

    ALsource p_test_stream;

    ALbuffer p_skid_sample;
    ALbuffer p_hit_sample[2];

    void playSoundTest(float);
};

class Sound_car {
public:
    Sound_car() : p_skid_stream(0), p_engine0_stream(0), p_engine1_stream(0), p_engine0_state(0), p_engine1_state(0), p_skid_state(0), p_time(0),
        p_T(0), p_running_pitch(0), p_pan(0), p_player(0), p_brake_volume(0), p_engine_on(0), p_global_volume(0) { }
    void init(ALsource stream_idle, ALsource stream_running, float running_pitch, ALsource stream_skid, int player, int players_n);
    void frame(float deltaT, int engine_state/*0 - nultý, 1 - první, 2 - první potichu*/, float engine_pitch, const float velocity[2]);
    void stop();
    ALsource p_skid_stream;
    ALsource p_engine0_stream;
    ALsource p_engine1_stream;
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
    const float* p_global_volume;
};

class Sound_crash {
public:
    Sound_crash() { p_sz_samples = 2; p_width = 5; p_global_volume = 0; p_hit_stream = 0; p_fronta_pos[0] = 0; p_fronta_pos[1] = 0; }
    void init(ALsource* stream_hit); // load zvuků
    void play(float c_j); // přehraje zvuk nárazu
    int p_sz_samples;
    int p_width; // šířka fronty
    ALsource* p_hit_stream;
    int p_fronta_pos[10]; // stačí 2

    const float* p_global_volume;
    // n = 3
    // f = 5
    // samply[n]
    // fronty[f*n]
    // fronty_i[n]
};

#endif
