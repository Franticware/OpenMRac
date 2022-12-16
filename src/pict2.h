#ifndef HLIDAC_PICT_H
#define HLIDAC_PICT_H

#include <cstring>
#include <vector>
#include <cstdint>
#include <algorithm>

#define PICT2_create_8b         1
#define PICT2_create_24b        3
#define PICT2_create_32b        4

class Pict2 {
public:
    Pict2() : p_w(0), p_h(0), p_d(0), p_px(0) {}
    explicit Pict2(int prm_w, int prm_h, int prm_d, const unsigned char* prm_px) {
        create(prm_w, prm_h, prm_d, prm_px);
    }
    typedef std::vector<unsigned char> Vec_b;
    int loadjpeg(const char*, unsigned int);
    int loadjpeg(const void*, unsigned int, unsigned int);
    int loadpng(const void*, unsigned int, unsigned int);
    int loadpng(const char*, unsigned int);
    void assign(int prm_w, int prm_h, int prm_d, const unsigned char* prm_px) {
        clear(); create(prm_w, prm_h, prm_d, prm_px); }
    void clear() { p_px.clear(); p_w = 0; p_h = 0; p_d = 0; }
    bool empty() const { return !(p_w && p_h && p_d && !p_px.empty()); }
    bool replace_alpha(const Pict2& pict);
    int w() const { return p_w; }
    int h() const { return p_h; }
    int d() const { return p_d; }
    uint8_t* px() { return p_px.data(); }
    const unsigned char* c_px() const { return p_px.data(); }
    unsigned char* px(int x, int y, int c = 0) { return p_px.data()+(x+y*p_w)*p_d+c; }
    const unsigned char* c_px(int x, int y, int c = 0) const { return p_px.data()+(x+y*p_w)*p_d+c; }
    unsigned char& operator[](size_t i) { return p_px[i]; }
    const unsigned char& operator[](size_t i) const { return p_px[i]; }
private:
    int p_w, p_h, p_d; // private width, height, depth
    std::vector<uint8_t> p_px; // pixels
    void create(int prm_w, int prm_h, int prm_d, const unsigned char* prm_px) {
        p_w = prm_w; p_h = prm_h; p_d = prm_d;
        p_px.resize(p_w*p_h*p_d);
        if (prm_px) std::copy_n(prm_px, p_px.size(), p_px.begin());
    }
    int loadpng_pom(bool bfile, const void* fname_data, unsigned int data_size, unsigned int mode);
    int loadjpeg_pom(bool bfile, const void* fname_data, unsigned int data_size, unsigned int mode);
};

#endif
