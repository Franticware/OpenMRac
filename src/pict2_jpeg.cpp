#include <cstdio>
#include <setjmp.h>

extern "C"
{
#ifdef JPEG_9D_STATIC
#include "jpeg-9d/jpeglib.h"
#include "jpeg-9d/jerror.h"
#else
#include <jpeglib.h>
#include <jerror.h>
#endif
}

#include "pict2.h"

struct my_error_mgr {
    jpeg_error_mgr pub;
    int* ret;
    jmp_buf setjmp_buffer;
};

METHODDEF(void) my_error_exit(j_common_ptr cinfo)
{
    longjmp(((my_error_mgr*)(((jpeg_compress_struct*)cinfo)->err))->setjmp_buffer, 1);
}

METHODDEF(void) my_emit_message(j_common_ptr cinfo, int)
{
    *(((my_error_mgr*)(((jpeg_compress_struct*)cinfo)->err))->ret) |= 4;
}

METHODDEF(void) my_output_message(j_common_ptr)
{
}

//

int Pict2::loadjpeg(const char* fname)
{
    return loadjpeg_pom(true, fname, 0);
}

int Pict2::loadjpeg(const void* data, unsigned int size)
{
    return loadjpeg_pom(false, data, size);
}

METHODDEF(void) my_init_source(j_decompress_ptr /*cinfo*/)
{
}

METHODDEF(boolean) my_fill_input_buffer(j_decompress_ptr cinfo)
{
    ERREXIT(cinfo, 1);
    return TRUE;
}

METHODDEF(void) my_skip_input_data(j_decompress_ptr cinfo, long num_bytes)
{
    if (num_bytes <= 0)
        return;
    if ((unsigned int)num_bytes > cinfo->src->bytes_in_buffer)
        ERREXIT(cinfo, 1);
    cinfo->src->bytes_in_buffer -= num_bytes;
    cinfo->src->next_input_byte += num_bytes;
}

METHODDEF(void) my_term_source(j_decompress_ptr /*cinfo*/)
{
}

int Pict2::loadjpeg_pom(bool bfile, const void* fname_data, unsigned int data_size)
{
    /* This struct contains the JPEG decompression parameters and pointers to
     * working space (which is allocated as needed by the JPEG library).
     */
    struct jpeg_decompress_struct cinfo;
    /* We use our private extension JPEG error handler.
     * Note that this struct must live as long as the main JPEG parameter
     * struct, to avoid dangling-pointer problems.
     */
    //struct my_error_mgr jerr;
    /*jpeg_error_mgr*/my_error_mgr jerr;
    /* More stuff */
    static FILE* fin = 0;		/* source file */
    JSAMPARRAY buffer;		/* Output row buffer */
    int row_stride;		/* physical row width in output buffer */

    jpeg_source_mgr source_mgr;

    //bool breturnval = true;
    int ret = 1;

    clear();

    /* In this example we want to open the input file before doing anything else,
     * so that the setjmp() error recovery below can assume the file is open.
     * VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
     * requires it in order to read binary files.
     */

    if (bfile)
    {
        fin = fopen((const char*)fname_data, "rb");
        //fprintf(stderr, "%s %s\n", __PRETTY_FUNCTION__, fname_data);

        if (!fin)
            return 0;
    }

    /* Step 1: allocate and initialize JPEG decompression object */

    /* We set up the normal JPEG error routines, then override error_exit. */
    cinfo.err = jpeg_std_error((jpeg_error_mgr*)&jerr);
    jerr.pub.error_exit = my_error_exit;
    jerr.pub.emit_message = my_emit_message;
    jerr.pub.output_message = my_output_message;
    jerr.ret = &ret;

    if (setjmp(jerr.setjmp_buffer))
    {
        clear();
        jpeg_destroy_decompress(&cinfo);
        /*if (buncreate)
            uncreate();*/
        if (bfile)
            fclose(fin);
        return 0;
    }

    /* Now we can initialize the JPEG decompression object. */
    jpeg_create_decompress(&cinfo);

    /* Step 2: specify data source (eg, a file) */

    if (bfile)
        jpeg_stdio_src(&cinfo, fin);
    else {
        source_mgr.next_input_byte = (const unsigned char*)fname_data;
        source_mgr.bytes_in_buffer = data_size;

        source_mgr.init_source = my_init_source;
        source_mgr.fill_input_buffer = my_fill_input_buffer;
        source_mgr.skip_input_data = my_skip_input_data;
        source_mgr.resync_to_restart = jpeg_resync_to_restart;
        source_mgr.term_source = my_term_source;
        cinfo.src = &source_mgr;
    }

    /* Step 3: read file parameters with jpeg_read_header() */

    jpeg_read_header(&cinfo, TRUE);
    /* We can ignore the return value from jpeg_read_header since
     *   (a) suspension is not possible with the stdio data source, and
     *   (b) we passed TRUE to reject a tables-only JPEG file as an error.
     * See libjpeg.doc for more info.
     */

    /* Step 4: set parameters for decompression */

    /* In this example, we don't need to change any of the defaults set by
     * jpeg_read_header(), so we do nothing here.
     */

    /* Step 5: Start decompressor */

    jpeg_start_decompress(&cinfo);
    /* We can ignore the return value since suspension is not possible
     * with the stdio data source.
     */

    {
        cinfo.out_color_space = JCS_EXT_BGRX;
        create(cinfo.output_width, cinfo.output_height, 0);
        row_stride = cinfo.output_width * 4;
        buffer = (*cinfo.mem->alloc_sarray)
            ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);
        while (cinfo.output_scanline < cinfo.output_height) {
            jpeg_read_scanlines(&cinfo, buffer, 1);
            memcpy(
                p_px.data()+(cinfo.output_height-cinfo.output_scanline)*row_stride,
                *buffer, row_stride);
        }
    }

    /* Step 7: Finish decompression */
    jpeg_finish_decompress(&cinfo);
    /* We can ignore the return value since suspension is not possible
     * with the stdio data source.
     */

    /* Step 8: Release JPEG decompression object */

    /* This is an important step since it will release a good deal of memory. */
    jpeg_destroy_decompress(&cinfo);

    /* After finish_decompress, we can close the input file.
     * Here we postpone it until after no more JPEG errors are possible,
     * so as to simplify the setjmp error logic above.  (Actually, I don't
     * think that jpeg_destroy can do an error exit, but why assume anything...)
     */
    if (bfile)
        fclose(fin);

    if (empty())
    {
        clear();
        return 0;
    }

    /* At this point you may want to check to see whether any corrupt-data
     * warnings occurred (test whether jerr.pub.num_warnings is nonzero).
     */

    if (jerr.pub.num_warnings)
        ret |= 4;

    /* And we're done! */
    return ret;
}
