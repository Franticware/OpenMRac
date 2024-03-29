#ifndef HLIDAC_MATMNG_H
#define HLIDAC_MATMNG_H

#include "3dm.h"
#include "octopus.h"
#include "pict2.h"
#include "glm1.h"
#include "gl1.h"
#include "gl_shared.h"
#include <cstring>

class Transf {
public:
    void init(unsigned int i_from, unsigned int i_size)
    {
        p_ibegin = i_from;
        p_iend = i_from+i_size;
        p_mwmx.clear(); p_mwmx.resize(i_size);
    }
    void set_mwmx(const glm::mat4& mwmx, unsigned int i_pos)
    {
        if (i_pos >= p_ibegin && i_pos < p_iend)
        {
            p_mwmx[i_pos-p_ibegin] = mwmx;
        }
    }
    glm::mat4 mult_mwmx(unsigned int i_pos) const
    {
        // get multiplication matrix for sub-object
        if (i_pos >= p_ibegin && i_pos < p_iend)
        {
            return p_mwmx[i_pos-p_ibegin];
        }
        return glm::mat4(1);
    }

    unsigned int p_ibegin = 0;
    unsigned int p_iend = 0;
    std::vector<glm::mat4> p_mwmx;
};

class Mat {
public:
    Mat() : balpha_test(false), benv_map(false), bboth_side(false), blighting(false), bmipmap(false), bsunken(false), special(0) { texd_name[0] = 0; texa_name[0] = 0; sund_name[0] = 0; suna_name[0] = 0; }
    void load(const char* fname);
    void default_mat();
    void setgl();

    SharedGLtex texture; // id textury
    SharedGLtex texsunk; // id sunken textury
    bool balpha_test; // alfa test
    bool benv_map; // mapa prostředí
    bool bboth_side; // oboustranný materiál
    bool blighting;
    bool bmipmap; //
    bool bsunken;
    char texd_name[256]; // název difuzní textury
    char texa_name[256]; // název alfa textury
    char sund_name[256]; // název barvy sunken textury
    char suna_name[256]; // název alpha sunken textury
    int special; // zároveň bspecial; 0 - nothing, 1 - two-sided top car part, 2 - blended (fence around Speedway), 3 - shadow
    float color[4];
};

class Matmng {
public:
    void load(const T3dm* t3dm);
    const T3dm* p_t3dm = nullptr;
    std::vector<Mat> p_mat;
};

class Gamemng;

class Rendermng {
public:
    Rendermng() : p_t3dm(0), p_matmng(0), p_octopus(0), p_boctocube(false), b_visible(false), p_transf(0) { }
    void init(Gamemng* gamemng, const T3dm* t3dm, const Matmng* matmng, Octopus* octopus = 0);
    void set_oc(const float frustum[6], const T3dm& t3dm); // nastaví jednotlivou octocube (asi)
    void set_transf(const Transf* transf) { p_transf = transf; }
    void render_o_pass1(const float* modelview_matrix);
    void render_o_pass1_lim(const float* modelview_matrix, unsigned int face_limit);
    void render_o_pass2(const glm::mat4& m);
    void render_o_pass_glassTint(const glm::mat4& m);
    void render_o_pass_glassReflection(const glm::mat4& m);
    void render_o_pass3();
    void render_o_pass_s2();
    void render_o_pass_s3();
    bool isVisible() const
    {
        return !(p_boctocube && !b_visible);
    }
    const T3dm* p_t3dm;
    const Matmng* p_matmng;
    Gamemng* p_gamemng;
    Octopus* p_octopus; // pokud == 0 -> nepoužito
    bool p_boctocube;
    Octocube p_octocube;
    bool b_visible;
    Octopus p_octocube_base;
    SharedGLtex p_skycmtex;
    const Transf* p_transf;
};

#endif
