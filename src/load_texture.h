#ifndef HLIDAC_LOAD_TEXTURE_H
#define HLIDAC_LOAD_TEXTURE_H

#include <GL/gl.h>
#include "pict2.h"

GLuint load_texture(const Pict2& pict, bool bmipmap = false);

void subMem(int w, int h);

extern unsigned int g_texture_memory_usage;

#endif
