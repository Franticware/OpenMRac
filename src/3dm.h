#ifndef HLIDAC_3DM_H
#define HLIDAC_3DM_H

#include <cstring>

class O3dm {
public:
    O3dm():p_i(0),p_sz(0),p_m(0),p_gi(0){}
    ~O3dm(){delete[] p_i;}
    unsigned short* p_i; // pole indexů faců
    unsigned int p_sz; // počet indexů
    unsigned int p_m; // id materiálu
    unsigned int p_gi; // index transformační skupiny
};


class T3dm {
public:
    typedef char Matname[256];
    void load_(const char* fname, const char** o_names = 0); // grouping names (0 - model je statický, transformace konečná)
    void load(const char* fname, const char** o_names = 0);
    void scale(float aspect);
    int getgidobj(unsigned int gid) const; // najde první objekt s číslem skupiny, pokud ne, vrátí -1
    T3dm() : p_v(0), p_n(0), p_t(0), p_v_sz(0), p_o(0), p_sz(0), p_m(0), p_m_sz(0), p_cen(0) { }
    ~T3dm(){delete[] p_v; delete[] p_n; delete[] p_t; delete[] p_o; delete[] p_cen; delete[] p_m;}

    float* p_v; // v0 v1 v2
    float* p_n; // n0 n1 n2
    float* p_t; // t0 t1
    unsigned int p_v_sz; // počet vertexů (= normál, texcoords)
    O3dm* p_o; // objekty
    unsigned int p_sz; // počet objektů
    Matname* p_m;
    unsigned int p_m_sz; // počet textur
    float* p_cen; // translační matice objektů
    void uncreate() {delete[] p_v; delete[] p_n; delete[] p_t; delete[] p_o; delete[] p_cen;
        p_v=0;p_n=0;p_t=0;p_o=0;p_sz=0;p_cen=0;}
};

#endif
