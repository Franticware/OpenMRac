#ifndef GL_SHARED_H
#define GL_SHARED_H

#include "gl1.h"
#include <cstdint>
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
        uint32_t count; // Reference count

    public:
        void AddRef()
        {
            // Increment the reference count
            count++;
        }

        uint32_t Count() const { return count; }

        int Release()
        {
            assert(count > 0);
            // Decrement the reference count and
            // return the reference count.
            return --count;
        }
    };
    GLuint pData;       // pointer
    RC* reference; // Reference count

public:
    SharedGLobj() : pData(0), reference(nullptr)
    {
        // Create a new reference
        reference = new RC();
        // Increment the reference count
        reference->AddRef();
        //printf("ctor zero %u\n", reference->Count()); fflush(stdout);
    }

    SharedGLobj(GLuint pValue) : pData(pValue), reference(nullptr)
    {
        // Create a new reference
        reference = new RC();
        // Increment the reference count
        reference->AddRef();
        //printf("ctor %u %u\n", pValue, reference->Count()); fflush(stdout);
    }

    SharedGLobj(const SharedGLobj<DelFun>& sp) : pData(sp.pData), reference(sp.reference)
    {
        // Copy constructor
        // Copy the data and reference pointer
        // and increment the reference count
        reference->AddRef();
    }

    ~SharedGLobj()
    {
        // Destructor
        // Decrement the reference count
        // if reference become zero delete the data
        if (reference->Release() == 0)
        {
            if (pData)
            {
                DelFun(1, &pData);
                checkGL();
            }
            delete reference;
        }
        //printf("dtor %u\n", pData); fflush(stdout);
    }

    operator GLuint() const { return pData; }

    SharedGLobj<DelFun>& operator = (const SharedGLobj<DelFun>& sp)
    {
        // Assignment operator
        if (this != &sp) // Avoid self assignment
        {
            // Decrement the old reference count
            // if reference become zero delete the old data
            if (reference->Release() == 0)
            {
                if (pData)
                {
                    DelFun(1, &pData);
                    checkGL();
                }
                delete reference;
            }

            // Copy the data and reference pointer
            // and increment the reference count
            pData = sp.pData;
            reference = sp.reference;
            reference->AddRef();
        }
        return *this;
    }
};

using SharedGLtex = SharedGLobj<glDeleteTextures>;
using SharedGLbuf = SharedGLobj<glDeleteBuffers>;

#endif // GL_SHARED_H
