#ifndef HLIDAC_DATKEY_H
#define HLIDAC_DATKEY_H

#include <cstring>
#include <cstdio>
#include <vector>

struct Datfile {
    char p_fname[1024];
    unsigned int p_size;
    unsigned int p_offset;
};

class Datdec { // tar file reader
public:
    bool init(const char* fname);
    bool load(const char* fname);
    char p_fname[1024];
    std::vector<Datfile> p_files;
    std::vector<unsigned char> p_buff;
};

#endif
