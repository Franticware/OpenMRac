#include "ghost.h"
#include "fopendir.h"
#include "appdefs.h"
#include <cstdio>

Ghost::Ghost(bool frames) {
    m_version = 104;
    m_track = 0;
    m_reverse = 0;
    m_car = 0;
    m_carcolor = 0;
    m_seconds = 0;
    m_maxnum = 1800;
    m_num = 0;
    m_frames = frames ? new float[m_maxnum*4] : 0;
}

Ghost::~Ghost() {
    delete[] m_frames;
}

bool Ghost::load(int track, int reverse) {
    m_track = track;
    m_reverse = reverse;
    char filename[1024];
    getfname(filename);
    FILE* fin = fopenDir(filename, "rb", OPENMRAC_ORG, OPENMRAC_APP);
    //fprintf(stderr, "%s_%s\n", __PRETTY_FUNCTION__, filename);
    if (fin == NULL) return false;
    if (fread(&m_version, sizeof(int), 1, fin) != 1) {fclose(fin); return false;}
    if (m_version != 104) {fclose(fin); return false;}
    if (fread(&m_track, sizeof(int), 1, fin) != 1) {fclose(fin); return false;}
    if (m_track != track) {fclose(fin); return false;}
    if (fread(&m_reverse, sizeof(int), 1, fin) != 1) {fclose(fin); return false;}
    if (m_reverse != reverse) {fclose(fin); return false;}
    if (fread(&m_car, sizeof(int), 1, fin) != 1) {fclose(fin); return false;}
    if (m_car < 0 || m_car >= 3) {fclose(fin); return false;}
    if (fread(&m_carcolor, sizeof(int), 1, fin) != 1) {fclose(fin); return false;}
    if (m_carcolor < 0 || m_carcolor >= 4) {fclose(fin); return false;}
    if (fread(&m_seconds, sizeof(float), 1, fin) != 1) {fclose(fin); return false;}
    if (m_seconds < 10.0 || m_seconds > 31536000.0) {fclose(fin); return false;}
    if (m_frames)
    {
        if (fread(&m_num, sizeof(int), 1, fin) != 1) {fclose(fin); return false;}
        if (m_num < 0 || m_num > m_maxnum) {fclose(fin); return false;}
        if (static_cast<int>(fread(m_frames, sizeof(float)*4, m_num, fin)) != m_num) {fclose(fin); return false;}
    }
    fclose(fin);
    return true;
}

void Ghost::save() {
    char filename[1024];
    getfname(filename);
    //FILE* fout = fopen(filename, "wb");
    FILE* fout = fopenDir(filename, "wb", OPENMRAC_ORG, OPENMRAC_APP);
    //fprintf(stderr, "%s_%s\n", __PRETTY_FUNCTION__, filename);
    if (fout == NULL) return;
    fwrite(&m_version, sizeof(int), 1, fout);
    fwrite(&m_track, sizeof(int), 1, fout);
    fwrite(&m_reverse, sizeof(int), 1, fout);
    fwrite(&m_car, sizeof(int), 1, fout);
    fwrite(&m_carcolor, sizeof(int), 1, fout);
    fwrite(&m_seconds, sizeof(float), 1, fout);
    if (m_frames)
    {
        fwrite(&m_num, sizeof(int), 1, fout);
        fwrite(m_frames, sizeof(float)*4, m_num, fout);
    }
    fclose(fout);
}

void Ghost::getfname(char* buff) { // 1024 bajtů
    buff[1023] = 0;
    const char* filenames[4] = {"spdw", "blvd", "sbrb", "iwrk"};
    snprintf(buff, 1023, "%s%s.mrr", filenames[m_track], m_reverse ? "-rev" : "");
}

void Ghost::copyFrom(const Ghost& gnew) {
    m_version = gnew.m_version;
    m_track = gnew.m_track;
    m_reverse = gnew.m_reverse;
    m_car = gnew.m_car;
    m_carcolor = gnew.m_carcolor;
    m_seconds = gnew.m_seconds;
    m_maxnum = gnew.m_maxnum;
    if (m_frames == 0 || gnew.m_frames == 0)
    {
        m_num = 0;
    }
    else
    {
        m_num = gnew.m_num;
        memcpy(m_frames, gnew.m_frames, m_num*sizeof(float)*4);
    }
}
