#ifndef HLIDAC_3DM_H
#define HLIDAC_3DM_H

#include <cstring>
#include <cstdint>
#include <vector>
#include <string>

class O3dm {
public:
    std::vector<uint16_t> p_i; // pole indexů faců
    unsigned int p_m = 0; // id materiálu
    unsigned int p_gi = 0; // index transformační skupiny
};

class T3dm {
public:
    void load(const char* fname, const char** o_names = 0); // grouping names (0 - model je statický, transformace konečná)
    void scale(float aspect);
    int getgidobj(unsigned int gid) const; // najde první objekt s číslem skupiny, pokud ne, vrátí -1
    std::vector<float> p_v;    // 0:v0 1:v1 2:v2   3:n0 4:n1 5:n2   6:t0 7:t1
    std::vector<O3dm> p_o; // objekty
    std::vector<std::string> p_m; // materiály
    std::vector<float> p_cen; // středy objektů
    void clear() { p_v.clear(); p_o.clear(); p_m.clear(); p_cen.clear(); }
};

#endif
