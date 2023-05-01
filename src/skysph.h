#ifndef HLIDAC_SKYSPH_H
#define HLIDAC_SKYSPH_H

#include "pict2.h"
#include <GL/gl.h>
#include "glhelpers1.h"

class Skysph
{
public:
    Skysph() : r(0), ang(0), vert(0), texc(0), quads(0), size(0), bsun(false), tex_sky(0)
    {
        for (int i = 0; i != 4*3; ++i)
        {
            sun_v[i] = 0;
        }
        for (int i = 0; i != 4*2; ++i)
        {
            sun_t[i] = 0;
        }
    }
    ~Skysph() { delete[] vert; delete[] texc; delete[] quads; /* delete textures dodělat*//*glDeleteTextures(1, &tex_sky); checkGL(); glDeleteTextures(1, &tex_sun); checkGL();*/ }
    void init(float r_prm, float ang_prm, int h = 40, int v = 10);
    //void set_sun(const Pict2& pict);
    //void set_sun_(const Pict2& pict);
    void set_tex(GLuint tex_sky_prm) { tex_sky = tex_sky_prm; }
    void set_light_pos();
    void render();
    float r;
    float ang;
    float* vert;
    float* texc;
    unsigned short* quads;
    unsigned int size;
    bool bsun;
    float sun_v[4*3];
    float sun_t[4*2];
    GLuint tex_sky;
    //GLuint tex_sun;
    float light_pos[4];
};

#endif
