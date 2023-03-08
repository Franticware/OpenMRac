#include "ghost.h"
#include "fopendir.h"
#include "appdefs.h"
#include <cstdio>
#include <algorithm>
#include <memory>

Ghost::Ghost(bool frames) {
    m_version = 104;
    m_track = 0;
    m_reverse = 0;
    m_car = 0;
    m_carcolor = 0;
    m_seconds = 0;
    m_maxnum = 1800;
    m_num = 0;
    m_frames.resize(frames ? m_maxnum*4 : 0);
}

bool Ghost::load(int track, int reverse) {
    m_track = track;
    m_reverse = reverse;
    char filename[1024];
    getfname(filename);
    std::unique_ptr<FILE, decltype(&fclose)> fin(fopenDir(filename, "rb", OPENMRAC_ORG, OPENMRAC_APP), &fclose);
    if (fin.get() == nullptr) return false;
    if (fread(&m_version, sizeof(int), 1, fin.get()) != 1) return false;
    if (m_version != 104) return false;
    if (fread(&m_track, sizeof(int), 1, fin.get()) != 1) return false;
    if (m_track != track) return false;
    if (fread(&m_reverse, sizeof(int), 1, fin.get()) != 1) return false;
    if (m_reverse != reverse) return false;
    if (fread(&m_car, sizeof(int), 1, fin.get()) != 1) return false;
    if (m_car < 0 || m_car >= 3) return false;
    if (fread(&m_carcolor, sizeof(int), 1, fin.get()) != 1) return false;
    if (m_carcolor < 0 || m_carcolor >= 4) return false;
    if (fread(&m_seconds, sizeof(float), 1, fin.get()) != 1) return false;
    if (m_seconds < 10.0 || m_seconds > 31536000.0) return false;
    if (!m_frames.empty())
    {
        if (fread(&m_num, sizeof(int), 1, fin.get()) != 1) return false;
        if (m_num < 0 || m_num > m_maxnum) return false;
        if (static_cast<int>(fread(m_frames.data(), sizeof(float)*4, m_num, fin.get())) != m_num) return false;
    }
    return true;
}

void Ghost::save() {
    char filename[1024];
    getfname(filename);
    FILE* fout = fopenDir(filename, "wb", OPENMRAC_ORG, OPENMRAC_APP);
    if (fout == NULL) return;
    fwrite(&m_version, sizeof(int), 1, fout);
    fwrite(&m_track, sizeof(int), 1, fout);
    fwrite(&m_reverse, sizeof(int), 1, fout);
    fwrite(&m_car, sizeof(int), 1, fout);
    fwrite(&m_carcolor, sizeof(int), 1, fout);
    fwrite(&m_seconds, sizeof(float), 1, fout);
    if (!m_frames.empty())
    {
        fwrite(&m_num, sizeof(int), 1, fout);
        fwrite(m_frames.data(), sizeof(float)*4, m_num, fout);
    }
    fclose(fout);
}

void Ghost::getfname(char* buff) { // 1024 bajtů
    buff[1023] = 0;
    const char* filenames[4] = {"speedway", "boulevard", "suburb", "ironworks"};
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
    if (m_frames.empty() || gnew.m_frames.empty())
    {
        m_num = 0;
    }
    else
    {
        m_num = gnew.m_num;
        std::copy_n(gnew.m_frames.begin(), m_num * 4, m_frames.begin());
    }
}
