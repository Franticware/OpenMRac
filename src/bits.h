#ifndef HLIDAC_BITS_H
#define HLIDAC_BITS_H

inline unsigned int bits_count(unsigned int value)
{
    unsigned int ret = 0;
    unsigned int bit_selector = 1;
    while (bit_selector)
    {
        if (bit_selector & value)
            ++ret;
        bit_selector = bit_selector << 1;
    }
    return ret;
}

inline unsigned int bits_crop_npot(unsigned int value)
{
    if (value == 0) return 0;
    unsigned int ret = value;
    value >>= 1;
    for (int i = 0; i != sizeof(unsigned int) * 8; ++i)
    {
        ret &= ~value;
        value >>= 1;
    }
    return ret;
}

#endif
