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

enum class T3dmA {
    Pos0 = 0,
    Pos1,
    Pos2,
    Tex0,
    Tex1,
    Norm0,
    Norm1,
    Norm2,
    Tan0,
    Tan1,
    Tan2,
    Bitan0,
    Bitan1,
    Bitan2,
    //Dummy0,
    //Dummy1,
    Count
};

class T3dm {
public:
    void load(const char* fname, const char** o_names = 0); // grouping names (0 - model je statický, transformace konečná)
    void scale(float aspect);
    int getgidobj(unsigned int gid) const; // najde první objekt s číslem skupiny, pokud ne, vrátí -1
    std::vector<float> p_v; // viz T3dmA
    std::vector<O3dm> p_o; // objekty
    std::vector<std::string> p_m; // materiály
    std::vector<float> p_cen; // středy objektů
    void clear() { p_v.clear(); p_o.clear(); p_m.clear(); p_cen.clear(); }
};

#endif
