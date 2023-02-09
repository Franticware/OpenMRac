#ifndef HLIDAC_LOAD_TEXTURE_H
#define HLIDAC_LOAD_TEXTURE_H

#include "gl1.h"
#include "pict2.h"

GLuint load_texture(const Pict2& pict, bool bmipmap = false);
GLuint load_texture_cube_map(const Pict2& pict);

extern unsigned int g_texture_memory_usage;

#endif
