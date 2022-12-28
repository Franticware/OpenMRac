#ifndef HLIDAC_PLATFORM_H
#define HLIDAC_PLATFORM_H

#define isfinite1 std::isfinite

#if defined(__MORPHOS__) // I don't know define for AROS
// use framerate limiter instead
#define USE_VSYNC 0
#else
#define USE_VSYNC 1
#endif

#endif // HLIDAC_PLATFORM_H
