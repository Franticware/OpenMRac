#ifndef GL1_H
#define GL1_H

#if defined(__WIN32__)

#include <GL/GLee.h>

#define USE_GL_COMPAT 1
#define USE_GL_CORE3 0
#define USE_GL_ES2 0 // does not work on windows

#else

#define GL_GLEXT_PROTOTYPES
#include <GL/glcorearb.h>

#define USE_GL_COMPAT 0
#define USE_GL_CORE3 0
#define USE_GL_ES2 1

#endif

#ifndef GL_MAX_TEXTURE_MAX_ANISOTROPY
#define GL_MAX_TEXTURE_MAX_ANISOTROPY GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT
#endif
#ifndef GL_TEXTURE_MAX_ANISOTROPY
#define GL_TEXTURE_MAX_ANISOTROPY GL_TEXTURE_MAX_ANISOTROPY_EXT
#endif

#define ASSERT_ANISOTROPY

#ifndef ENABLE_CHECKGL
#define ENABLE_CHECKGL 0
#endif

#if ENABLE_CHECKGL
#include <cassert>
#include <cstdio>

inline void checkGL()
{
    GLenum err = glGetError();
    //assert(err == GL_NO_ERROR);
    if (err != GL_NO_ERROR)
    {
        fprintf(stderr, "gl err %d\n", err);
        fflush(stderr);
    }
}
#else
#define checkGL() ;
#endif

extern int g_multisampleMode;

/*
inline void setStandardAlphaTest(bool enable)
{
#if !defined(__MACOSX__) && !defined(__MORPHOS__)
    if (g_multisampleMode)
    {
        if (enable)
        {
            glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE_ARB);
            checkGL();
        }
        else
        {
            glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE_ARB);
            checkGL();
        }
    }
    else
#endif
    {
        if (enable)
        {
            glEnable(GL_ALPHA_TEST);
            checkGL();
        }
        else
        {
            glDisable(GL_ALPHA_TEST);
            checkGL();
        }
    }
}
*/

#endif // GL1_H
