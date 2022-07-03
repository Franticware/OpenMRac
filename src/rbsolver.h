#ifndef HLIDAC_RBSOLVER_H
#define HLIDAC_RBSOLVER_H

#include "platform.h"

#include "3dm.h"
#include <cmath>

namespace RBf {
    inline bool equals(const float* u, const float* v)
    {
        return u[0] == v[0] && u[1] == v[1];
    }
    inline void zerov(float* ret)
    {
        ret[0] = 0.f;
        ret[1] = 0.f;
    }
    inline const float* zerov()
    {
        static const float ret[2] = {0.f, 0.f};
        return ret;
    }
    inline void cpy(float* ret, const float* v)
    {
        ret[0] = v[0];
        ret[1] = v[1];
    }
    // vector add
    inline float* add(float* ret, const float* u, const float* v)
    {
        ret[0] = u[0] + v[0];
        ret[1] = u[1] + v[1];
        return ret;
    }
    inline float* add(const float* u, const float* v)
    {
        static float ret[2];
        ret[0] = u[0] + v[0];
        ret[1] = u[1] + v[1];
        return ret;
    }
    inline float* ladd(float* ret, const float* v)
    {
        ret[0] += v[0];
        ret[1] += v[1];
        return ret;
    }
    // vector subtract
    inline float* sub(float* ret, const float* u, const float* v)
    {
        ret[0] = u[0] - v[0];
        ret[1] = u[1] - v[1];
        return ret;
    }
    inline float* sub(const float* u, const float* v)
    {
        static float ret[2];
        ret[0] = u[0] - v[0];
        ret[1] = u[1] - v[1];
        return ret;
    }
    inline float* lsub(float* ret, const float* v)
    {
        ret[0] -= v[0];
        ret[1] -= v[1];
        return ret;
    }
    // vector multiply
    inline float* mul(float* ret, const float* v, float a)
    {
        ret[0] = v[0]*a;
        ret[1] = v[1]*a;
        return ret;
    }
    inline float* mul(const float* v, float a)
    {
        static float ret[2];
        ret[0] = v[0]*a;
        ret[1] = v[1]*a;
        return ret;
    }
    inline float* lmul(float* ret, float a)
    {
        ret[0] *= a;
        ret[1] *= a;
        return ret;
    }
    // vector divide
    /* unsafe */ inline float* div(float* ret, const float* v, float a)
    {
        ret[0] = v[0]/a;
        ret[1] = v[1]/a;
        return ret;
    }
    /* unsafe */ inline float* div(const float* v, float a)
    {
        static float ret[2];
        ret[0] = v[0]/a;
        ret[1] = v[1]/a;
        return ret;
    }
    /* unsafe */ inline float* ldiv(float* ret, float a)
    {
        ret[0] /= a;
        ret[1] /= a;
        return ret;
    }
    inline float xprod(const float* u, const float* v)
    {
        return u[0]*v[1]-u[1]*v[0];
    }
    inline float dprod(const float* u, const float* v)
    {
        return u[0]*v[0]+u[1]*v[1];
    }
    inline float len2(const float* v) // délka vektoru^2
    {
        return v[0]*v[0]+v[1]*v[1];
    }
    inline float len(const float* v) // délka vektoru^2
    {
        return sqrtf(len2(v));
    }
    inline float safe(float f)
    {
        if (!isfinite1(f))
            return 0.f;
        return f;
    }
    inline void safe(float* v)
    {
        v[0] = safe(v[0]);
        v[1] = safe(v[1]);
    }
    inline void norm_s(float* v)
    {
        ldiv(v, len(v));
        safe(v);
    }
    inline void norm_s(float* v1, const float* v)
    {
        div(v1, v, len(v));
        safe(v1);
    }
    /* unsafe */ inline void norm(float* v)
    {
        ldiv(v, len(v));
    }
    /* unsafe */ inline void norm(float* v1, const float* v)
    {
        div(v1, v, len(v));
    }
    /* unsafe */ inline float dist(const float* x, const float* a, const float* v) // vzdálenost bodu od přímky //^2
    {
        return (xprod(v,x)+xprod(a,v))/len(v);
    }
    inline float distn(const float* x, const float* a, const float* v) // vzdálenost bodu od přímky
    {
        return xprod(v,x)+xprod(a,v);
    }
    inline void rot(float* m, float a) // vytvoření 2D rotační matice
    {
        m[0] = cosf(a);
        m[1] = sinf(a);
    }
    inline void rot(float* f, const float* m, const float* f0) // aplikace 2D rotační matice
    {
        f[0] = m[0]*f0[0]-m[1]*f0[1];
        f[1] = m[1]*f0[0]+m[0]*f0[1];
    }
    /* unsafe */ inline float intersect(const float* u, const float* b, const float* v) // průsečík přímek ([0], (u)) a ([b], (v))
    {
        return xprod(v,b)/xprod(v,u); // průsečík = výsledek*u
    }
    /* unsafe */ inline float intersect2(const float* u, const float* b, const float* v) // průsečík přímek ([0], (u)) a ([b], (v))
    {
        return xprod(b,u)/xprod(u,v); // průsečík = výsledek*u
    }

    /* unsafe */ inline float intersect(const float* a, const float* u, const float* b, const float* v) // průsečík přímek ([0], (u)) a ([b], (v))
    {
        float b2[2];
        sub(b2, b, a);
        return intersect(u, b2, v); // průsečík = výsledek*u
    }
    /* unsafe */ inline float intersect2(const float* a, const float* u, const float* b, const float* v) // průsečík přímek ([0], (u)) a ([b], (v))
    {
        float b2[2];
        sub(b2, b, a);
        return intersect2(u, b2, v); // průsečík = výsledek*u
    }

    inline bool line_intersect(const float* u, const float* b, const float* v)
    {
        float int_result  = intersect (u, b, v);
        float int_result2 = intersect2(u, b, v);
        if (int_result >= 0.f && int_result <= 1.f && int_result2 >= 0.f && int_result2 <= 1.f)
            return true;
        return false;
    }
    inline bool line_intersect(const float* a, const float* u, const float* b, const float* v)
    {
        float b2[2];
        sub(b2, b, a);
        float int_result  = intersect (u, b2, v);
        float int_result2 = intersect2(u, b2, v);
        if (int_result >= 0.f && int_result <= 1.f && int_result2 >= 0.f && int_result2 <= 1.f)
            return true;
        return false;
    }
    inline void intersect3(float* t, const float* a, const float* u, const float* b, const float* v)
    {
        float b2[2];
        sub(b2, b, a);
        t[0] = intersect (u, b2, v);
        t[1] = intersect2(u, b2, v);
    }
    inline void perp(float* ret, const float* v) // kolmý (perpendicular) vektor - kladný technický směr
    {
        ret[0] = -v[1];
        ret[1] = v[0];
    }
    inline void perp(float* v) // kolmý (perpendicular) vektor - kladný technický směr
    {
        float f;
        f = v[0];
        v[0] = -v[1];
        v[1] = f;
    }
    inline void neg(float* v)
    {
        v[0] = -v[0];
        v[1] = -v[1];
    }
    inline void neg(float* ret, const float* v)
    {
        ret[0] = -v[0];
        ret[1] = -v[1];
    }
}

void damp_v(float* p_v, float f/*součinitel smykového tření*/, float p_T);

class TimeSync {
public:
    void init(float T = 0.01);
    void add_deltaT(float deltaT);
    bool step();

    float p_prevT;
    float p_T;
};

struct BBox {
    float x[2]; // bod
    float v[2]; // vektor
    float v1[2]; // normalizovaný vektor
    float n1[2]; // normalizovaný normálový vektor bodu mířící dovnitř
    BBox* next; // ukazatel na další bod
    unsigned int bnconv; // je bod nekonvexní (např. roh objektu)
};

void bbox_create(BBox* &bbox, unsigned int& size, float* center, const T3dm* t3dm);

class RBSolver {
public:
    RBSolver() { p_bbox = 0; p_bbox_rot = 0; }
    ~RBSolver() { delete[] p_bbox; delete[] p_bbox_rot; }

    void init(const float* x, float ax, const float* v, float av, float m, float am, const TimeSync* timesync, const T3dm* t3dm);
    void addF(const float* F, const float* A = 0); // přidání další síly k výslednici, souřadnice světa (0 - síla působí v těžišti)
    void addFb(const float* F, const float* A = 0); // přidání další síly k výslednici, souřadnice objektu (0 - síla působí v těžišti)
    void update(bool bstep);

    void damp_v(float f);
    void damp_av(float f, float r);

    void get_tm(float* tm);

    void get_vp(float* vp, float* pp, const float* pt) const; // rychlost, bod v globál. souř., kolmý vektor od středu k bodu

    float v_a_dprod();

    const TimeSync* p_timesync;

    float p_x[2]; // polohový vektor
    float p_ax;   // úhel
    float p_x0[2]; // extrapolovaný polohový vektor
    float p_ax0;   // extrapolovaný úhel
    float p_x_prev[2]; // polohový vektor
    float p_ax_prev;   // úhel
    float p_v[2]; // rychlost
    float p_av;   // úhlová rychlost
    float p_a[2]; // zrychlení
    float p_aa;   // úhlové zrychlení
    float p_m;    // hmotnost
    float p_am;   // moment setrvačnosti
    float p_F[2]; // síla
    float p_aF;   // moment síly

    bool p_bupdm;
    float p_tm[2];
    BBox* p_bbox;
    BBox* p_bbox_rot;
    unsigned int p_bbox_sz;

    bool p_bcolprev; // v předchozím kroku byla kolize
    bool p_bcolnow;
};

#endif
