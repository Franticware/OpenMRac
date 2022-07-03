#include "platform.h"

#ifndef HLIDAC_GLEXT1_H
#define HLIDAC_GLEXT1_H

#ifndef __MACOSX__
#include <GL/gl.h>
#ifndef __MORPHOS__
#include <GL/glext.h>
#endif // __MORPHOS__
#else
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#endif // __MACOSX__

//typedef void (APIENTRY * Ext1_glActiveTexture)(GLenum);

#if defined(__amigaos4__) || defined(__MORPHOS__)
#define DISABLE_ANISOTROPY
#else
#define ASSERT_ANISOTROPY
#endif

#if !defined(__MACOSX__) && !defined(__MORPHOS__)
typedef void (APIENTRY * Ext1_glSampleCoverageARB)(GLclampf value, GLboolean invert);
#endif

class Glext {
public:
    //bool init_ARB_multitexture();
    bool init_ARB_texture_cube_map();
    bool init_ARB_multisample();
};

#if !defined(__MACOSX__) && !defined(__MORPHOS__)
/*extern Ext1_glActiveTexture ext1_glActiveTexture;
#define glActiveTexture ext1_glActiveTexture*/

extern Ext1_glSampleCoverageARB ext1_glSampleCoverageARB;
#define glSampleCoverageARB ext1_glSampleCoverageARB
#endif

#endif
