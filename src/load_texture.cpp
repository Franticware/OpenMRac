#include "load_texture.h"
#include "bits.h"

#include <cstdio>

unsigned int g_texture_memory_usage = 0;

extern int g_textureFiltering;

static const GLint textureWrap = GL_REPEAT;

inline void checkGL(){}

static uint32_t memory = 0;

void subMem(int w, int h)
{
    memory -= w * h * 2;
}

GLuint load_texture(const Pict2& pict, bool /*bmipmap*/)
{
    memory += pict.w() * pict.h() * 2;
    //printf("texmem usage: %u\n", memory); fflush(stdout);

    if (bits_count(pict.w()) != 1)
        return 0;
    if (bits_count(pict.h()) != 1 && bits_count(pict.h()+1) != 1)
        return 0;
    {
        GLuint textura;
        glGenTextures(1, &textura); checkGL();
        glBindTexture(GL_TEXTURE_2D, textura); checkGL();
        //if (!bmipmap) // no mipmaps
        {
            unsigned int pict_h = pict.h();
            if (bits_count(pict.h()) != 1 && bits_count(pict.h()+1) == 1) // předpřipravené mipmapy
                pict_h = (pict.h()+1)/2;

            if (pict.packed565)
            {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, pict.w(), pict_h, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, pict.c_px()); checkGL();
            }
            else
            {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pict.w(), pict_h, 0, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, pict.c_px()); checkGL();
            }


            g_texture_memory_usage += 4*pict.w()*pict_h;
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); checkGL();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); checkGL();

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, textureWrap); checkGL();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, textureWrap); checkGL();
        }
        glBindTexture(GL_TEXTURE_2D, 0); checkGL();
        return textura;
    }
    return 0;
}
