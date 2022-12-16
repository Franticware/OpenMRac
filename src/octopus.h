#ifndef HLIDAC_OCTOPUS_H
#define HLIDAC_OCTOPUS_H

#include "3dm.h"

#include <cstdint>
#include <vector>
#include <memory>

class Octocm {
public:
    std::vector<uint16_t> p_i;
    unsigned int p_sz = 0;
};

class Octopus;

class Octocube {
public:
    Octocube();
    int test(const float modelview_matrix[16]); // 0 - mimo, 1 - plně, 2 - část
    void subdiv();
    void count_maxsub(unsigned int* count);

    void render_level_pom(unsigned int lev);

    int render_pass1(const float modelview_matrix[16]);
    int render_pass1_(const float modelview_matrix[16]);
    void render_pass2();

    std::vector<Octocm> p_mi; // indexy příslušné materiálům
    std::unique_ptr<Octocube> p_sub[8]; // tam, kde je 0, nejsou další tris
    Octocube* p_up; // předek této octocube
    float p_cen[3]; // střed bounding sphere
    float p_cen_t[3]; // transformovaný střed (pro sort)
    float p_r; // poloměr bounding sphere
    float p_a_2; // polovina strany krychle
    unsigned int p_lev; // úroveň dělení
    Octopus* p_base; // základní předek všech octocube

    unsigned int p_face_num; // počet faců v octocube
};

class Octopus {
public:
    Octopus() : p_near(0), p_far(0), p_vw(0), p_vw_sz(0), p_t3dm(nullptr), p_m_sz(0), p_min_tris(0), p_max_lev(0)
    {
        for (int i = 0; i != 2; ++i)
        {
            p_left[i] = p_right[i] = p_bottom[i] = p_top[i] = 0;
        }
    }
    void init(const float frustum[6], const T3dm& t3dm, unsigned int min_tris, unsigned int max_lev);
    void init_frustum(const float frustum[6]);
    void render_bbox();
    void render_level(unsigned int lev);

    void render_pass1(const float modelview_matrix[16]);
    void render_pass1_lim(const float modelview_matrix[16], unsigned int face_limit); // začátek testování bboxů
    void render_pass2();

    float p_left[2]; // hranice frustumu
    float p_right[2];
    float p_bottom[2];
    float p_top[2];
    float p_near;
    float p_far;
    std::unique_ptr<Octocube> p_oc; // základní krychle (octocube)
    std::vector<Octocube*> p_vw; // pole ukazatelů na octocube pro poznamenání nalezených octocube
    unsigned int p_vw_sz; // počet nalezených položek
    const T3dm* p_t3dm;
    unsigned int p_m_sz; // počet materiálů

    unsigned int p_min_tris, p_max_lev;
};

#endif
