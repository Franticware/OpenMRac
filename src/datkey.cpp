#include "platform.h"

#include "datkey.h"

#include <SDL/SDL_endian.h>
#include <cstdio>
#include <cstring>
#include <algorithm>

bool Datdec::load(const char* fname)
{
    unsigned int i = 0;
    while (i != p_files.size())
    {
        if (strcmp(p_files[i].p_fname, fname) == 0)
            break;
        ++i;
    }
    if (i == p_files.size())
    {
        p_buff.clear();
        return false;
    }
    FILE* fin = fopen(p_fname, "rb");
    fseek(fin, p_files[i].p_offset, SEEK_SET);
    p_buff.resize(p_files[i].p_size);
    size_t newSize = fread(&p_buff[0], 1, p_files[i].p_size, fin);
    p_buff.resize(newSize);
    fclose(fin);
    return true;
}

struct TarHeader
{                              /* byte offset */
  char name[100];               /*   0 */
  char mode[8];                 /* 100 */
  char uid[8];                  /* 108 */
  char gid[8];                  /* 116 */
  char size[12];                /* 124 */
};

bool Datdec::init(const char* fname)
{
    strncpy(p_fname, fname, 1023);
    FILE* fin = fopen(p_fname, "rb");
    if (fin == NULL)
        return false;
    unsigned char tarBlock[512];
    TarHeader tarHeader;
    Datfile f;
    size_t offset = 0;
    for (;;)
    {
        if (fread(tarBlock, 512, 1, fin) != 1)
        {
            fclose(fin);
            return true;
        }
        offset += 512;
        memcpy(&tarHeader, tarBlock, sizeof(TarHeader));
        if (tarHeader.name[0] == 0)
        {
            fclose(fin);
            return true;
        }
        f.p_fname[99] = 0;
        strncpy(f.p_fname, tarHeader.name, 99);
        int fileSize = 0;
        if (sscanf(tarHeader.size, "%o", &fileSize) != 1)
        {
            fclose(fin);
            return false;
        }
        f.p_size = fileSize;
        f.p_offset = offset;
        int blocks = fileSize / 512 + !!(fileSize % 512);
        fseek(fin, blocks * 512, SEEK_CUR);
        offset += blocks * 512;
        p_files.push_back(f);
    }
    return true;
}
