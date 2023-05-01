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

void srand1()
{
    time_t t = time(nullptr);
    srand(t);
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
