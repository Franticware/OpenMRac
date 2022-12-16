#ifndef HLIDAC_COLLIDER_H
#define HLIDAC_COLLIDER_H

#include "3dm.h"
#include "rbsolver.h"
#include "soundmng.h"
#include <cstring>

inline bool isintersect(const float* t)
{
    return t[0] >= 0.f && t[0] <= 1.f && t[1] >= 0.f && t[1] <= 1.f;
}

class ColliderGrid {
public:
    std::vector<const BBox*> points; // pole ukazatelů na vrcholy
    unsigned int  points_sz = 0;
    std::vector<const BBox*> lines; // pole ukazatelů na úsečky
    unsigned int  lines_sz = 0;
};

class Collider {
public:
    Collider() : p_gs(0), p_r(0), p_r_m2(0), p_r_m2_p2(0), p_colg(0), p_rbos(0), p_rbos_sz(0), p_bbox_sz(0), p_players(0), p_sound_crash(0) { }
    void init(float gs, float r, const T3dm* t3dm, RBSolver** rbos);
    void test();

    void render(float x, float y);
    int get_subg(const float pos[2]) const;

    bool test_line(const unsigned int pos[2], unsigned int iline);
    bool test_point(const unsigned int pos[2], unsigned int ipoint);

    void create_grid(const unsigned int pos[2],
    std::vector<const BBox*> &ilines, std::vector<const BBox*> &ipoints, unsigned int &lines_sz, unsigned int &points_sz, bool pass2 = false);

    float p_gs; // stana čtverečku
    float p_r;
    float p_r_m2;
    float p_r_m2_p2;

    float p_base[2];
    float p_top[2];
    unsigned int p_sz[2];

    std::vector<ColliderGrid> p_colg; // čtvercová síť s příslušnými kolizními objekty
    float p_cen[2]; // střed

    RBSolver** p_rbos;
    unsigned int p_rbos_sz;

    std::vector<BBox> p_bbox;
    unsigned int p_bbox_sz;

    unsigned int p_players; // počet hráčů ve hře, kvůli zvuku

    Sound_crash* p_sound_crash;
};

#endif
