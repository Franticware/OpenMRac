#include "platform.h"

#include <cstdlib>
#include <ctime>
#include "rand1.h"


int rand1()
{
    return rand();
}

void srand1(unsigned int seed)
{
    srand(seed);
}

#if defined(__WIN32__) && defined(__MINGW32__)

#include <windows.h> // pro visual C++ je windows.h vloženo už v platform.h

#elif defined(__unix__) || defined(__linux__) || defined(__HAIKU__) || defined(__amigaos4__) || defined(__MORPHOS__)

#include <sys/times.h>

#endif

void srand1()
{
    // Visual C++
    #if defined(_MSC_VER)
    srand(GetTickCount());

    // mingw
    #elif defined(__WIN32__) && defined(__MINGW32__)
    srand(GetTickCount());

    // gcc v linuxu
    #elif defined(__unix__) || defined(__linux__) || defined(__HAIKU__) || defined(__amigaos4__)
    //srand(předělat na times);
    tms tm;
    srand(times(&tm));

    #elif defined(__MACOSX__)
    srand(time(0)^mach_absolute_time()); // FIXME only a concept that has to be finished to compile correctly

    #elif defined(__MORPHOS__)
    srand(time(0)); // simplest method, good enough when running single instance (which is usually the case here)

    #else
    #error "unknown platform"

    #endif
}

int randn1(int size)
{
	const int rozsah_casti = RAND_MAX / size;
	int r;

	do r = rand1() / rozsah_casti;
	while (r >= size);

	return r;
}

void swap_rand1(void* ptr1, void* ptr2, unsigned int size)
{
    if (ptr1 == ptr2)
        return;
    for (unsigned int i = 0; i != size; ++i)
    {
        ((unsigned char*)(ptr1))[i] ^= ((unsigned char*)(ptr2))[i];
        ((unsigned char*)(ptr2))[i] ^= ((unsigned char*)(ptr1))[i];
        ((unsigned char*)(ptr1))[i] ^= ((unsigned char*)(ptr2))[i];
    }
}

void randn1mix(void* ptr, unsigned int size, unsigned int count)
{
    unsigned char* ptr1 = (unsigned char*)ptr;
    for (int i = 0; i < int(count)-1; ++i)
    {
        int j = i + randn1(int(count)-i);
        swap_rand1(ptr1+i*size, ptr1+j*size, size);
    }
}
