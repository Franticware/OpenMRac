#ifndef HLIDAC_PLATFORM_H
#define HLIDAC_PLATFORM_H

// Visual C++
#if defined(_MSC_VER)
    #include <windows.h>
        #undef min
    #undef max
    #define snprintf _snprintf
    #define M_PI		3.14159265358979323846
#endif

// Visual C++
#if defined(_MSC_VER)
#include <float.h>
#define isfinite1 _finite

// mingw
#elif defined(__WIN32__) && defined(__MINGW32__)
#include <cmath>
#define isfinite1 std::isfinite

// gcc v linuxu
#elif defined(__unix__) || defined(__linux__) || defined(__HAIKU__) || defined(__amigaos4__) || defined(__MORPHOS__) || defined(__MACOSX__)
#include <cmath>
#define isfinite1 std::isfinite

// neznámá platforma
#else
#error unknown platform
#endif

#if defined(__MORPHOS__) // I don't know define for AROS
// use framerate limiter instead
#define USE_VSYNC 0
#else
#define USE_VSYNC 1
#endif

#endif // HLIDAC_PLATFORM_H
