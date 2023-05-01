#include "fopendir.h"

FILE* fopenDir(const char* filename, const char* mode, const char*, const char*)
{
    return fopen(filename, mode);
}
