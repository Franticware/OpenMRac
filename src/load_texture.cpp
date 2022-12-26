#include "load_texture.h"
#include "glext1.h"
#include "glhelpers1.h"
#include "bits.h"

unsigned int g_texture_memory_usage = 0;

extern int g_mipmaps_available;

extern int g_textureFiltering;

static const GLint textureWrap = GL_REPEAT;

GLuint load_texture(const Pict2& pict, bool bmipmap)
{
    if (bits_count(pict.w()) != 1)
        return 0;
    if (bits_count(pict.h()) != 1 && bits_count(pict.h()+1) != 1)
        return 0;
    {
        GLuint textura;
        glGenTextures(1, &textura); checkGL();
        glBindTexture(GL_TEXTURE_2D, textura); checkGL();
        if (!bmipmap || !g_mipmaps_available) // no mipmaps
        {
            unsigned int pict_h = pict.h();
            if (bits_count(pict.h()) != 1 && bits_count(pict.h()+1) == 1) // předpřipravené mipmapy
                pict_h = (pict.h()+1)/2;
            glTexImage2D(GL_TEXTURE_2D, 0, 4, pict.w(), pict_h, 0, GL_BGRA, GL_UNSIGNED_BYTE, pict.c_px()); checkGL();
            g_texture_memory_usage += 4*pict.w()*pict_h;
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); checkGL();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); checkGL();

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, textureWrap); checkGL();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, textureWrap); checkGL();
        } else { // mipmaps
            if (bits_count(pict.h()) != 1 && bits_count(pict.h()+1) == 1) // předpřipravené mipmapy
            {
                unsigned int pict_h = (pict.h()+1)/2;
                glTexImage2D(GL_TEXTURE_2D, 0, 4, pict.w(), pict_h, 0, GL_BGRA, GL_UNSIGNED_BYTE, pict.c_px()); checkGL();
                unsigned int pict_w = pict.w()/2;
                unsigned int start_y = pict_h;
                pict_h /= 2;
                std::vector<unsigned char> data(pict_w*pict_h*4);
                unsigned int miplevel = 1;
                while (pict_w || pict_h)
                {
                    for (unsigned int x = 0; x != pict_w; ++x)
                    {
                        for (unsigned int y = 0; y != pict_h; ++y)
                        {
                            memcpy(data.data()+(x+y*pict_w)*4, pict.c_px(x, y+start_y), 4);
                        }
                    }

                    glTexImage2D(GL_TEXTURE_2D, miplevel, 4, std::max(pict_w, (unsigned int)1), std::max(pict_h, (unsigned int)1), 0, GL_BGRA, GL_UNSIGNED_BYTE, data.data()); checkGL();
                    g_texture_memory_usage += 4*pict.w()*pict_h;
                    start_y += pict_h;
                    pict_w /= 2;
                    pict_h /= 2;
                    ++miplevel;
                }
            } else {
                glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE); checkGL();
                glTexImage2D(GL_TEXTURE_2D, 0, 4, pict.w(), pict.h(), 0, GL_BGRA, GL_UNSIGNED_BYTE, pict.c_px()); checkGL();
                g_texture_memory_usage += 4*pict.w()*pict.h()+4*pict.w()*pict.h()/2;
            }
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, g_textureFiltering > 0 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR_MIPMAP_NEAREST); checkGL();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); checkGL();

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, textureWrap); checkGL();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, textureWrap); checkGL();
        }
        if (g_textureFiltering == 2)
        {
            #ifndef DISABLE_ANISOTROPY
            ASSERT_ANISOTROPY
            GLfloat maxAniso = 0.0f;
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso); checkGL();
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAniso); checkGL();
            #endif
        }
        glBindTexture(GL_TEXTURE_2D, 0); checkGL();
        return textura;
    }
    return 0;
}

GLuint load_texture_cube_map(const Pict2& pict)
{
    if (pict.w()*6 != pict.h())
        return 0;
    GLuint ret = 0;
    glGenTextures(1, &ret); checkGL();
    glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, ret); checkGL();
    static const GLenum faceTarget[6] = {
        GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB,
        GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB
        };
    for (unsigned int i = 0; i != 6; ++i)
    {
        glTexImage2D(faceTarget[i],
            0,                  //level
            4,            //internal format
            pict.w(),                 //width
            pict.w(),                 //height
            0,                  //border
            GL_BGRA,             //format
            GL_UNSIGNED_BYTE,   //type
            pict.c_px()+pict.w()*pict.w()*4*i); checkGL(); // pixel data
        g_texture_memory_usage += pict.w()*pict.w()*4;
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR); checkGL();
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR); checkGL();
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); checkGL();
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); checkGL();
    glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, 0); checkGL();
    return ret;
}
