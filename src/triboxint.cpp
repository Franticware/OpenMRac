#include "rbsolver.h"

float det3(const float c0[3], const float c1[3], const float c2[3])
{
    return c0[0]*c1[1]*c2[2]+c0[1]*c1[2]*c2[0]+c0[2]*c1[0]*c2[1]
        -c2[0]*c1[1]*c0[2]-c2[1]*c1[2]*c0[0]-c2[2]*c1[0]*c0[1];
}

void intersect3pl(float res[3], const float A[3], const float p[3], const float q[3], const float X[3], const float v[3])
{
    float det_b = det3(p, q, v); // tady je minus, i tam, takze se to zkrati
    float XmA[3] = {X[0]-A[0], X[1]-A[1], X[2]-A[2]};
    res[0] = det3(XmA, q, v)/det_b;
    res[1] = det3(p, XmA, v)/det_b;
    res[2] = -det3(p, q, XmA)/det_b;
}

bool isintquad(const float fv[3])
{
    return fv[0] >= 0.f && fv[0] <= 1.f && fv[1] >= 0.f && fv[1] <= 1.f && fv[2] >= 0.f && fv[2] <= 1.f;
}

bool isinttri(const float fv[3])
{
    return isintquad(fv) && fv[0]+fv[1] <= 1.f;
}

bool triboxint2(const float boxcen[3], const float a_2[3], const float v0[3], const float v1[3], const float v2[3])
{
    float mincoord[3];
    float maxcoord[3];
    for (int i = 0; i != 3; ++i)
    {
        mincoord[i] = boxcen[i]-a_2[i];
        maxcoord[i] = boxcen[i]+a_2[i];

        // optimalizace - hned na začátku se vyhážou trojúhelníky úplně mimo
        if (v0[i] > maxcoord[i] && v1[i] > maxcoord[i] && v2[i] > maxcoord[i])
            return false;
        if (v0[i] < mincoord[i] && v1[i] < mincoord[i] && v2[i] < mincoord[i])
            return false;
    }
    // testují se vrcholy trojúhelníků na přítomnost v krychli
    if ((v0[0] <= maxcoord[0] && v0[0] >= mincoord[0]
        && v0[1] <= maxcoord[1] && v0[1] >= mincoord[1]
        && v0[2] <= maxcoord[2] && v0[2] >= mincoord[2]) ||
        (v1[0] <= maxcoord[0] && v1[0] >= mincoord[0]
        && v1[1] <= maxcoord[1] && v1[1] >= mincoord[1]
        && v1[2] <= maxcoord[2] && v1[2] >= mincoord[2]) ||
        (v2[0] <= maxcoord[0] && v2[0] >= mincoord[0]
        && v2[1] <= maxcoord[1] && v2[1] >= mincoord[1]
        && v2[2] <= maxcoord[2] && v2[2] >= mincoord[2]))
        return true;

    // test průniku hran trojúhelníku se stěnami krychle

    float quadv0[3] = {maxcoord[0]-mincoord[0], 0, 0}; // vektory pro použití ve čtvercích
    float quadv1[3] = {0, maxcoord[1]-mincoord[1], 0};
    float quadv2[3] = {0, 0, maxcoord[2]-mincoord[2]};
    float quadp0[3] = {mincoord[0], mincoord[1], mincoord[2]}; // rohy čtverců
    float quadp1[3] = {maxcoord[0], mincoord[1], mincoord[2]};
    float quadp2[3] = {mincoord[0], maxcoord[1], mincoord[2]};
    float quadp3[3] = {mincoord[0], mincoord[1], maxcoord[2]};

    float* quads_p [6] = {quadp0, quadp0, quadp0, quadp1, quadp2, quadp3}; // definice všech čtverců
    float* quads_v0[6] = {quadv1, quadv0, quadv0, quadv1, quadv0, quadv0};
    float* quads_v1[6] = {quadv2, quadv2, quadv1, quadv2, quadv2, quadv1};

    float triv0[3] = {v1[0]-v0[0], v1[1]-v0[1], v1[2]-v0[2]}; // vektory linek trojůhelníku
    float triv1[3] = {v2[0]-v1[0], v2[1]-v1[1], v2[2]-v1[2]};
    float triv2[3] = {v0[0]-v2[0], v0[1]-v2[1], v0[2]-v2[2]};

    const float* trilines_p[3] = {v0, v1, v2};
    float* trilines_v[3] = {triv0, triv1, triv2};

    for (int i = 0; i != 6; ++i)
        for (int j = 0; j != 3; ++j)
        {
            float int_res[3];
            intersect3pl(int_res, quads_p[i], quads_v0[i], quads_v1[i], trilines_p[j], trilines_v[j]);
            if (isintquad(int_res))
                return true;
        }

    return false;
}
