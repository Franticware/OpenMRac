#include "soundmng.h"
#include "rand1.h"
#include "gbuff_in.h"

#include <vector>
#include <cmath>
#include <climits>

int g_sound_on = 1;

extern std::vector<ALuint> global_al_sources;
extern std::vector<ALuint> global_al_buffers;

ALuint createSource(ALuint buffer)
{
    ALuint source = 0;
    alGenSources(1, &source);
    if (alGetError() == AL_OUT_OF_MEMORY)
    {
        fprintf(stderr, "AL source allocation error\n");
    }
    global_al_sources.push_back(source);
    const ALfloat sourcePos[] = { 0.0, 0.0, 0.0 };
    const ALfloat sourceVel[] = { 0.0, 0.0, 0.0 };
    alSourcef (source, AL_PITCH,    1.0      );
    alSourcef (source, AL_GAIN,     1.0     );
    alSourcefv(source, AL_POSITION, sourcePos);
    alSourcefv(source, AL_VELOCITY, sourceVel);
    alSourcei (source, AL_LOOPING,  0);
    if (buffer != 0)
        alSourcei (source, AL_BUFFER, buffer);
    return source;
}

void Sound_game_static::playSoundTest(float gain)
{
    alSourceStop(p_hit_stream[0]);
    alSourceRewind(p_hit_stream[0]);
    alSourcei(p_hit_stream[0], AL_BUFFER, p_hit_sample[0]);
    alSourcef(p_hit_stream[0], AL_GAIN, 1.0);
    alSourcef(p_hit_stream[0], AL_PITCH, 1.0);
    alListenerf(AL_GAIN, gain);
    alSourcePlay(p_hit_stream[0]);
}

void Sound_game_static::init()
{
    gbuff_in.f_open("crash0.raw", "rb");if (smallSampleRam())gbuff_in.downsampleAudio16();
    alGenBuffers(1, &(p_hit_sample[0])); global_al_buffers.push_back(p_hit_sample[0]);
    swapArrayLE16(gbuff_in.fbuffptr(), gbuff_in.fbuffsz());
    alBufferData(p_hit_sample[0], AL_FORMAT_MONO16, gbuff_in.fbuffptr(), gbuff_in.fbuffsz(), smallSampleRam() ? 11025 : 22050);
    if (alGetError() == AL_OUT_OF_MEMORY)
    {
        fprintf(stderr, "AL buffer data allocation error\n");
    }
    gbuff_in.fclose();

    gbuff_in.f_open("crash1.raw", "rb");if (smallSampleRam())gbuff_in.downsampleAudio16();
    alGenBuffers(1, &(p_hit_sample[1])); global_al_buffers.push_back(p_hit_sample[1]);
    swapArrayLE16(gbuff_in.fbuffptr(), gbuff_in.fbuffsz());
    alBufferData(p_hit_sample[1], AL_FORMAT_MONO16, gbuff_in.fbuffptr(), gbuff_in.fbuffsz(), smallSampleRam() ? 11025 : 22050);
    if (alGetError() == AL_OUT_OF_MEMORY)
    {
        fprintf(stderr, "AL buffer data allocation error\n");
    }
    gbuff_in.fclose();

    gbuff_in.f_open("skid.raw", "rb");if (smallSampleRam())gbuff_in.downsampleAudio16();
    alGenBuffers(1, &(p_skid_sample)); global_al_buffers.push_back(p_skid_sample);
    swapArrayLE16(gbuff_in.fbuffptr(), gbuff_in.fbuffsz());
    tweakLoop(gbuff_in.fbuffptr(), gbuff_in.fbuffsz());
    alBufferData(p_skid_sample, AL_FORMAT_MONO16, gbuff_in.fbuffptr(), gbuff_in.fbuffsz(), smallSampleRam() ? 11025 : 22050);
    if (alGetError() == AL_OUT_OF_MEMORY)
    {
        fprintf(stderr, "AL buffer data allocation error\n");
    }
    gbuff_in.fclose();

    for (unsigned int i = 0; i != 4; ++i) {
        p_skid_stream[i] = createSource(p_skid_sample);
    }
    for (int i = 0; i != hitStreamCount; ++i)
    {
        p_hit_stream[i] = createSource(0);
    }
}

void Sound_game_static::load(unsigned int i, ALbuffer engine0_sample, ALbuffer engine1_sample)
{
    if (i >= 4)
        return;
    if (p_engine_stream[i] == 0)
        p_engine_stream[i] = createSource(0);

    p_engine0_sample[i] = engine0_sample;
    p_engine1_sample[i] = engine1_sample;
}

void Sound_car::stop()
{
    if (p_engine0_state)
    {
        alGetSourcef(p_engine_stream, AL_SAMPLE_OFFSET, &p_engine0_offset);
    }
    else if (p_engine1_state)
    {
        alGetSourcef(p_engine_stream, AL_SAMPLE_OFFSET, &p_engine1_offset);
    }
    alSourceStop(p_engine_stream);
    p_engine0_state = 0;
    p_engine1_state = 0;
    alSourcePause(p_skid_stream);
    p_skid_state = 0;
}

static constexpr float engine1_volume0 = 0.75f;

void Sound_car::frame(float deltaT, int engine_state /*0 - nultý, 1 - první, 2 - první potichu*/, float engine_pitch, const float velocity[2])
{
    p_time += deltaT;
    if (p_time >= p_T) p_time = 0.f; else return;
    if (!p_engine_on && engine_state == 2)
        engine_state = 0;
    if (engine_state == 0)
    {
        p_engine_on = 0;
        alSourcef(p_engine_stream, AL_GAIN, 0.5f);
        if (p_engine1_state) {
            alGetSourcef(p_engine_stream, AL_SAMPLE_OFFSET, &p_engine1_offset);
            alSourceStop(p_engine_stream);
            p_engine1_state = 0;
        }
        if (!p_engine0_state) {
            alSourcef(p_engine_stream, AL_GAIN, 0.5f);
            alSourcef(p_engine_stream, AL_PITCH, 1.f);
            alSourcei(p_engine_stream, AL_BUFFER, p_engine0_sample);
            alSourcef(p_engine_stream, AL_SAMPLE_OFFSET, p_engine0_offset);
            alSourcePlay(p_engine_stream);
            p_engine0_state = 1;
        }
    } else {
        if (engine_state == 1) {
            p_engine_on = 1;
            alSourcef(p_engine_stream, AL_GAIN, engine1_volume0);
        } else {
            alSourcef(p_engine_stream, AL_GAIN, 0.5f);
        }

        alSourcef(p_engine_stream, AL_PITCH, engine_pitch*p_running_pitch);

        if (p_engine0_state) {
            alGetSourcef(p_engine_stream, AL_SAMPLE_OFFSET, &p_engine0_offset);
            alSourceStop(p_engine_stream);
            p_engine0_state = 0;
        }
        if (!p_engine1_state) {
            alSourcei(p_engine_stream, AL_BUFFER, p_engine1_sample);
            alSourcef(p_engine_stream, AL_SAMPLE_OFFSET, p_engine1_offset);
            alSourcePlay(p_engine_stream);
            p_engine1_state = 1;
        }
    }

    if (p_brake_volume <= 0.f)
    {
        if (p_skid_state) {
            alSourceStop(p_skid_stream);
            p_skid_state = 0;
        }
    } else {
        alSourcef(p_skid_stream, AL_GAIN, p_brake_volume);

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

void Sound_car::init(ALsource stream_engine, ALbuffer sample_idle, ALbuffer sample_running, float running_pitch, ALsource stream_skid, int player, int players_n)
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
    p_engine_stream = stream_engine;
    p_engine0_sample = sample_idle;
    p_engine1_sample = sample_running;
    p_skid_stream = stream_skid;
    p_time = 0.f;
    p_T = 0.01f;

    alSourcei(p_engine_stream, AL_LOOPING, 1);
    alSourcei(p_skid_stream, AL_LOOPING, 1);
    // nejmenší délka v samplech je 32000

    p_engine0_offset = (smallSampleRam() ? 16000.0 : 32000.0)*double(player)/double(players_n);
    p_engine1_offset = (smallSampleRam() ? 16000.0 : 32000.0)*double(player)/double(players_n);

    alSourcef(p_skid_stream, AL_SAMPLE_OFFSET, (smallSampleRam() ? 16000.0 : 32000.0)*double(player)/double(players_n));

    p_engine0_state = 0; // 0 - nehraje, 1 - hraje
    p_engine1_state = 0; // 0 - nehraje, 1 - hraje

    p_skid_state = 0;

    p_engine_on = 0;
}

void Sound_crash::init(const ALsource* stream_hit, const ALbuffer* sample_hit) // load zvuků
{
    p_hit_streams = stream_hit;
    p_hit_samples = sample_hit;
}

void Sound_crash::play(float c_j) // přehraje zvuk nárazu
{
    if (c_j <= 400.f) return;
    float volume = c_j/12000.f;
    if (volume > 1.f) volume = 1.f;
    if (volume < 0.f) volume = 0.f;
    static const int sampleIndexer[6] = {0, 1, 0, 1, 0, 1};
    int sample_sel = sampleIndexer[randn1(5)];

    ALuint stream = p_hit_streams[p_currentStream];

    alSourceStop(stream);
    alSourceRewind(stream);
    alSourcei(stream, AL_BUFFER, p_hit_samples[sample_sel]);
    alSourcef(stream, AL_GAIN, volume);

    float pitch_min = 0.88;
    float pitch = pitch_min+randn1(int((1.f-pitch_min)*2.f*1000.f))*0.001;
    alSourcef(stream, AL_PITCH, pitch);
    alSourcePlay(stream);

    p_currentStream = (p_currentStream + 1) % Sound_game_static::hitStreamCount;
}
