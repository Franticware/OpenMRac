#ifndef PLATFORM_H
#define PLATFORM_H

// #if 0
#ifndef __DJGPP__

#include <AL/al.h>
#include <AL/alc.h>

#include <chrono>
#include <thread>

static const ALCint* attribs = 0;

void delay_s(int s)
{
    std::this_thread::sleep_for(std::chrono::seconds(s));
}

#else

#include "../minial.h"
#include <ctime>
#include <dos.h>

static const ALCint attribs[] = {ALC_FREQUENCY, 22050, ALC_MONO_SOURCES, 14, 0, 0};

void delay_s(int s)
{
    uclock_t start = uclock();
    while (uclock() < start + UCLOCKS_PER_SEC * s)
    {
        MA_periodicStream();
        delay(1);
    }
}

#endif

#endif // PLATFORM_H
