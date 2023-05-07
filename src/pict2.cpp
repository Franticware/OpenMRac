#include "pict2.h"
#include "bits.h"

int Pict2::loaderr()
{
    p_w = 8;
    p_h = 8;
    p_px.resize(4 * 8 * 8);
    int k = 0;
    for (int i = 0; i != 8 * 8; ++i)
    {
        for (int j = 0; j != 4; ++j)
        {
            p_px[k] = j == 0 || j == 3 ? 0xff : 0x00;
            ++k;
        }
    }
    p_px[3] = 0;
    return 1;
}

bool Pict2::r2a()
{
    p_hasAlphaInit = true;
    p_hasAlpha = true;
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
    p_hasAlphaInit = true;
    p_hasAlpha = true;
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
        }
    }
    p_w = newW;
    p_h = newH;
}

void Pict2::pack16(int alignment)
{
    if (!p_hasAlphaInit)
    {
        p_hasAlphaInit = true;
        p_hasAlpha = false;
        for (int i = 0; i != p_w * p_h; ++i)
        {
            uint8_t a = p_px[i * 4 + 3];
            if (a != 0xff)
            {
                p_hasAlpha = true;
                break;
            }
        }
    }

    int i = 0; // input index
    int j = 0;
    int alignedWidth = ((((p_w * 2) / alignment) + !!((p_w * 2) % alignment)) * alignment) / 2;

    p_ppx.resize(alignedWidth * p_h);

    for (int y = 0; y != p_h; ++y)
    {
        j = alignedWidth * y;
        for (int x = 0; x != p_w; ++x, ++i, ++j)
        {
            if (p_hasAlpha)
            {
                uint8_t r = (p_px[i * 4 + 0]);
                uint8_t g = (p_px[i * 4 + 1]);
                uint8_t b = (p_px[i * 4 + 2]);
                uint8_t a = p_px[i * 4 + 3];
                p_ppx[j] = (uint16_t(r & 0xf0) << 8) | (uint16_t(g & 0xf0) << 4) | (uint16_t(b & 0xf0)) | (uint16_t(a & 0xf0) >> 4);
            }
            else
            {
                uint16_t r = p_px[i * 4 + 0];
                uint16_t g = p_px[i * 4 + 1];
                uint16_t b = p_px[i * 4 + 2];
                p_ppx[j] = ((r & 0b11111000) << 8) | ((g & 0b11111100) << 3) | (b >> 3);
            }
        }
    }
}

void Pict2::cropNpotH()
{
    p_h = bits_crop_npot(p_h);
}
