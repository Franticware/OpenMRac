#include <cstdio>

#ifdef PNG_1_6_37_STATIC
#include "libpng-1.6.37/png.h"
#else
#include <png.h>
#endif

#include "pict2.h"

void user_error_fn(png_structp png_ptr, png_const_charp /*err_str*/)
{
#ifdef __MORPHOS__
	longjmp(png_ptr->jmpbuf59, 1);		
#else
    longjmp(png_jmpbuf(png_ptr), 1);
#endif
}

void user_warning_fn(png_structp, png_const_charp)
{
}

int Pict2::loadpng(const void* data, unsigned int size, unsigned int mode)
{
    return loadpng_pom(false, data, size, mode);
}

int Pict2::loadpng(const char* fname, unsigned int mode)
{
    return loadpng_pom(true, fname, 0, mode);
}

struct Pngbuff {
    unsigned char* data;
    unsigned int data_size;
    unsigned int pos;
};

void buff_png_read(png_struct* png, png_bytep buf, png_size_t size)
{
	Pngbuff* pngbuff = (Pngbuff*)png_get_io_ptr(png);
	if (pngbuff->pos+size > pngbuff->data_size) // pokud by čtení šlo mimo buffer, vyhoď chybu
        png_error(png, "user(r): unexpected end of stream");

	memcpy(buf, pngbuff->data + pngbuff->pos, size);
	pngbuff->pos+=size;
}

// 0: ERROR; 1: OK; > 1: WARNING, OTHER
int Pict2::loadpng_pom(bool bfile, const void* fname_data, unsigned int data_size, unsigned int mode) /*data_size použito, pokud bfile je false*/ // 8bitovou paletu nezkonvertuje na RGB (default = false)
{
    png_structp png_ptr;
    png_infop info_ptr;

    static FILE* fin = 0; // kvůli warningu kompileru
    Pngbuff pngbuff;
    static int ret = 1;

    std::vector<unsigned char*> rowsVector;
    unsigned char** rows = 0;

    std::vector<unsigned char> pxVector;
    unsigned char* px = 0;

    clear();

    if (bfile)
    {
        fin = fopen((const char*)fname_data, "rb");
        //fprintf(stderr, "%s %s\n", __PRETTY_FUNCTION__, fname_data);
        if (!fin)
            return 0;
    } else {
        pngbuff.data = (unsigned char*)fname_data;
        pngbuff.data_size = data_size;
        pngbuff.pos = 0;
    }

    /* Create and initialize the png_struct with the desired error handler
     * functions.  If you want to use the default stderr and longjump method,
     * you can supply NULL for the last three parameters.  We also supply the
     * the compiler header file version, so that we know if the application
     * was compiled with a compatible version of the library.  REQUIRED
     */
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, user_error_fn, user_warning_fn);
    if (png_ptr == NULL)
    {
        if (bfile)
            fclose(fin);
        return 0;
    }

    /* Allocate/initialize the memory for image information.  REQUIRED. */
    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL)
    {
        png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
        if (bfile)
            fclose(fin);
        return 0;
    }

    /* Error handling - REQUIRED by user error handler */
#ifdef __MORPHOS__
	if ( setjmp59(png_ptr->jmpbuf59))
#else
    if (setjmp(png_jmpbuf(png_ptr)))
#endif
    {
        clear();
        /* Free all of the memory associated with the png_ptr and info_ptr */
        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
        if (bfile)
            fclose(fin);
        //MessageBox(0, "error", "png error", MB_OK);
        /* If we get here, we had a problem reading the file */

        return 0;
    }

    /* One of the following I/O initialization methods is REQUIRED */
    //#ifdef streams /* PNG file I/O method 1 */
    /* Set up the input control if you are using standard C streams */
    if (bfile)
        png_init_io(png_ptr, fin);
    else
        png_set_read_fn(png_ptr, &pngbuff, buff_png_read);

    //#else no_streams /* PNG file I/O method 2 */
    /* If you are using replacement read functions, instead of calling
     * png_init_io() here you would call:
     */
    //png_set_read_fn(png_ptr, (void *)user_io_ptr, user_read_fn);
    /* where user_io_ptr is a structure you want available to the callbacks */
    //#endif no_streams /* Use only one I/O method! */

    /* If we have already read some of the signature */
    //png_set_sig_bytes(png_ptr, sig_read);

    /* The call to png_read_info() gives us all of the information from the
     * PNG file before the first IDAT (image data chunk).  REQUIRED
     */
    png_read_info(png_ptr, info_ptr);

    /*png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
        &interlace_type, NULL, NULL);*/

    /**** Set up the data transformations you want.  Note that these are all
     **** optional.  Only call them if you want/need them.  Many of the
     **** transformations only work on specific types of images, and many
     **** are mutually exclusive.
     ****/

    /* Extract multiple pixels with bit depths of 1, 2, and 4 from a single byte into separate bytes (useful for paletted and grayscale images). */
    if (png_get_bit_depth(png_ptr, info_ptr) < 8)
        png_set_packing(png_ptr);

    if (mode == PICT2_create_8b)
    {
        /* Expand paletted colors into true RGB triplets */
        if (png_get_color_type(png_ptr, info_ptr) == PNG_COLOR_TYPE_PALETTE)
            png_set_expand(png_ptr);
        /* Expand grayscale images to the full 8 bits from 1, 2, or 4 bits/pixel */
        if (png_get_color_type(png_ptr, info_ptr) == PNG_COLOR_TYPE_GRAY && png_get_bit_depth(png_ptr, info_ptr) < 8)
            png_set_expand(png_ptr);
        /* Expand paletted or RGB images with transparency to full alpha channels so the data will be available as RGBA quartets. */
        if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
            png_set_expand(png_ptr);

        png_read_update_info(png_ptr, info_ptr);
        if (png_get_bit_depth(png_ptr, info_ptr) == 8 && (png_get_channels(png_ptr, info_ptr) == 1 || png_get_channels(png_ptr, info_ptr) == 3 || png_get_channels(png_ptr, info_ptr) == 4))
        {
            int channels = png_get_rowbytes(png_ptr, info_ptr)/png_get_image_width(png_ptr, info_ptr);
            if (png_get_channels(png_ptr, info_ptr) != channels)
                png_error(png_ptr, "user(r): bit depth is not matching the row size");
            create(png_get_image_width(png_ptr, info_ptr), png_get_image_height(png_ptr, info_ptr), 1/*depth*/, 0);
            rowsVector.resize(p_h, 0);
            rows = &(rowsVector[0]);
            if (channels == 1)
            {
                for (int i = 0; i != p_h; ++i)
                    rows[i] = p_px.data()+(p_h-i-1)*p_w*p_d;
                png_read_image(png_ptr, rows);
            } else {
                pxVector.resize(p_w*p_h*channels);
                px = &(pxVector[0]);
                for (int i = 0; i != p_h; ++i)
                    rows[i] = px+(p_h-i-1)*p_w*channels;
                png_read_image(png_ptr, rows);
                for (int i = 0; i != p_w*p_h; ++i)
                    p_px[i] = px[i*channels];
                ret |= 2;
            }
        } else {
            png_error(png_ptr, "user(r): unsupported bit or pixel depth");
        }
    }
    else if (mode == PICT2_create_24b)
    {
        /* Expand paletted colors into true RGB triplets */
        if (png_get_color_type(png_ptr, info_ptr) == PNG_COLOR_TYPE_PALETTE)
            png_set_expand(png_ptr);
        /* Expand grayscale images to the full 8 bits from 1, 2, or 4 bits/pixel */
        if (png_get_color_type(png_ptr, info_ptr) == PNG_COLOR_TYPE_GRAY && png_get_bit_depth(png_ptr, info_ptr) < 8)
            png_set_expand(png_ptr);
        /* Expand paletted or RGB images with transparency to full alpha channels so the data will be available as RGBA quartets. */
        if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
            png_set_expand(png_ptr);

        png_set_strip_alpha(png_ptr);
        png_read_update_info(png_ptr, info_ptr);
        if (png_get_bit_depth(png_ptr, info_ptr) == 8 && (png_get_channels(png_ptr, info_ptr) == 1 || png_get_channels(png_ptr, info_ptr) == 3))
        {
            int channels =  png_get_rowbytes(png_ptr, info_ptr)/png_get_image_width(png_ptr, info_ptr);
            if (png_get_channels(png_ptr, info_ptr) != channels)
                png_error(png_ptr, "user(r): bit depth is not matching the row size");
            create(png_get_image_width(png_ptr, info_ptr), png_get_image_height(png_ptr, info_ptr), 3/*depth*/, 0);
            rowsVector.resize(p_h, 0);
            rows = &(rowsVector[0]);
            for (int i = 0; i != p_h; ++i)
                rows[i] = p_px.data()+(p_h-i-1)*p_w*channels;
            png_read_image(png_ptr, rows);
            if (png_get_channels(png_ptr, info_ptr) == 1)
            {
                // rozkopírovat bajty do 3 kanálů
                for (int i0 = p_w*p_h; i0 != 0; --i0)
                {
                    int i = i0 - 1;
                    p_px[i*3] = p_px[i*3+1] = p_px[i*3+2] = p_px[i];
                }
                ret |= 2;
            }

        } else {
            png_error(png_ptr, "user(r): unsupported bit or pixel depth");
        }
    }
    else if (mode == PICT2_create_32b)
    {
        bool bfiller = true;
        /* Expand paletted colors into true RGB triplets */
        if (png_get_color_type(png_ptr, info_ptr) == PNG_COLOR_TYPE_PALETTE)
            png_set_expand(png_ptr);
        // pokud je obrázek ve stupních šedi, tak bude 8-bitový a filler se nepřidá
        if (png_get_color_type(png_ptr, info_ptr) == PNG_COLOR_TYPE_GRAY)
            bfiller = false;
        /* Expand grayscale images to the full 8 bits from 1, 2, or 4 bits/pixel */
        if (png_get_color_type(png_ptr, info_ptr) == PNG_COLOR_TYPE_GRAY && png_get_bit_depth(png_ptr, info_ptr) < 8)
            png_set_expand(png_ptr);
        /* Expand paletted or RGB images with transparency to full alpha channels so the data will be available as RGBA quartets. */
        if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
            png_set_expand(png_ptr);
        if (bfiller)
            png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
        png_read_update_info(png_ptr, info_ptr);

        if (png_get_bit_depth(png_ptr, info_ptr) == 8 && (png_get_channels(png_ptr, info_ptr) == 1 || png_get_channels(png_ptr, info_ptr) == 4))
        {
            int channels = png_get_rowbytes(png_ptr, info_ptr)/png_get_image_width(png_ptr, info_ptr);
            if (png_get_channels(png_ptr, info_ptr) != channels)
                png_error(png_ptr, "user(r): bit depth is not matching the row size");
            create(png_get_image_width(png_ptr, info_ptr), png_get_image_height(png_ptr, info_ptr), 4/*depth*/, 0);
            rowsVector.resize(p_h, 0);
            rows = &(rowsVector[0]);
            for (int i = 0; i != p_h; ++i)
                rows[i] = p_px.data()+(p_h-i-1)*p_w*channels;
            png_read_image(png_ptr, rows);
            if (png_get_channels(png_ptr, info_ptr) == 1)
            {
                // rozkopírovat bajty do 3 kanálů, přidat alfa kanál 0xff
                for (int i0 = p_w*p_h; i0 != 0; --i0)
                {
                    int i = i0 - 1;
                    p_px[i*4] = p_px[i*4+1] = p_px[i*4+2] = p_px[i];
                    p_px[i*4+3] = 0xff;
                }
                ret |= 2;
            }
        } else {
            png_error(png_ptr, "user(r): unsupported bit or pixel depth");
        }
    }
    else {
        png_error(png_ptr, "user(r): unsupported mode");
    }

    /* read rest of file, and get additional chunks in info_ptr - REQUIRED */
    png_read_end(png_ptr, info_ptr);

    /* clean up after the read, and free any memory allocated - REQUIRED */
    png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);

    /* close the file */
    if (bfile)
        fclose(fin);

    if (empty())
    {
        clear();
        return 0;
    }

    /* that's it */
    return ret;
}
