#include "platform.h"
#include "glhelpers1.h"
#include "skysph.h"
#include <cmath>
#ifndef __MACOSX__
#include <GL/gl.h>
#else
#include <OpenGL/gl.h>
#endif

/* orientace úhlu */
/*
    +------- x
    |
    |
    |
    z -> ang
*/

void Skysph::set_light_pos()
{
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos); checkGL();
}

void Skysph::init(float r_prm, float ang_prm, int h, int v) // úhel ve stupních
{
    float ah = ang_prm/180.f*M_PI;
    float av = M_PI*0.25f;
    light_pos[0] = sinf(ah)*cosf(av);
    light_pos[1] = sinf(av);
    light_pos[2] = cosf(ah)*cosf(av);
    light_pos[3] = 0.f;
    r = r_prm;
    ang = ang_prm;
    vert = new float[(v*(h+1)+1)*3];
    texc = new float[(v*(h+1)+1)*2];
    quads = new unsigned short[v*h*4];
    size = v*(h)*4;
    for (int x = 0; x != h+1; ++x)
    {
        for (int y = 0; y != v; ++y)
        {
            float uhel_h = ang/180.f*M_PI+M_PI*2.f/float(h)*float(x)+M_PI;
            float uhel_v = (-M_PI/2.f)/10.f+(M_PI/2.f)*1.1/float(v)*float(y);
            vert[(x+y*(h+1))*3  ] = sinf(uhel_h)*cosf(uhel_v)*r;
            vert[(x+y*(h+1))*3+1] = sinf(uhel_v)*r;
            vert[(x+y*(h+1))*3+2] = cosf(uhel_h)*cosf(uhel_v)*r;
            texc[(x+y*(h+1))*2  ] = /*uhel_h/M_PI/2.f*/float(x)/float(h);
            texc[(x+y*(h+1))*2+1] = float(uhel_v/M_PI*2.f*2.f); //std::max(std::min(float(uhel_v/M_PI*2.f*2.f), /*1.f*/255.f/256.f), 1.f/256.f);
            int y1 = y+1;
            int x1 = (x+1);//%h;
            if (x != h)
            {
                if (y1 == v)
                {
                    quads[(x+y*(h))*4+3] = x+y*(h+1);
                    quads[(x+y*(h))*4+2] = x1+y*(h+1);
                    quads[(x+y*(h))*4+1] = v*(h+1);
                    quads[(x+y*(h))*4  ] = v*(h+1);
                } else {
                    quads[(x+y*(h))*4+3] = x+y*(h+1);
                    quads[(x+y*(h))*4+2] = x1+y*(h+1);
                    quads[(x+y*(h))*4+1] = x1+y1*(h+1);
                    quads[(x+y*(h))*4  ] = x+y1*(h+1);
                }
            }
        }
    }
    vert[(v*(h+1))*3  ] = 0.f;
    vert[(v*(h+1))*3+1] = r;
    vert[(v*(h+1))*3+2] = 0.f;
    texc[(v*(h+1))*2  ] = 0.5f;
    texc[(v*(h+1))*2+1] = 1.f;
}

void Skysph::render()
{
    glEnableClientState(GL_VERTEX_ARRAY); checkGL();
    glEnableClientState(GL_TEXTURE_COORD_ARRAY); checkGL();
    glVertexPointer(3, GL_FLOAT, 0, vert); checkGL();
    glTexCoordPointer(2, GL_FLOAT, 0, texc); checkGL();
    glBindTexture(GL_TEXTURE_2D, tex_sky); checkGL();
    glDrawElements(GL_QUADS, size, GL_UNSIGNED_SHORT, quads); checkGL();
    if (bsun)
    {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); checkGL();
        glEnable(GL_BLEND); checkGL();
        glVertexPointer(3, GL_FLOAT, 0, sun_v); checkGL();
        glTexCoordPointer(2, GL_FLOAT, 0, sun_t); checkGL();
        glBindTexture(GL_TEXTURE_2D, tex_sun); checkGL();
        glDrawArrays(GL_QUADS, 0, 4); checkGL();
        glDisable(GL_BLEND); checkGL();
    }
    glDisableClientState(GL_VERTEX_ARRAY); checkGL();
    glDisableClientState(GL_TEXTURE_COORD_ARRAY); checkGL();
}

inline void xprod3(float* n, const float* u, const float* v)
{
    n[0] = u[1]*v[2]-u[2]*v[1];
    n[1] = u[2]*v[0]-u[0]*v[2];
    n[2] = u[0]*v[1]-u[1]*v[0];
}

inline void norm3(float* f)
{
    float len = sqrtf(f[0]*f[0]+f[1]*f[1]+f[2]*f[2]);
    f[0] /= len;
    f[1] /= len;
    f[2] /= len;
}

inline void safev3(float* f, const float* alt)
{
    if (!isfinite1(f[0]) || !isfinite1(f[1]) || !isfinite1(f[2]))
    {
        memcpy(f, alt, sizeof(float)*3);
    }
}

void Skysph::set_sun_(const Pict2& pict)
{
    if (pict.d() < 3)
        return;
    float xf = 0, yf = 0;
    float nf = 0;
    for (int x = 0; x != pict.w(); ++x)
    {
        for (int y = 0; y != pict.h(); ++y)
        {
            if (pict.c_px(x, y)[0] == 0 && pict.c_px(x, y)[1] == 255 && pict.c_px(x, y)[2] == 0)
            {
                bsun = true;
                xf += x+0.5f;
                yf += y+0.5f;
                nf += 1.f;
            }
        }
    }
    if (bsun)
    {
        xf /= nf*pict.w();
        yf /= nf*pict.h();
        float h = xf*M_PI*2.f+M_PI+ang/180.f*M_PI;
        float v = yf*M_PI/2.f;
        float n3[3] = {sinf(h)*cosf(v), sinf(v), cosf(h)*cosf(v)};
        light_pos[3] = 0.f;
        float t3[3] = {0, 1, 0};
        float h3[3];
        xprod3(h3, n3, t3);
        norm3(h3);
        float defh3[3] = {0, 0, 1};
        safev3(h3, defh3);
        float v3[3];
        xprod3(v3, h3, n3);
        norm3(v3);
        float sr = 0.12;
        v3[0] *= sr;
        v3[1] *= sr;
        v3[2] *= sr;
        h3[0] *= sr;
        h3[1] *= sr;
        h3[2] *= sr;
        sun_t[0] = 0;
        sun_t[1] = 0;
        sun_t[2] = 1;
        sun_t[3] = 0;
        sun_t[4] = 1;
        sun_t[5] = 1;
        sun_t[6] = 0;
        sun_t[7] = 1;
        sun_v[0] = n3[0]-h3[0]-v3[0];
        sun_v[1] = n3[1]-h3[1]-v3[1];
        sun_v[2] = n3[2]-h3[2]-v3[2];
        sun_v[3] = n3[0]+h3[0]-v3[0];
        sun_v[4] = n3[1]+h3[1]-v3[1];
        sun_v[5] = n3[2]+h3[2]-v3[2];
        sun_v[6] = n3[0]+h3[0]+v3[0];
        sun_v[7] = n3[1]+h3[1]+v3[1];
        sun_v[8] = n3[2]+h3[2]+v3[2];
        sun_v[9] = n3[0]-h3[0]+v3[0];
        sun_v[10]= n3[1]-h3[1]+v3[1];
        sun_v[11]= n3[2]-h3[2]+v3[2];
        for (int i = 0; i != 12; ++i)
        {
            sun_v[i] *= r;
        }
    }
}

void Skysph::set_sun(const Pict2& pict)
{
    if (pict.d() < 3)
        return;
    float xf = 0, yf = 0;
    float nf = 0;
    for (int x = 0; x != pict.w(); ++x)
    {
        for (int y = 0; y != pict.h(); ++y)
        {
            if (pict.c_px(x, y)[0] == 0 && pict.c_px(x, y)[1] == 255 && pict.c_px(x, y)[2] == 0)
            {
                bsun = true;
                xf += x+0.5f;
                yf += y+0.5f;
                nf += 1.f;
            }
        }
    }
    if (bsun)
    {
        xf /= nf*pict.w();
        yf /= nf*pict.h();
        float h = xf*M_PI*2.f+M_PI+ang/180.f*M_PI;
        float v = yf*M_PI/2.f/2.f;
        float n3[3] = {sinf(h)*cosf(v), sinf(v), cosf(h)*cosf(v)};
        light_pos[3] = 0.f;
        float t3[3] = {0, 1, 0};
        float h3[3];
        xprod3(h3, n3, t3);
        norm3(h3);
        float defh3[3] = {0, 0, 1};
        safev3(h3, defh3);
        float v3[3];
        xprod3(v3, h3, n3);
        norm3(v3);
        float sr = 0.12;
        v3[0] *= sr;
        v3[1] *= sr;
        v3[2] *= sr;
        h3[0] *= sr;
        h3[1] *= sr;
        h3[2] *= sr;
        sun_t[0] = 0;
        sun_t[1] = 0;
        sun_t[2] = 1;
        sun_t[3] = 0;
        sun_t[4] = 1;
        sun_t[5] = 1;
        sun_t[6] = 0;
        sun_t[7] = 1;
        sun_v[0] = n3[0]-h3[0]-v3[0];
        sun_v[1] = n3[1]-h3[1]-v3[1];
        sun_v[2] = n3[2]-h3[2]-v3[2];
        sun_v[3] = n3[0]+h3[0]-v3[0];
        sun_v[4] = n3[1]+h3[1]-v3[1];
        sun_v[5] = n3[2]+h3[2]-v3[2];
        sun_v[6] = n3[0]+h3[0]+v3[0];
        sun_v[7] = n3[1]+h3[1]+v3[1];
        sun_v[8] = n3[2]+h3[2]+v3[2];
        sun_v[9] = n3[0]-h3[0]+v3[0];
        sun_v[10]= n3[1]-h3[1]+v3[1];
        sun_v[11]= n3[2]-h3[2]+v3[2];
        for (int i = 0; i != 12; ++i)
        {
            sun_v[i] *= r;
        }
    }
}
