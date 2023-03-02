#ifndef GL1_H
#define GL1_H

#define PROFILE_COMPAT 0
#define PROFILE_ES2 1
#define PROFILE_CORE33 2

#define PROFILE_MIN PROFILE_COMPAT
#define PROFILE_MAX PROFILE_CORE33

#if defined(__WIN32__)

//#define GLEW_STATIC
#include <GL/glew.h>

inline bool initGlExt(void)
{
    return glewInit() == GLEW_OK;
}

#define GLEXT_ERROR_MESSAGE "glewInit error"

// GLES2 did not work on windows
#define DEFAULT_PROFILE PROFILE_COMPAT

#define GLDEBUG_APIENTRY __attribute__((stdcall))

#else

#define GL_GLEXT_PROTOTYPES
#include <GL/glcorearb.h>

inline bool initGlExt(void)
{
    return true;
}

#define GLEXT_ERROR_MESSAGE ""

#define DEFAULT_PROFILE PROFILE_ES2

#define GLDEBUG_APIENTRY APIENTRY

#endif // __WIN32__

#define ENABLE_GLDEBUG 0
#define ENABLE_glslangValidator 0

extern int g_opengl_profile;

inline void glDepthRange1(float a, float b)
{
    if (g_opengl_profile == PROFILE_ES2)
        glDepthRangef(a, b);
    else
        glDepthRange(a, b);
}

#undef glDepthRangef
#define glDepthRangef use_glDepthRange1_instead_of_glDepthRangef
#undef glDepthRange
#define glDepthRange use_glDepthRange1_instead_of_glDepthRange

inline void glDeleteBuffers1(GLsizei n, const GLuint * buffers)
{
    // workaround for GLEW (win32), no harm on other platforms
    glDeleteBuffers(n, buffers);
}

#ifndef GL_MAX_TEXTURE_MAX_ANISOTROPY
#define GL_MAX_TEXTURE_MAX_ANISOTROPY GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT
#endif
#ifndef GL_TEXTURE_MAX_ANISOTROPY
#define GL_TEXTURE_MAX_ANISOTROPY GL_TEXTURE_MAX_ANISOTROPY_EXT
#endif

#define ASSERT_ANISOTROPY

#if ENABLE_CHECKGL
#include <cassert>
#include <cstdio>

#define checkGL_case(a) case a: fprintf(stderr, "gl err %s (0x%x)\n", #a, a); break

inline void checkGL()
{
    GLenum err = glGetError();
    //assert(err == GL_NO_ERROR);
    if (err != GL_NO_ERROR)
    {
        switch (err)
        {
        checkGL_case(GL_INVALID_OPERATION);
        checkGL_case(GL_INVALID_ENUM);
        checkGL_case(GL_INVALID_VALUE);
        default: fprintf(stderr, "gl err %u (0x%x)\n", err, err); break;
        }
        fflush(stderr);
    }
}

#undef checkGL_case

#else
inline void checkGL() { }
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
