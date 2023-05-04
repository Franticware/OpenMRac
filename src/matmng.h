#ifndef HLIDAC_MATMNG_H
#define HLIDAC_MATMNG_H

#include <GL/gl.h>
#include <cstring>

#ifndef SETKEYS
    #include "3dm.h"
    #include "octopus.h"
#endif
#include "glhelpers1.h"
#include "pict2.h"

extern int g_hq_textures;

class Transf {
public:
    Transf() : p_ibegin(0), p_iend(0), p_mwmx(0) { }
    ~Transf() { delete[] p_mwmx; }
    void init(unsigned int i_from, unsigned int i_size)
    {
        p_ibegin = i_from;
        p_iend = i_from+i_size;
        p_mwmx = new float[16*i_size];
    }
    void set_mwmx(const float* mwmx, unsigned int i_pos)
    {
        if (i_pos >= p_ibegin && i_pos < p_iend)
            memcpy(p_mwmx+(i_pos-p_ibegin)*16, mwmx, sizeof(float)*16);
    }
    void mult_mwmx(unsigned int i_pos) const
    {
        if (i_pos >= p_ibegin && i_pos < p_iend)
        {
            glMultMatrixf(p_mwmx+(i_pos-p_ibegin)*16); checkGL();
        }
    }

    unsigned int p_ibegin;
    unsigned int p_iend;
    float* p_mwmx;
};

class Mat {
public:
    Mat() : texture(0), balpha_test(false), benv_map(false), bboth_side(false), blighting(false), bmipmap(false), special(0) { texd_name[0] = 0; texa_name[0] = 0; }
    ~Mat() { glDeleteTextures(1, &texture); checkGL(); texture = 0; }
    void load(const char* fname);
    void default_mat();
    void setgl();

    GLuint texture; // id textury
    bool bhiquality;
    bool balpha_test; // alfa test
    bool benv_map; // mapa prostředí
    bool bboth_side; // oboustranný materiál
    bool blighting;
    bool bmipmap; //
    char texd_name[256]; // název difuzní textury
    char texa_name[256]; // název alfa textury
    int special; // zároveň bspecial
    float color[4];
};

#ifndef SETKEYS
class Matmng {
public:
    Matmng() : p_t3dm(0), p_mat(0), p_m_sz(0), p_bstatic_light(false), p_vcolor(0), p_vcolor_back(0) { }
    ~Matmng() { delete[] p_mat; delete[] p_vcolor; delete[] p_vcolor_back; }
    void load(const T3dm* t3dm, const float* ambcolor = 0, const float* diffcolor = 0, const float* lightpos = 0);

    const T3dm* p_t3dm;
    Mat* p_mat;
    unsigned int p_m_sz;
    bool p_bstatic_light; // not dynamic lighting?
    float* p_vcolor; //
    float* p_vcolor_back;
};

class Rendermng {
public:
    Rendermng() : p_t3dm(0), p_matmng(0), p_octopus(0), p_boctocube(false), b_visible(false), /*p_skycmtex(0),*/ p_transf(0) { }
    ~Rendermng() {  }
    void init(const T3dm* t3dm, const Matmng* matmng, Octopus* octopus = 0);
    void set_oc(const float frustum[6], const T3dm& t3dm); // nastaví jednotlivou octocube (asi)
    void set_transf(const Transf* transf) { p_transf = transf; }
    void render_o_pass1(const float* modelview_matrix);
    void render_o_pass1_lim(const float* modelview_matrix, unsigned int face_limit);
    void render_o_pass2();
    void render_o_pass3();
    void render_o_pass_s2();
    void render_o_pass_s3();

    bool isVisible() const
    {
        return !(p_boctocube && !b_visible);
    }

    const T3dm* p_t3dm;
    const Matmng* p_matmng;
    Octopus* p_octopus; // pokud == 0 -> nepoužito
    bool p_boctocube;
    Octocube p_octocube;
    bool b_visible;
    Octopus p_octocube_base;
    // rozhodnout se, kdo bude vlastníkem objektů

    //GLuint p_skycmtex;

    const Transf* p_transf;
};
#endif

#endif
