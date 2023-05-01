#ifndef HLIDAC_GLHELPERS1_H
#define HLIDAC_GLHELPERS1_H

#include <GL/gl.h>
#include <cmath>

//extern int g_multisampleMode;

#define ENABLE_CHECKGL 0

#if ENABLE_CHECKGL
#include <cassert>
#include <cstdio>

inline void checkGL()
{
    GLenum err = glGetError();
    assert(err == GL_NO_ERROR);
    if (err != GL_NO_ERROR)
    {
        fprintf(stderr, "gl err %d\n", err);
        fflush(stderr);
    }
}
#else
#define checkGL() ;
#endif

inline void setStandardAlphaTest(bool enable)
{
/*#if !defined(__MACOSX__) && !defined(__MORPHOS__)
    if (g_multisampleMode)
    {
        if (enable)
        {
            glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE_ARB);
            checkGL();
        }
        else
        {
            glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE_ARB);
            checkGL();
        }
    }
    else
#endif*/
    {
        if (enable)
        {
            glEnable(GL_ALPHA_TEST);
            checkGL();
        }
        else
        {
            glDisable(GL_ALPHA_TEST);
            checkGL();
        }
    }
}

inline void multMatVect(float result[4], const float matr[16], const float vect[4])
{
    result[0] = matr[0]*vect[0] + matr[4]*vect[1] + matr[8]*vect[2] + matr[12]*vect[3];
    result[1] = matr[1]*vect[0] + matr[5]*vect[1] + matr[9]*vect[2] + matr[13]*vect[3];
    result[2] = matr[2]*vect[0] + matr[6]*vect[1] + matr[10]*vect[2] + matr[14]*vect[3];
    result[3] = matr[3]*vect[0] + matr[7]*vect[1] + matr[11]*vect[2] + matr[15]*vect[3];
}

inline void multMatPos(float result[3], const float m[16], const float position[3])
{
    result[0] = m[0]*position[0] + m[4]*position[1] + m[8]*position[2] + m[12];
    result[1] = m[1]*position[0] + m[5]*position[1] + m[9]*position[2] + m[13];
    result[2] = m[2]*position[0] + m[6]*position[1] + m[10]*position[2] + m[14];
}

inline void multMatDir(float result[3], const float m[16], const float direction[3])
{
    result[0] = m[0]*direction[0] + m[4]*direction[1] + m[8]*direction[2];
    result[1] = m[1]*direction[0] + m[5]*direction[1] + m[9]*direction[2];
    result[2] = m[2]*direction[0] + m[6]*direction[1] + m[10]*direction[2];
}

inline void normalize(float vect[3])
{
    float l = sqrt(vect[0] * vect[0] + vect[1] * vect[1] + vect[2] * vect[2]);
    vect[0] /= l;
    vect[1] /= l;
    vect[2] /= l;
    if (!std::isfinite(vect[0]) || !std::isfinite(vect[1]) || !std::isfinite(vect[2]))
    {
        vect[0] = 0.f;
        vect[1] = 0.f;
        vect[2] = 1.f;
    }
}

#endif // HLIDAC_GLHELPERS1_H
