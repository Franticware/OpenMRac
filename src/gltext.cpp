#include "load_texture.h"
#include "gltext.h"
#include "pict2.h"
#include "gbuff_in.h"
#include "matmng.h"
#include <algorithm>
#include "shadermng.h"

void Glfont::set_texture(SharedGLtex texture)
{
    p_texture = texture;
}

void Gltext::set_pos(float x, float y)
{
    static const float scaletext = 0.319f;
    mtrx = glm::scale(glm::mat4(1), glm::vec3(scaletext, scaletext, 1.f));
    mtrx = glm::translate(mtrx, glm::vec3(x, y, 0.f));
}

void Gltext::render(GLuint texture, ShaderMng* shadermng, bool useColor, float scale)
{
    glm::mat4 m = glm::scale(mtrx, glm::vec3(scale, scale, 1.f));
    shadermng->set(ShaderUniMat4::ModelViewMat, m);
    shadermng->use(ShaderId::ColorTex);
    shadermng->set(ShaderUniInt::AlphaDiscard, (GLint)0);
    glBindTexture(GL_TEXTURE_2D, texture); checkGL();
    glEnable(GL_BLEND); checkGL();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); checkGL();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buf);
    glEnableVertexAttribArray((GLuint)ShaderAttrib::Pos); checkGL();
    glEnableVertexAttribArray((GLuint)ShaderAttrib::Tex); checkGL();
    for (unsigned int i = 0; i != p_h; ++i)
    {
        glBindBuffer(GL_ARRAY_BUFFER, p_lines[i].buf);
        if (!p_lines[i].bufUpdated)
        {
            glBufferSubData(GL_ARRAY_BUFFER, 0, p_lines[i].vert.size() * sizeof(float), p_lines[i].vert.data());
            p_lines[i].bufUpdated = true;
        }
        if (useColor)
        {
            glVertexAttrib4fv((GLuint)ShaderAttrib::Color, p_lines[i].color_b);
        }
        glVertexAttribPointer((GLuint)ShaderAttrib::Pos, 3, GL_FLOAT, GL_FALSE, sizeof(float)*5, 0);
        glVertexAttribPointer((GLuint)ShaderAttrib::Tex, 2, GL_FLOAT, GL_FALSE, sizeof(float)*5, (void*)(sizeof(float)*3));
        glDrawElements(GL_TRIANGLES, p_lines[i].isize, GL_UNSIGNED_SHORT, 0); checkGL();
    }
    glDisableVertexAttribArray((GLuint)ShaderAttrib::Pos); checkGL();
    glDisableVertexAttribArray((GLuint)ShaderAttrib::Tex); checkGL();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDisable(GL_BLEND); checkGL();
}

void Gltext::render_c(GLuint texture, ShaderMng* shadermng)
{
    render(texture, shadermng, true);
}

void Gltext::renderscale(float scale, GLuint texture, ShaderMng* shadermng)
{
    render(texture, shadermng, false, scale);
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
        p_lines[i].isize = textsz1*6; // počet vrcholů na řádku pro render
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
            p_lines[i].vert[(j*4+0)*5+0] = line_left+char_left;
            p_lines[i].vert[(j*4+1)*5+0] = line_left+char_right;
            p_lines[i].vert[(j*4+2)*5+0] = line_left+char_right;
            p_lines[i].vert[(j*4+3)*5+0] = line_left+char_left;
            char_left = char_right + p_font->p_dist*p_fontsize;

            // texturové souřadnice
            p_lines[i].vert[(j*4+0)*5+3] = p_font->p_charmap[p_font->get_char_i(text[j])].texc[0];
            p_lines[i].vert[(j*4+1)*5+3] = p_font->p_charmap[p_font->get_char_i(text[j])].texc[1];
            p_lines[i].vert[(j*4+2)*5+3] = p_font->p_charmap[p_font->get_char_i(text[j])].texc[1];
            p_lines[i].vert[(j*4+3)*5+3] = p_font->p_charmap[p_font->get_char_i(text[j])].texc[0];

            p_lines[i].vert[(j*4+0)*5+4] = p_font->p_charmap[p_font->get_char_i(text[j])].texc[2];
            p_lines[i].vert[(j*4+1)*5+4] = p_font->p_charmap[p_font->get_char_i(text[j])].texc[2];
            p_lines[i].vert[(j*4+2)*5+4] = p_font->p_charmap[p_font->get_char_i(text[j])].texc[3];
            p_lines[i].vert[(j*4+3)*5+4] = p_font->p_charmap[p_font->get_char_i(text[j])].texc[3];

            p_lines[i].bufUpdated = false;
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
    p_lines.resize(p_h);
    p_fontsize = fontsize;
    p_cen_x = cen_x;
    p_cen_y = cen_y;
    p_font = font;
    for (unsigned int i = 0; i != p_h; ++i)
    {
        memcpy(p_lines[i].color_b, color_b, sizeof(float)*4);
        p_lines[i].size = 0; // max p_w*4
        p_lines[i].vert.clear();
        p_lines[i].vert.resize(p_w*4*5);

        GLuint tmpBuf;
        glGenBuffers(1, &tmpBuf);
        p_lines[i].buf = tmpBuf;
        glBindBuffer(GL_ARRAY_BUFFER, p_lines[i].buf);
        glBufferData(GL_ARRAY_BUFFER, p_lines[i].vert.size() * sizeof(float), 0, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    indices.clear();
    indices.resize(p_w*6);
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
            p_lines[i].vert[(j*4+0)*5+2] = -10.f;
            p_lines[i].vert[(j*4+1)*5+2] = -10.f;
            p_lines[i].vert[(j*4+2)*5+2] = -10.f;
            p_lines[i].vert[(j*4+3)*5+2] = -10.f;

            // x-ové souřadnice se inicializují na 0
            p_lines[i].vert[(j*4+0)*5+0] = 0.f;
            p_lines[i].vert[(j*4+1)*5+0] = 0.f;
            p_lines[i].vert[(j*4+2)*5+0] = 0.f;
            p_lines[i].vert[(j*4+3)*5+0] = 0.f;

            // souřadnice se odečítají, protože pozice se počítá odspodu
            p_lines[i].vert[(j*4+0)*5+1] = text_top-(float(i)+1.f)*p_fontsize;
            p_lines[i].vert[(j*4+1)*5+1] = text_top-(float(i)+1.f)*p_fontsize;
            p_lines[i].vert[(j*4+2)*5+1] = text_top-(float(i))*p_fontsize;
            p_lines[i].vert[(j*4+3)*5+1] = text_top-(float(i))*p_fontsize;
        }
        p_lines[i].bufUpdated = false;
    }

    for (unsigned int j = 0; j != p_w; ++j)
    {
        indices[j * 6 + 0] = j * 4 + 0;
        indices[j * 6 + 1] = j * 4 + 1;
        indices[j * 6 + 2] = j * 4 + 2;
        indices[j * 6 + 3] = j * 4 + 0;
        indices[j * 6 + 4] = j * 4 + 2;
        indices[j * 6 + 5] = j * 4 + 3;
    }

    GLuint tmpBuf;
    glGenBuffers(1, &tmpBuf);
    buf = tmpBuf;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buf);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLushort), indices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
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
