#include "platform.h"

#include <SDL2/SDL.h>

double GetSystemTime()
{
    return SDL_GetTicks();
}

void getdeltaT_init()
{
}

float getdeltaT()
{
    static bool bfirstrun = true;
    static double tm_prev;
    if (bfirstrun)
    {
        tm_prev = (double)GetSystemTime()/1000.f;
        bfirstrun = false;
        return 0.f;
    }
    double tm_now = (double)GetSystemTime()/1000.f;
    double deltaT = tm_now-tm_prev;
    tm_prev = tm_now;
    if (deltaT < 0.f)
        return 0.f;
    return deltaT;
}
