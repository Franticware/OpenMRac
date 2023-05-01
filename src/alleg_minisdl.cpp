#include "alleg_minisdl.h"

#include <cstring>
#include <cstdio>

const char* SDL_GetKeyName(int key)
{
    return scancode_to_name(key);
}

static char keyPrev[KEY_MAX] = {0};
static char keyNow[KEY_MAX] = {0};
static int keyI = 0;

void preparePollEvent(void)
{
    memcpy(keyPrev, keyNow, KEY_MAX);
    for (int i = 0; i != KEY_MAX; ++i)
    {
        keyNow[i] = key[i];
    }
    keyI = 0;
}

int SDL_PollEvent(SDL_Event * event)
{
    while (keyI != KEY_MAX)
    {
        if (keyPrev[keyI] != keyNow[keyI])
        {
            event->type = keyPrev[keyI] == 0 ? SDL_KEYDOWN : SDL_KEYUP;
            event->key.keysym.sym = keyI;
            ++keyI;
            return 1;
        }
        else
        {
            ++keyI;
        }
    }
    return 0;
}
