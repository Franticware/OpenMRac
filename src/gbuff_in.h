#ifndef HLIDAC_GBUFF_IN_H
#define HLIDAC_GBUFF_IN_H

#include <cstring>
#include <cstdio>
#include "datkey.h"

class Gbuff_in {
public:
    Gbuff_in() : p_bactive(false), p_bdat(false), p_bbin(false), p_buff_pos(0), p_fin(0), p_stdin_buff_ptr(0), p_stdin_buff_sz(0) { }
    ~Gbuff_in() { delete[] p_stdin_buff_ptr; if (p_fin) ::fclose(p_fin); }
    void init_stdin() { p_bdat = false; }
    bool init_dat(const char* fname);
    // společné fce
    bool f_open(const char* fname, const char* mode); /* "r" nebo "rb" */
    void fclose(); // normálně int fclose(FILE*)
    // fce pro práci s textovým souborem
    char* fgets(char* str, int num); // normálně char * fgets ( char * str, int num, FILE * stream )
    void rewind();
    // fce pro práci s binárním souborem
    /*const*/ unsigned char* fbuffptr();
    unsigned int fbuffsz();
private:
    bool p_bactive;
    bool p_bdat; // načítání z datového souboru místo normálních souborů
    bool p_bbin; // načítání z binárního bufferu
    Datdec p_datdec;
    unsigned int p_buff_pos;
    FILE* p_fin;
    unsigned char* p_stdin_buff_ptr;
    unsigned int p_stdin_buff_sz;
};

extern Gbuff_in gbuff_in;

#endif
