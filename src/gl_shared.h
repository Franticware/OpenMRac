#ifndef GL_SHARED_H
#define GL_SHARED_H

#include "gl1.h"
#include <cstdint>
#include <cassert>
//#include <cstdio>

/*
 * Based on Madhu Raykar's 2006 article "Implementing a simple smart pointer in C++"
 * Things were way simpler back then...
 */

template <auto DelFun> class SharedGLobj
{
private:
    class RC
    {
    private:
        uint32_t count; // reference count

    public:
        void AddRef()
        {
            // increment the reference count
            count++;
        }

        int Release()
        {
            assert(count > 0);
            // decrement the reference count and
            // return the reference count.
            return --count;
        }
    };

    GLuint pData;  // OpenGL object name, 0 - no object
    RC* reference; // reference count

    void create()
    {
        if (pData)
        {
            reference = new RC();
            reference->AddRef();
            //printf("ctor %u\n", pValue); fflush(stdout);
        }
    }

    void destroy()
    {
        if (pData)
        {
            assert(reference);
            if (reference->Release() == 0)
            {
                DelFun(1, &pData);
                checkGL();
                delete reference;
                //printf("dtor %u\n", pData); fflush(stdout);
            }
        }
    }

public:
    SharedGLobj() : pData(0), reference(nullptr)
    {
        create();
    }

    SharedGLobj(GLuint pValue) : pData(pValue), reference(nullptr)
    {
        create();
    }

    SharedGLobj(const SharedGLobj<DelFun>& sp) : pData(sp.pData), reference(sp.reference)
    {
        if (pData)
        {
            assert(reference);
            reference->AddRef();
        }
    }

    ~SharedGLobj()
    {
        destroy();
    }

    SharedGLobj<DelFun>& operator = (const SharedGLobj<DelFun>& sp)
    {
        if (this != &sp) // avoid self assignment
        {
            destroy();
            pData = sp.pData;
            reference = sp.reference;
            if (pData)
            {
                assert(reference);
                reference->AddRef();
            }
        }
        return *this;
    }

    operator GLuint() const { return pData; }
};

using SharedGLtex = SharedGLobj<glDeleteTextures>;
using SharedGLbuf = SharedGLobj<glDeleteBuffers1>;

#endif // GL_SHARED_H
