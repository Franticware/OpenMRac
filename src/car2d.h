#ifndef HLIDAC_CAR2D_H
#define HLIDAC_CAR2D_H

#include "rbsolver.h"

class Car2D {
public:
    void init(float whf_x, float whf_y, float whb_x, float whb_y, RBSolver* rbsolver);

    float p_whf_x; // přední osa (+)
    float p_whf_y; // šířka předních kol (+)
    float p_whb_x; // zadní osa (-)
    float p_whb_y; // šířka zadních kol (+)
    float p_whl_a; // úhel natočení levého předního kola
    float p_whr_a; //               pravého
    float p_whl_a_i; // úhel - 0 .. 1
    float p_whr_a_i; //
    float p_whl_a_ren_i; // úhel kol pro zobrazení
    float p_whl_a_ren;

    bool p_b_acc;
    bool p_b_brake;

    float p_v_prev[2];
    float p_av_prev;

    float p_brake_i; // síla brzdy 0..1
    float p_brake_f; // velikost síly brzdy - brzdí všechna kola a u ruční jenom zadní (ruční tady nemáme)

    RBSolver* p_rbsolver;

    void steer_left();
    void steer_right();
    void steer_center();
    void steer_aux();

    void go_forward();  // dopředu / brzda při couvání
    void go_backward(); // dozadu  / brzda při jízdě vpřed
    void go_zero();     //
    void go_brake();    // brzda - obě klávesy

    float p_engine_pitch;

    float get_engine_pitch();
    void damp_engine_pitch(float f);

    float get_acc_aux();
    float get_rev_aux();

    void get_Fb_whl_acc(float F[2], float A[2]);
    void get_Fb_whr_acc(float F[2], float A[2]);
    void get_Fb_whl_rev(float F[2], float A[2]);
    void get_Fb_whr_rev(float F[2], float A[2]);
    float get_Fb_whfl_sl(float F[2], float A[2]);
    float get_Fb_whfr_sl(float F[2], float A[2]);
    float get_Fb_whbl_sl(float F[2], float A[2]);
    float get_Fb_whbr_sl(float F[2], float A[2]);

    float get_Fb_wh_sl_aux(float F[2], const float A[2], float sl_ratio1, float wh_a = 0.f);

    float add_sl_F();

    void brake_end();

};

// Orientace auta
//     |y
//     |
// +-b-|-f-+
// |   0------> x
// +-b---f-+

#endif
