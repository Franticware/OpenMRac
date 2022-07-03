#include "platform.h"
#include "glhelpers1.h"
#include "glext1.h"
#include "cstring1.h"

#include <SDL/SDL.h>

/*Ext1_glActiveTexture ext1_glActiveTexture = 0;

bool Glext::init_ARB_multitexture()
{
    if (!strstrtok1((const char*)glGetString(GL_EXTENSIONS), "GL_ARB_multitexture"))
        return false;
    ext1_glActiveTexture = (Ext1_glActiveTexture)SDL_GL_GetProcAddress("glActiveTexture");
    if (ext1_glActiveTexture == 0)
        return false;
    return true;
}*/

bool Glext::init_ARB_texture_cube_map()
{
    if (!strstrtok1((const char*)glGetString(GL_EXTENSIONS), "GL_ARB_texture_cube_map"))
    {
        checkGL();
        return false;
    }
    checkGL();
    return true;
}

#if !defined(__MACOSX__) && !defined(__MORPHOS__)
Ext1_glSampleCoverageARB ext1_glSampleCoverageARB = 0;
#endif

bool Glext::init_ARB_multisample()
{
    if (!strstrtok1((const char*)glGetString(GL_EXTENSIONS), "GL_ARB_multisample"))
    {
        checkGL();
        return false;
    }
    checkGL();
#if !defined(__MACOSX__) && !defined(__MORPHOS__)
    ext1_glSampleCoverageARB = (Ext1_glSampleCoverageARB)SDL_GL_GetProcAddress("glSampleCoverageARB");
    if (ext1_glSampleCoverageARB == 0)
        return false;
#endif
    return true;
}
