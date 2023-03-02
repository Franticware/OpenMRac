#ifndef HLIDAC_SKYSPH_H
#define HLIDAC_SKYSPH_H

#include "pict2.h"
#include "gl1.h"
#include "glm1.h"
#include "gl_shared.h"

#include <cstdint>
#include <vector>

class Gamemng;

class Skysph
{
public:
    Skysph() : r(0), ang(0), size(0) { }
    void init(Gamemng* gamemng, float r_prm, float ang_prm, int h = 40, int v = 10);
    void set_tex(SharedGLtex tex_sky_prm) { tex_sky = tex_sky_prm; }
    void render(const glm::mat4& sky_mat);
    float r;
    float ang;
    std::vector<float> vert;
    SharedGLbuf arrayBuf;
    std::vector<uint16_t> tris;
    SharedGLbuf elemBuf;
    unsigned int size;
    SharedGLtex tex_sky;

    float light_pos[4];
    Gamemng* p_gamemng;
};

#endif
