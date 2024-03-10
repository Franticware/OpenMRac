#ifndef MINISDL_H
#define MINISDL_H

#include <cstdint>
//#include <allegro.h>
#include "scancodes.h"

typedef uint16_t Uint16;
typedef int16_t Sint16;
typedef uint32_t Uint32;
typedef uint8_t Uint8;

typedef enum SDL_EventType
{
    SDL_NOEVENT,
    SDL_ACTIVEEVENT,
    SDL_KEYDOWN,
    SDL_KEYUP,
    SDL_MOUSEMOTION,
    SDL_MOUSEBUTTONDOWN,
    SDL_MOUSEBUTTONUP,
    SDL_JOYAXISMOTION,
    SDL_JOYBALLMOTION,
    SDL_JOYHATMOTION,
    SDL_JOYBUTTONDOWN,
    SDL_JOYBUTTONUP,
    SDL_QUIT,
    SDL_SYSWMEVENT,
    SDL_EVENT_RESERVEDA,
    SDL_EVENT_RESERVEDB,
    SDL_VIDEORESIZE,
    SDL_VIDEOEXPOSE,
    SDL_EVENT_RESERVED2,
    SDL_EVENT_RESERVED3,
    SDL_EVENT_RESERVED4,
    SDL_EVENT_RESERVED5,
    SDL_EVENT_RESERVED6,
    SDL_EVENT_RESERVED7,
    SDL_USEREVENT = 24,
    SDL_NUMEVENTS = 32
} SDL_EventType;

#define SDLK_RIGHT K_RIGHT
#define SDLK_LEFT K_LEFT
#define SDLK_UP K_UP
#define SDLK_DOWN K_DOWN
#define SDLK_ESCAPE K_ESC
#define SDLK_RETURN K_ENTER
#define SDLK_KP_ENTER K_NENTER
#define SDLK_F12 K_F12
/*#define SDLK_PRINT KEY_PRTSCR*/
#define SDLK_NUMLOCK K_NLOCK
#define SDLK_DELETE K_DEL

#define SDLK_LAST KEYB_SIZE

typedef int SDLKey;

typedef struct SDL_keysym
{
    SDLKey sym;
} SDL_keysym;

typedef struct SDL_KeyboardEvent
{
    uint8_t type;
    uint8_t which;
    uint8_t state;
    SDL_keysym keysym;
} SDL_KeyboardEvent;

typedef union SDL_Event
{
    uint8_t type;
    SDL_KeyboardEvent key;
} SDL_Event;

int SDL_PollEvent(SDL_Event * event);
const char* SDL_GetKeyName(int key);

inline void SDL_LockAudio(void) {}
inline void SDL_UnlockAudio(void) {}

#endif // MINISDL_H
