#ifndef HLIDAC_TRIBOXINT_H
#define HLIDAC_TRIBOXINT_H

bool triboxint2(const float boxcen[3], const float a_2[3], const float v0[3], const float v1[3], const float v2[3]);

inline bool triboxint(const float boxcen[3], const float a_2[3], const float v0[3], const float v1[3], const float v2[3])
{
    return triboxint2(boxcen, a_2, v0, v1, v2);
}

#endif
