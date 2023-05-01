#ifndef ALLEG_MINISDL_H
#define ALLEG_MINISDL_H

#include <cstdint>
#include <allegro.h>

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

#define SDLK_RIGHT KEY_RIGHT
#define SDLK_LEFT KEY_LEFT
#define SDLK_UP KEY_UP
#define SDLK_DOWN KEY_DOWN
#define SDLK_ESCAPE KEY_ESC
#define SDLK_RETURN KEY_ENTER
#define SDLK_KP_ENTER KEY_ENTER_PAD
#define SDLK_F12 KEY_F12
#define SDLK_PRINT KEY_PRTSCR
#define SDLK_NUMLOCK KEY_NUMLOCK
#define SDLK_DELETE KEY_DEL

#define SDLK_LAST KEY_MAX

typedef int SDLKey;

typedef struct SDL_keysym
{
    //uint8_t scancode;
    SDLKey sym;
    //SDLMod mod;
    //Uint16 unicode;
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

void preparePollEvent(void);

int SDL_PollEvent(SDL_Event * event);
const char* SDL_GetKeyName(int key);

inline void SDL_LockAudio(void) {}
inline void SDL_UnlockAudio(void) {}

#endif // ALLEG_MINISDL_H
