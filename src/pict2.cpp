#include "pict2.h"
#include "bits.h"

bool Pict2::r2a()
{
    for (int i = 0; i != p_w * p_h; ++i)
    {
        p_px[i * 4 + 3] = p_px[i * 4 + 2];
        p_px[i * 4 + 2] = 0xff;
        p_px[i * 4 + 1] = 0xff;
        p_px[i * 4 + 0] = 0xff;
    }
    return true;
}

bool Pict2::r2a(const Pict2& pict)
{
    if (p_w != pict.p_w || p_h != pict.p_h)
        return false;
    for (int i = 0; i != p_w * p_h; ++i)
    {
        p_px[i * 4 + 3] = *(pict.c_px() + i * 4 + 2);
    }
    return true;
}

void Pict2::scale(int newW, int newH)
{
    // only to shrink down, POT textures
    if (newW > p_w) newW = p_w;
    if (newH > p_h) newH = p_h;
    const int divW = p_w/newW;
    const int divH = p_h/newH;
    const uint32_t divCount = divW * divH;
    for (int y = 0; y != newH; ++y)
    {
        for (int x = 0; x != newW; ++x)
        {
            for (int i = 0; i != 4; ++i)
            {
                uint32_t accum = 0;
                for (int b = 0; b != divH; ++b)
                {
                    for (int a = 0; a != divW; ++a)
                    {
                        accum += p_px[(x * divW + a + (y * divH + b) * p_w) * 4 + i];
                    }
                }
                accum /= divCount;
                p_px[(x + y * newW) * 4 + i] = accum;
            }
            /*for (int i = 3; i != 4; ++i)
            {
                uint8_t accum = 0;
                for (int b = 0; b != divH; ++b)
                {
                    for (int a = 0; a != divW; ++a)
                    {
                        uint8_t tmp = p_px[(x * divW + a + (y * divH + b) * p_w) * 4 + i];
                        if (tmp > accum) accum = tmp;
                    }
                }
                p_px[(x + y * newW) * 4 + i] = accum;
            }*/
        }
    }
    p_w = newW;
    p_h = newH;
}

void Pict2::pack16(void)
{
    bool use565 = true;
    for (int i = 0; i != p_w * p_h; ++i)
    {
        uint8_t a = p_px[i * 4 + 3];
        if (a != 0xff)
        {
            use565 = false;
            break;
        }
    }
    if (use565) // all 0xff alpha channel can be thrown away
    {
        pack565();
    }
    else
    {
        pack4444();
    }
}

inline uint8_t dither16(uint8_t b)
{
    return b; // no-op
}

void Pict2::pack4444(void)
{
    for (int i = 0; i != p_w * p_h; ++i)
    {
        uint8_t r = dither16(p_px[i * 4 + 0]);
        uint8_t g = dither16(p_px[i * 4 + 1]);
        uint8_t b = dither16(p_px[i * 4 + 2]);
        uint8_t a = p_px[i * 4 + 3];

        p_px[i * 2 + 1] = (r & 0xf0) | (g >> 4);
        p_px[i * 2 + 0] = (b & 0xf0) | (a >> 4);

    }
    packed565 = false;
}

void Pict2::pack565(void)
{
    for (int i = 0; i != p_w * p_h; ++i)
    {
        uint16_t r = p_px[i * 4 + 0];
        uint16_t g = p_px[i * 4 + 1];
        uint16_t b = p_px[i * 4 + 2];

        ((uint16_t*)p_px.data())[i] = ((r & 0b11111000) << 8) | ((g & 0b11111100) << 3) | (b >> 3);
    }
    packed565 = true;
}
