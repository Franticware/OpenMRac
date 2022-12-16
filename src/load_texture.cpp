#include "load_texture.h"
#include "glext1.h"
#include "glhelpers1.h"
#include "bits.h"
#include "platform.h"

#if !USE_GENERATE_MIPMAP
#ifdef __MACOSX__
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif
#endif

unsigned int g_texture_memory_usage = 0;

extern int g_mipmaps_available;

#ifndef USE_GENERATE_MIPMAP
#error
#endif

#if USE_GENERATE_MIPMAP
#ifndef GL_GENERATE_MIPMAP
#define GL_GENERATE_MIPMAP                0x8191
#endif
#endif // USE_GENERATE_MIPMAP

#ifdef __MORPHOS__
// const void * to GLvoid * cast
#undef glTexImage2D
#define glTexImage2D(target, level, internalFormat, width, height, border, format, type, data) \
    GLTexImage2D(__tglContext, (target), (level), (internalFormat), (width), (height), (border), (format), (type), (GLvoid *)(data))
GLint gluBuild2DMipmaps_stub(GLenum target, GLint component, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *data)
{
    return gluBuild2DMipmaps(target, component, width, height, format, type, (GLvoid *)data);
}
#define gluBuild2DMipmaps gluBuild2DMipmaps_stub
#endif

extern int g_textureFiltering;

static const GLint textureWrap =
    GL_REPEAT
    //GL_CLAMP_TO_EDGE
    ;

GLuint load_texture(const Pict2& pict, bool bmipmap)
{
    if (bits_count(pict.w()) != 1)
        return 0;
    if (bits_count(pict.h()) != 1 && bits_count(pict.h()+1) != 1)
        return 0;
    if (pict.d() == 3 || pict.d() == 4)
    {
        GLuint textura;
        glGenTextures(1, &textura); checkGL();
        glBindTexture(GL_TEXTURE_2D, textura); checkGL();
        if (!bmipmap || !g_mipmaps_available) // no mipmaps
        {
            unsigned int pict_h = pict.h();
            if (bits_count(pict.h()) != 1 && bits_count(pict.h()+1) == 1) // předpřipravené mipmapy
                pict_h = (pict.h()+1)/2;
            glTexImage2D(GL_TEXTURE_2D, 0, pict.d(), pict.w(), pict_h, 0,
                (pict.d() == 3) ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, pict.c_px()); checkGL();
            g_texture_memory_usage += pict.d()*pict.w()*pict_h;
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); checkGL();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); checkGL();

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, textureWrap); checkGL();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, textureWrap); checkGL();
        } else { // mipmaps
            if (bits_count(pict.h()) != 1 && bits_count(pict.h()+1) == 1) // předpřipravené mipmapy
            {
                unsigned int pict_h = (pict.h()+1)/2;
                glTexImage2D(GL_TEXTURE_2D, 0, pict.d(), pict.w(), pict_h, 0,
                    (pict.d() == 3) ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, pict.c_px()); checkGL();
                unsigned int pict_w = pict.w()/2;
                unsigned int start_y = pict_h;
                pict_h /= 2;
                std::vector<unsigned char> data(pict_w*pict_h*pict.d());
                unsigned int miplevel = 1;
                while (pict_w || pict_h)
                {
                    for (unsigned int x = 0; x != pict_w; ++x)
                    {
                        for (unsigned int y = 0; y != pict_h; ++y)
                        {
                            memcpy(data.data()+(x+y*pict_w)*pict.d(), pict.c_px(x, y+start_y), pict.d());
                        }
                    }

                    glTexImage2D(GL_TEXTURE_2D, miplevel, pict.d(), std::max(pict_w, (unsigned int)1), std::max(pict_h, (unsigned int)1), 0,
                        (pict.d() == 3) ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, data.data()); checkGL();
                    g_texture_memory_usage += pict.d()*pict.w()*pict_h;
                    start_y += pict_h;
                    pict_w /= 2;
                    pict_h /= 2;
                    ++miplevel;
                }
            } else {
#if USE_GENERATE_MIPMAP
                glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE); checkGL();
#endif              
                glTexImage2D(GL_TEXTURE_2D, 0, pict.d(), pict.w(), pict.h(), 0,
                    (pict.d() == 3) ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, pict.c_px()); checkGL();
#if !USE_GENERATE_MIPMAP
                gluBuild2DMipmaps(GL_TEXTURE_2D, pict.d(), pict.w(), pict.h(),
                    (pict.d() == 3) ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, pict.c_px()); checkGL();
#endif
                g_texture_memory_usage += pict.d()*pict.w()*pict.h()+pict.d()*pict.w()*pict.h()/2;
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

GLuint load_texture_alpha(const Pict2& pict, bool bmipmap)
{
    if (pict.d() == 1)
    {
        GLuint textura;
        glGenTextures(1, &textura); checkGL();
        glBindTexture(GL_TEXTURE_2D, textura); checkGL();
        if (!bmipmap || !g_mipmaps_available) // no mipmaps
        {
            glColor4f(1, 1, 1, 1); checkGL();
            glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, pict.w(), pict.h(), 0,
                GL_ALPHA, GL_UNSIGNED_BYTE, pict.c_px()); checkGL();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); checkGL();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); checkGL();

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, textureWrap); checkGL();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, textureWrap); checkGL();

            g_texture_memory_usage += pict.w()*pict.h();
        } else { // mipmaps
#if USE_GENERATE_MIPMAP
            glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE); checkGL();
#endif
            glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, pict.w(), pict.h(), 0,
                GL_ALPHA, GL_UNSIGNED_BYTE, pict.c_px()); checkGL();
#if !USE_GENERATE_MIPMAP
            gluBuild2DMipmaps(GL_TEXTURE_2D, GL_ALPHA, pict.w(), pict.h(),
                GL_ALPHA, GL_UNSIGNED_BYTE, pict.c_px()); checkGL();
#endif
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, g_textureFiltering > 0 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR_MIPMAP_NEAREST); checkGL();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); checkGL();

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, textureWrap); checkGL();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, textureWrap); checkGL();

            g_texture_memory_usage += pict.w()*pict.h()+pict.w()*pict.h()/2;
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
    if (pict.d() != 3)
        return 0;
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
            3,            //internal format
            pict.w(),                 //width
            pict.w(),                 //height
            0,                  //border
            GL_RGB,             //format
            GL_UNSIGNED_BYTE,   //type
            pict.c_px()+pict.w()*pict.w()*3*i); checkGL(); // pixel data
        g_texture_memory_usage += pict.w()*pict.w()*3;
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR); checkGL();
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR); checkGL();
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); checkGL();
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); checkGL();
    glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, 0); checkGL();
    return ret;
}
