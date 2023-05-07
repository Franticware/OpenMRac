#include "load_texture.h"
#include "bits.h"

#include <cstdio>

unsigned int g_texture_memory_usage = 0;
int g_textureFiltering = 0; // 0 - bilinear, 1 - trilinear, 2 - aniso
int g_textureMipmaps = 0; // 0 - off, 1 - on

static const GLint textureWrap = GL_REPEAT;

inline void checkGL(){}

static uint32_t memory = 0;

void subMem(int w, int h)
{
    memory -= w * h * 2;
}

GLuint load_texture(Pict2 pict, bool bmipmap)
{
    pict.cropNpotH();

    memory += pict.w() * pict.h() * 2;
    //printf("texmem usage: %u\n", memory); fflush(stdout);

    if (g_textureMipmaps == 0)
    {
        bmipmap = false;
    }
    GLint packAlignment;
    glGetIntegerv(GL_PACK_ALIGNMENT, &packAlignment);

    pict.pack16(packAlignment);

    if (bits_count(pict.w()) != 1 || bits_count(pict.h()) != 1)
        return 0;
    /*if (bits_count(pict.h()) != 1 && bits_count(pict.h()+1) != 1)
        return 0;*/
    {
        GLuint textura;
        glGenTextures(1, &textura); checkGL();
        glBindTexture(GL_TEXTURE_2D, textura); checkGL();

        for (int level = 0;;++level)
        {
            unsigned int pict_h = bits_crop_npot(pict.h());
            g_texture_memory_usage += 4*pict.w()*pict_h;
            if (pict.p_hasAlpha)
            {
                glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA, pict.w(), pict_h, 0, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, pict.c_ppx()); checkGL();
            }
            else
            {
                glTexImage2D(GL_TEXTURE_2D, level, GL_RGB, pict.w(), pict_h, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, pict.c_ppx()); checkGL();
            }
            if (!bmipmap || (pict_h == 1 && pict.w() == 1)) break;
            int newW = pict.w() >> 1;
            int newH = pict_h >> 1;
            if (newW == 0) newW = 1;
            if (newH == 0) newH = 1;
            pict.scale(newW, newH);
            pict.pack16(packAlignment);
        }

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, bmipmap ? (g_textureFiltering == 0 ? GL_LINEAR_MIPMAP_NEAREST : GL_LINEAR_MIPMAP_LINEAR) : GL_LINEAR); checkGL();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); checkGL();

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, textureWrap); checkGL();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, textureWrap); checkGL();

        glBindTexture(GL_TEXTURE_2D, 0); checkGL();
        return textura;
    }
    return 0;
}
