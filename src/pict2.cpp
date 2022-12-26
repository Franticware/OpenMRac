#include "pict2.h"

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
