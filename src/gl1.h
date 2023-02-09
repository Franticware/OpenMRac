#ifndef GL1_H
#define GL1_H

#if defined(__WIN32__)
#error "TODO: Find a way to link GLES2 on Win32"
#else
#define GL_GLEXT_PROTOTYPES
#include <GL/glcorearb.h>
#endif

#endif // GL1_H
