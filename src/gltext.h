#ifndef HLIDAC_GLTEXT_H
#define HLIDAC_GLTEXT_H

#include <cstring>
#include <GL/gl.h>

struct Glfchar {
    float vertw;
    float texc[4]; // l, r, b, t
};

class Glfont {
public:
    Glfont() : p_texture(0), p_dist(0), p_charmap_start(0), p_charmap_size(0), p_defaultchar(0) { }
    ~Glfont() { }
    void init(const unsigned int mapsize[2]/*počet znaků*/, const unsigned int mapsize1[2]/*rozměry jednoho znaku*/, unsigned int mapcharh, const char* mapfname);
    void set_texture(GLuint texture);
    unsigned int get_char_i(char c) const;
    GLuint p_texture;
    float p_dist; // vzdálenost mezi znaky
    Glfchar p_charmap[256];
    unsigned int p_charmap_start;
    unsigned int p_charmap_size;
    unsigned char p_defaultchar;
};

struct Gltline {
    Gltline() : vert(0), texc(0), size(0), w(0) { }
    ~Gltline() { delete[] vert; delete[] texc; }
    float* vert;
    float* texc;
    unsigned int size; // počet vrcholů
    float w; // šířka řádku
    float color_b[4]; // barva spodní části písmen
    float color_t[4]; // barva vrchní části písmen (nepoužito)

    float bkgrect[12]; // obdélník na pozadí textu

};

class Gltext {
public:
    Gltext() : p_font(0), p_lines(0), p_h(0), p_w(0), p_cen_x(0), p_cen_y(0), p_fontsize(0), p_b_bkgrect(false) { }
    ~Gltext() { delete[] p_lines; }
    void init(unsigned int w, unsigned int h, float fontsize, int cen_x, int cen_y, const Glfont* font,
        const float color_b[4]); // počet písmen na řádku, počet písmen na výšku, výška znaků, zarovnání znaků, , odkaz na font, barvy
    void set_pos(float x, float y);
    void set_color(const float color[4]);
    void set_color(unsigned int i/*číslo řádku*/, const float color[4]);
    void set_bkgrect(const float* color = NULL);
    void puts(unsigned int i/*číslo řádku*/, const char* text);
    void render_();
    void render(GLuint texture);
    void render_c(GLuint texture);
    void render_c() { render_c(p_font->p_texture); }
    void renderscale_(float scale);
    void renderscale(float scale, GLuint texture);

    const Glfont* p_font; // odkaz na font
    Gltline* p_lines; // řádky
    unsigned int p_h; // počet řádků
    unsigned int p_w;
    int p_cen_x;
    int p_cen_y;
    float p_fontsize; // výška znaků
    float p_pos[2];

    float p_color_bkgrect[4];
    bool p_b_bkgrect;

};

#endif
