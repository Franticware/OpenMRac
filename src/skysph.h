#ifndef HLIDAC_SKYSPH_H
#define HLIDAC_SKYSPH_H

#include "platform.h"

#include "pict2.h"
#ifndef __MACOSX__
#include <GL/gl.h>
#else
#include <OpenGL/gl.h>
#endif
#include "glhelpers1.h"

#include <cstdint>
#include <vector>

class Skysph
{
public:
    Skysph() : r(0), ang(0), size(0), tex_sky(0) { }
    void init(float r_prm, float ang_prm, int h = 40, int v = 10);
    void set_tex(GLuint tex_sky_prm) { tex_sky = tex_sky_prm; }
    void set_light_pos();
    void render();
    float r;
    float ang;
    std::vector<float> vert;
    std::vector<uint16_t> tris;
    unsigned int size;
    GLuint tex_sky;
    float light_pos[4];
};

#endif
