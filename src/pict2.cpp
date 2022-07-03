#include "pict2.h"

bool Pict2::replace_alpha(const Pict2& pict)
{
    if (p_d != 4 || pict.p_d < 1 || p_w != pict.p_w || p_h != pict.p_h)
        return false;
    for (int i = 0; i != p_w*p_h; ++i)
        p_px[i*4+3] = pict.p_px[i*pict.p_d];
    return true;
}
