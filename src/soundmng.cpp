#include "soundmng.h"
#include "rand1.h"
#include "gbuff_in.h"

#include <vector>
#include <cmath>
#include <climits>

int g_sound_on = 1;
static const float maxGain = 0.9f;

extern std::vector<ALuint> global_al_sources;
extern std::vector<ALuint> global_al_buffers;

ALuint createSource(ALuint buffer)
{
    ALuint source;
    alGenSources(1, &source); global_al_sources.push_back(source);
    const ALfloat sourcePos[] = { 0.0, 0.0, 0.0 };
    const ALfloat sourceVel[] = { 0.0, 0.0, 0.0 };
    alSourcef (source, AL_PITCH,    1.0      );
    alSourcef (source, AL_GAIN,     1.0  * maxGain   );
    alSourcefv(source, AL_POSITION, sourcePos);
    alSourcefv(source, AL_VELOCITY, sourceVel);
    alSourcei (source, AL_LOOPING,  0);
    if (buffer != 0)
        alSourcei (source, AL_BUFFER, buffer);
    return source;
}

/*void Sound_game_static::playLaprecord(float gain)
{
    alSourcef(p_laprecord_stream, AL_GAIN, gain*0.87*maxGain);
    alSourceStop(p_laprecord_stream);
    alSourcePlay(p_laprecord_stream);
}*/

void Sound_game_static::playSoundTest(float gain)
{
    alSourceStop(p_test_stream);
    alSourceRewind(p_test_stream);
    alSourcef(p_test_stream, AL_GAIN, gain*maxGain);
    alSourcePlay(p_test_stream);
}

void Sound_game_static::init()
{
    gbuff_in.f_open("crash0.raw", "rb");
    alGenBuffers(1, &(p_hit_sample[0])); global_al_buffers.push_back(p_hit_sample[0]);
    swapArrayLE16(gbuff_in.fbuffptr(), gbuff_in.fbuffsz());
    alBufferData(p_hit_sample[0], AL_FORMAT_MONO16, gbuff_in.fbuffptr(), gbuff_in.fbuffsz(), 22050);
    gbuff_in.fclose();

    gbuff_in.f_open("crash1.raw", "rb");
    alGenBuffers(1, &(p_hit_sample[1])); global_al_buffers.push_back(p_hit_sample[1]);
    swapArrayLE16(gbuff_in.fbuffptr(), gbuff_in.fbuffsz());
    alBufferData(p_hit_sample[1], AL_FORMAT_MONO16, gbuff_in.fbuffptr(), gbuff_in.fbuffsz(), 22050);
    gbuff_in.fclose();

    gbuff_in.f_open("skid.raw", "rb");
    alGenBuffers(1, &(p_skid_sample)); global_al_buffers.push_back(p_skid_sample);
    swapArrayLE16(gbuff_in.fbuffptr(), gbuff_in.fbuffsz());
    tweakLoop(gbuff_in.fbuffptr(), gbuff_in.fbuffsz());
    alBufferData(p_skid_sample, AL_FORMAT_MONO16, gbuff_in.fbuffptr(), gbuff_in.fbuffsz(), 22050);
    gbuff_in.fclose();

    for (unsigned int i = 0; i != 4; ++i) {
        p_skid_stream[i] = createSource(p_skid_sample);
    }
    for (int j = 0; j != 2; ++j)
        for (int i = 0; i != 5; ++i) {
            p_hit_stream[i+j*5] = createSource(p_hit_sample[j]);
        }

    p_test_stream = createSource(p_hit_sample[0]);
}

void Sound_game_static::load(unsigned int i, ALbuffer engine0_sample, ALbuffer engine1_sample)
{
    if (i >= 4)
        return;
    if (p_engine0_stream[i] == 0)
        p_engine0_stream[i] = createSource(engine0_sample);
    else
        alSourcei(p_engine0_stream[i], AL_BUFFER, engine0_sample);
    if (p_engine1_stream[i] == 0)
        p_engine1_stream[i] = createSource(engine1_sample);
    else
        alSourcei(p_engine1_stream[i], AL_BUFFER, engine1_sample);
}


void Sound_car::stop()
{
    alSourceStop(p_engine0_stream);
    p_engine0_state = 0;
    alSourceStop(p_engine1_stream);
    p_engine1_state = 0;
    alSourceStop(p_skid_stream);
    p_skid_state = 0;
}

const float engine1_volume0 = 0.75f;

void Sound_car::frame(float deltaT, int engine_state /*0 - nultý, 1 - první, 2 - první potichu*/, float engine_pitch, const float velocity[2])
{
    //
    p_time += deltaT;
    if (p_time >= p_T) p_time = 0.f; else return;
    if (!p_engine_on && engine_state == 2)
        engine_state = 0;
    if (engine_state == 0)
    {
        p_engine_on = 0;
        alSourcef(p_engine0_stream, AL_GAIN, 0.5f**p_global_volume*maxGain);
        if (!p_engine0_state) {
            alSourcePlay(p_engine0_stream);
            p_engine0_state = 1;
        }
        if (p_engine1_state) {
            alSourceStop(p_engine1_stream);
            p_engine1_state = 0;
        }
    } else {
        if (engine_state == 1) {
            p_engine_on = 1;
            alSourcef(p_engine1_stream, AL_GAIN, engine1_volume0**p_global_volume*maxGain);
        } else {
            alSourcef(p_engine1_stream, AL_GAIN, 0.5f**p_global_volume*maxGain);
        }

        alSourcef(p_engine1_stream, AL_PITCH, engine_pitch*p_running_pitch);

        if (!p_engine1_state) {
            alSourcePlay(p_engine1_stream);
            p_engine1_state = 1;
        }
        if (p_engine0_state) {
            alSourceStop(p_engine0_stream);
            p_engine0_state = 0;
        }
    }
    
    if (p_brake_volume <= 0.f)
    {
        if (p_skid_state) {
            alSourceStop(p_skid_stream);
            p_skid_state = 0;
        }
    } else {
        alSourcef(p_skid_stream, AL_GAIN, p_brake_volume**p_global_volume*maxGain);

        float speed = std::sqrt(velocity[0]*velocity[0] + velocity[1]*velocity[1]);
        float skidPitch = 0.8f+(speed - 6.f)/24.f*0.4f;
        if (skidPitch < 0.8f) skidPitch = 0.8f;
        if (skidPitch > 1.1f) skidPitch = 1.1f;

        alSourcef (p_skid_stream, AL_PITCH, skidPitch);

        if (!p_skid_state) {
            alSourcePlay(p_skid_stream);
            p_skid_state = 1;
        }
    }
}

void Sound_car::init(ALsource stream_idle, ALsource stream_running, float running_pitch, ALsource stream_skid, int player, int players_n)
{
    p_brake_volume = 0.f;
    // toto zásadně změnit
    p_running_pitch = running_pitch;

    p_player = player;
    switch (players_n)
    {
    case 1:
        p_pan = 0;
        break;
    case 2:
        if (player == 0) p_pan = -1; else p_pan = 1;
        break;
    default: // 3, 4
        if (player < 2) p_pan = -1; else p_pan = 1;
    }
    p_engine0_stream = stream_idle;
    p_engine1_stream = stream_running;
    p_skid_stream = stream_skid;
    p_time = 0.f;
    p_T = 0.01f;
    
    alSourcei(p_engine0_stream, AL_LOOPING, 1);
    alSourcei(p_engine1_stream, AL_LOOPING, 1);
    alSourcei(p_skid_stream, AL_LOOPING, 1);    
    // nejmenší délka v samplech je 32000
    alSourcef(p_engine0_stream, AL_SAMPLE_OFFSET, 32000.0*double(player)/double(players_n));
    alSourcef(p_engine0_stream, AL_SAMPLE_OFFSET, 32000.0*double(player)/double(players_n));
    alSourcef(p_engine0_stream, AL_SAMPLE_OFFSET, 32000.0*double(player)/double(players_n));
    
    alSourcef(p_engine0_stream, AL_GAIN, 0.5**p_global_volume);
    alSourcef(p_engine1_stream, AL_GAIN, engine1_volume0**p_global_volume*maxGain);

    p_engine0_state = 0; // 0 - nehraje, 1 - hraje
    p_engine1_state = 0; // 0 - nehraje, 1 - hraje
    p_skid_state = 0;

    p_engine_on = 0;
}

void Sound_crash::init(ALsource* stream_hit) // load zvuků
{
    p_hit_stream = stream_hit;
}

void Sound_crash::play(float c_j) // přehraje zvuk nárazu
{
    if (c_j <= 400.f) return;
    float volume = c_j/12000.f;
    if (volume > 1.f) volume = 1.f;
    if (volume < 0.f) volume = 0.f;
    static const int sampleIndexer[6] = {0, 1, 0, 1, 0, 1};
    int sample_sel =sampleIndexer[randn1(5)];
    int stream_sel = p_fronta_pos[sample_sel]++;
    p_fronta_pos[sample_sel] %= p_width;
    int sel = stream_sel+sample_sel*p_width;
    
    alSourceStop(p_hit_stream[sel]);
    alSourceRewind(p_hit_stream[sel]);
    alSourcef(p_hit_stream[sel], AL_GAIN, volume**p_global_volume*maxGain);
    
    float pitch_min = 0.88;
    float pitch = pitch_min+randn1(int((1.f-pitch_min)*2.f*1000.f))*0.001;
    alSourcef(p_hit_stream[sel], AL_PITCH, pitch);
    alSourcePlay(p_hit_stream[sel]);
}
