#include <cstdio>

#ifdef PNG_1_6_37_STATIC
#include "libpng-1.6.37/png.h"
#else
#include <png.h>
#endif

#include "pict2.h"

int Pict2::loadpng(const void* data, unsigned int size)
{
    return loadpng_pom(false, data, size);
}

int Pict2::loadpng(const char* fname)
{
    return loadpng_pom(true, fname, 0);
}

// 0: ERROR; 1: OK; > 1: WARNING, OTHER
int Pict2::loadpng_pom(bool bfile, const void* fname_data, unsigned int data_size) /*data_size použito, pokud bfile je false*/ // 8bitovou paletu nezkonvertuje na RGB (default = false)
{
    png_image image;
    memset(&image, 0, sizeof(image));
    image.version = PNG_IMAGE_VERSION;
    if (bfile)
    {
        if (!png_image_begin_read_from_file(&image, (const char*)fname_data)) {
            return 0;
        }
    }
    else
    {
        if (!png_image_begin_read_from_memory(&image, fname_data, data_size)) {
            return 0;
        }
    }
    image.format = PNG_FORMAT_RGBA;
    create(image.width, image.height, 0);
    // a negative stride indicates that the bottom-most row is first in the buffer
    // (as expected by openGL)
    if (!png_image_finish_read(&image, NULL, p_px.data(), -image.width*4, NULL)) {
        png_image_free(&image);
        return 0;
    }
    return 1;
}
