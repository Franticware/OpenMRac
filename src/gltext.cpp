#include "gltext.h"

#include "load_texture.h"
#include "pict2.h"
#include "gbuff_in.h"
#include "matmng.h"
#include "glhelpers1.h"
#include <algorithm>

void Glfont::set_texture(GLuint texture)
{
    p_texture = texture;
}

void Gltext::set_pos(float x, float y)
{
    p_pos[0] = x;
    p_pos[1] = y;
}

void Gltext::render(GLuint texture)
{
    float scaletext = 0.319f;
    glLoadIdentity(); checkGL();

    glScalef(scaletext, scaletext, 1.0); checkGL();

    glTranslatef(p_pos[0], p_pos[1], 0.f); checkGL();

    glBindTexture(GL_TEXTURE_2D, texture); checkGL();

    glEnable(GL_BLEND); checkGL();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); checkGL();

    glEnableClientState(GL_VERTEX_ARRAY); checkGL();
    glEnableClientState(GL_TEXTURE_COORD_ARRAY); checkGL();

    for (unsigned int i = 0; i != p_h; ++i)
    {
        glVertexPointer(3, GL_FLOAT, 0, p_lines[i].vert); checkGL();
        glTexCoordPointer(2, GL_FLOAT, 0, p_lines[i].texc); checkGL();
        glDrawArrays(GL_QUADS, 0, p_lines[i].size); checkGL(); afterDrawcall();
    }

    glDisableClientState(GL_VERTEX_ARRAY); checkGL();
    glDisableClientState(GL_TEXTURE_COORD_ARRAY); checkGL();

    glDisable(GL_BLEND); checkGL();
}

void Gltext::render_c(GLuint texture)
{
    float scaletext = 0.319f;
    glLoadIdentity(); checkGL();

    glScalef(scaletext, scaletext, 1.0); checkGL();

    glTranslatef(p_pos[0], p_pos[1], 0.f); checkGL();

    glBindTexture(GL_TEXTURE_2D, texture); checkGL();

    glEnable(GL_BLEND); checkGL();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); checkGL();

    glEnableClientState(GL_VERTEX_ARRAY); checkGL();
    glEnableClientState(GL_TEXTURE_COORD_ARRAY); checkGL();

    for (unsigned int i = 0; i != p_h; ++i)
    {
        glColor4fv(p_lines[i].color_b); checkGL();
        glVertexPointer(3, GL_FLOAT, 0, p_lines[i].vert); checkGL();
        glTexCoordPointer(2, GL_FLOAT, 0, p_lines[i].texc); checkGL();
        glDrawArrays(GL_QUADS, 0, p_lines[i].size); checkGL(); afterDrawcall();
    }

    glDisableClientState(GL_VERTEX_ARRAY); checkGL();
    glDisableClientState(GL_TEXTURE_COORD_ARRAY); checkGL();

    glDisable(GL_BLEND); checkGL();
    glLoadIdentity(); checkGL();
}

void Gltext::renderscale(float scale, GLuint texture)
{
    float scaletext = 0.319f;
    glLoadIdentity(); checkGL();

    glScalef(scaletext, scaletext, 1.0); checkGL();

    glTranslatef(p_pos[0], p_pos[1], 0.f); checkGL();

    glScalef(scale, scale, 1.0); checkGL();

    glBindTexture(GL_TEXTURE_2D, texture); checkGL();

    glEnable(GL_BLEND); checkGL();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); checkGL();

    glEnableClientState(GL_VERTEX_ARRAY); checkGL();
    glEnableClientState(GL_TEXTURE_COORD_ARRAY); checkGL();

    for (unsigned int i = 0; i != p_h; ++i)
    {
        glVertexPointer(3, GL_FLOAT, 0, p_lines[i].vert); checkGL();
        glTexCoordPointer(2, GL_FLOAT, 0, p_lines[i].texc); checkGL();
        glDrawArrays(GL_QUADS, 0, p_lines[i].size); checkGL(); afterDrawcall();
    }

    glDisableClientState(GL_VERTEX_ARRAY); checkGL();
    glDisableClientState(GL_TEXTURE_COORD_ARRAY); checkGL();

    glDisable(GL_BLEND); checkGL();
}

unsigned int Glfont::get_char_i(char c) const
{
    unsigned char uc = c;
    unsigned int ret = (unsigned int)uc;
    if (ret < p_charmap_start)
        ret = (unsigned int)p_defaultchar;
    else if (ret >= p_charmap_start+p_charmap_size)
        ret = (unsigned int)p_defaultchar;
    return ret;
}

void Gltext::puts(unsigned int i/*číslo řádku*/, const char* text)
{
    for (;;)
    {
        if (i >= p_h)
            return;
        unsigned int utextsz = 0;
        while (text[utextsz] != '\n' && text[utextsz] != '\0')
            ++utextsz;
        // vypsat text
        // délka textu se omezí délkou alokovaného pole pro výpis textu
        unsigned int textsz1 = std::min(utextsz, p_w); // použitá délka řádku
        p_lines[i].size = textsz1*4; // počet vrcholů na řádku pro render
        float line_w = 0.f; // délka řádku je součet délek všech znaků a mezer mezi nimi, mezera je i za posledním znakem
        for (unsigned int i0 = 0; i0 != textsz1; ++i0)
            line_w += p_font->p_charmap[p_font->get_char_i(text[i0])].vertw + p_font->p_dist;
        line_w *= p_fontsize; // šířka se zvětší podle velikosti písma
        float line_left = 0.f; // levá relativní souřadnice textu (relativní k pozici p_pos)
        if (p_cen_x == 0) // střed
            line_left = -line_w*0.5f;
        else if (p_cen_x > 0) // zarovnání vlevo
            line_left = 0.f;
        else // p_cen_y < 0 // zarovnání vpravo
            line_left = -line_w;

        float char_left = 0.f;
        float char_right = 0.f;

        // nastavení obdélníka na pozadí
        p_lines[i].bkgrect[0] = p_lines[i].bkgrect[9] = line_left-p_font->p_dist;
        p_lines[i].bkgrect[3] = p_lines[i].bkgrect[6] = line_left+line_w;

        for (unsigned int j = 0; j != textsz1; ++j)
        {
            // x-ová souřadnice textu, ostatní byly nastaveny při inicializaci
            char_right = char_left + (p_font->p_charmap[p_font->get_char_i(text[j])].vertw)*p_fontsize;
            p_lines[i].vert[(j*4+0)*3+0] = line_left+char_left;
            p_lines[i].vert[(j*4+1)*3+0] = line_left+char_right;
            p_lines[i].vert[(j*4+2)*3+0] = line_left+char_right;
            p_lines[i].vert[(j*4+3)*3+0] = line_left+char_left;
            char_left = char_right + p_font->p_dist*p_fontsize;

            // texturové souřadnice
            p_lines[i].texc[(j*4+0)*2+0] = p_font->p_charmap[p_font->get_char_i(text[j])].texc[0];
            p_lines[i].texc[(j*4+1)*2+0] = p_font->p_charmap[p_font->get_char_i(text[j])].texc[1];
            p_lines[i].texc[(j*4+2)*2+0] = p_font->p_charmap[p_font->get_char_i(text[j])].texc[1];
            p_lines[i].texc[(j*4+3)*2+0] = p_font->p_charmap[p_font->get_char_i(text[j])].texc[0];

            p_lines[i].texc[(j*4+0)*2+1] = p_font->p_charmap[p_font->get_char_i(text[j])].texc[2];
            p_lines[i].texc[(j*4+1)*2+1] = p_font->p_charmap[p_font->get_char_i(text[j])].texc[2];
            p_lines[i].texc[(j*4+2)*2+1] = p_font->p_charmap[p_font->get_char_i(text[j])].texc[3];
            p_lines[i].texc[(j*4+3)*2+1] = p_font->p_charmap[p_font->get_char_i(text[j])].texc[3];
        }
        // přechod na další řádek
        if (text[utextsz] == '\n')
        {
            ++i;
            text = text+utextsz+1;
        }
        else
        {
            break;
        }
    }
}

void Gltext::init(unsigned int w, unsigned int h, float fontsize, int cen_x, int cen_y, const Glfont* font,
        const float color_b[4])
{
    p_h = h;
    p_w = w;
    p_lines = new Gltline[p_h];
    p_fontsize = fontsize;
    p_cen_x = cen_x;
    p_cen_y = cen_y;
    p_font = font;
    p_pos[0] = 0;
    p_pos[1] = 0;
    for (unsigned int i = 0; i != p_h; ++i)
    {
        memcpy(p_lines[i].color_b, color_b, sizeof(float)*4);
        p_lines[i].size = 0; // max p_w*4
        p_lines[i].vert = new float[p_w*4*3];
        p_lines[i].texc = new float[p_w*4*2];
    }
    float text_h = float(p_h)*p_fontsize;
    float text_top = 0.f;
    if (p_cen_y == 0)
        text_top = text_h*0.5f;
    else if (p_cen_y > 0)
        text_top = text_h;
    else // p_cen_y < 0
        text_top = 0.f;

    for (unsigned int i = 0; i != p_h; ++i)
    {
        p_lines[i].bkgrect[0] = p_lines[i].bkgrect[3] = p_lines[i].bkgrect[6] = p_lines[i].bkgrect[9] = 0.f;
        p_lines[i].bkgrect[1] = p_lines[i].bkgrect[4] = text_top-(float(i)+1.f)*p_fontsize;
        p_lines[i].bkgrect[2] = p_lines[i].bkgrect[5] = p_lines[i].bkgrect[8] = p_lines[i].bkgrect[11] = -10.f;
        p_lines[i].bkgrect[7] = p_lines[i].bkgrect[10] = text_top-(float(i))*p_fontsize;
        for (unsigned int j = 0; j != p_w; ++j)
        {
            // z-ové souřadnice textu jsou napevno -10
            p_lines[i].vert[(j*4+0)*3+2] = -10.f;
            p_lines[i].vert[(j*4+1)*3+2] = -10.f;
            p_lines[i].vert[(j*4+2)*3+2] = -10.f;
            p_lines[i].vert[(j*4+3)*3+2] = -10.f;

            // x-ové souřadnice se inicializují na 0
            p_lines[i].vert[(j*4+0)*3+0] = 0.f;
            p_lines[i].vert[(j*4+1)*3+0] = 0.f;
            p_lines[i].vert[(j*4+2)*3+0] = 0.f;
            p_lines[i].vert[(j*4+3)*3+0] = 0.f;

            // souřadnice se odečítají, protože pozice se počítá odspodu
            p_lines[i].vert[(j*4+0)*3+1] = text_top-(float(i)+1.f)*p_fontsize;
            p_lines[i].vert[(j*4+1)*3+1] = text_top-(float(i)+1.f)*p_fontsize;
            p_lines[i].vert[(j*4+2)*3+1] = text_top-(float(i))*p_fontsize;
            p_lines[i].vert[(j*4+3)*3+1] = text_top-(float(i))*p_fontsize;
        }
    }
}

void Glfont::init(const unsigned int mapsize[2]/*počet znaků*/, const unsigned int mapsize1[2]/*rozměry jednoho znaku*/, unsigned int mapcharh, const char* mapfname)
{
    p_charmap_start = 32;
    p_charmap_size = mapsize[0]*mapsize[1];
    p_defaultchar = 127;
    p_dist = 0.1-0.12;

    Pict2 pict;
    gbuff_in.f_open(mapfname, "rb");
    pict.loadpng(gbuff_in.fbuffptr(), gbuff_in.fbuffsz());
    gbuff_in.fclose();
    //

    for (unsigned int y0 = 0; y0 != mapsize[1]; ++y0)
    {
        for (unsigned int y1 = mapcharh; y1 != mapsize1[1]; ++y1)
        {
            for (unsigned int x = 0; x != (unsigned int)pict.w(); ++x)
            {
                *(pict.px(x, y0*mapsize1[1]+y1, 0)) = 0;
            }
        }
    }

    unsigned int i = p_charmap_start;
    for (unsigned int y = 0; y != mapsize[1]; ++y)
    {
        for (unsigned int x = 0; x != mapsize[0]; ++x)
        {

            p_charmap[i].texc[0] = float(x*mapsize1[0])/float(pict.w());
            unsigned int char_w = 0;
            while (char_w < mapsize1[0] && *(pict.px(char_w+x*mapsize1[0], y*mapsize1[1], 0)) < 64)
                ++char_w;
            for (unsigned int x1 = x*mapsize1[0]+char_w; x1 != (x+1)*mapsize1[0]; ++x1)
                for (unsigned int y1 = y*mapsize1[1]; y1 != y*mapsize1[1]+mapcharh; ++y1)
                    *(pict.px(x1, y1, 0)) = 0;
            p_charmap[i].texc[1] = float(x*mapsize1[0]+char_w)/float(pict.w());
            p_charmap[i].texc[2] = float(y*mapsize1[1])/float(pict.h());
            p_charmap[i].texc[3] = float(y*mapsize1[1]+mapcharh)/float(pict.h());
            p_charmap[i].vertw = float(char_w)/float(mapsize1[1]);
            ++i;
        }
    }
}

void Gltext::set_bkgrect(const float* color)
{
    p_b_bkgrect = color;
    if (p_b_bkgrect)
        memcpy(p_color_bkgrect, color, 4*sizeof(color));
}

void Gltext::set_color(unsigned int i/*číslo řádku*/, const float color[4])
{
    if (i < p_h)
        memcpy(p_lines[i].color_b, color, 4*sizeof(float));
}

void Gltext::set_color(const float color[4])
{
    for (unsigned int i = 0; i != p_h; ++i)
    {
        set_color(i/*číslo řádku*/, color);
    }
}
