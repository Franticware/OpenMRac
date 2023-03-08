#ifndef ERROR_MSG_H
#define ERROR_MSG_H

#if defined(__WIN32__)

#include <SDL2/SDL.h>

extern SDL_Window* gameWindow;

#define error_msg(...) do { \
    char buff[256] = {0}; snprintf(buff, 255, __VA_ARGS__); \
    size_t len = strlen(buff); \
    if (len > 0 && buff[len - 1] == '\n') buff[len - 1] = 0; \
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", buff, gameWindow); \
} while (0)

#else

#include <cstdio>
#define error_msg(...) do { fprintf(stderr, __VA_ARGS__); fflush(stderr); } while (0)

#endif

#endif // ERROR_MSG_H
