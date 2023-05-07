#ifndef HLIDAC_LOAD_TEXTURE_H
#define HLIDAC_LOAD_TEXTURE_H

#include <GL/gl.h>
#include "pict2.h"

GLuint load_texture(Pict2 pict, bool bmipmap = false);

void subMem(int w, int h);

extern unsigned int g_texture_memory_usage;
extern int g_textureFiltering;
extern int g_textureMipmaps;

#endif
