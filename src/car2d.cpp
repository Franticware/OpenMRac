#include "car2d.h"

#include <algorithm>

using namespace RBf;

const float g_front_slip_ratio = 1.3f;//1.2;
const float g_slip_rot_ratio = 1.005;
const float g_back_slip_ratio = g_front_slip_ratio*g_slip_rot_ratio;

void Car2D::init(float whf_x, float whf_y, float whb_x, float whb_y, RBSolver* rbsolver)
{
    p_whf_x = whf_x;
    p_whf_y = whf_y;
    p_whb_x = whb_x;
    p_whb_y = whb_y;
    p_whl_a = 0.f;
    p_whr_a = 0.f;
    p_whl_a_i = 0.f;
    p_whr_a_i = 0.f;
    p_whl_a_ren = 0.f;
    p_whl_a_ren_i = 0.f;
    p_rbsolver = rbsolver;
    p_engine_pitch = 1.f;
    p_b_acc = false;
    p_b_brake = false;
    p_brake_i = 0.f;
    zerov(p_v_prev);
    p_av_prev = 0.f;
}

void Car2D::damp_engine_pitch(float f)
{
    p_engine_pitch /= f;
    if (p_engine_pitch < 1.f)
        p_engine_pitch = 1.f;
}

void Car2D::go_forward()
{
    p_b_brake = false;
    if (p_rbsolver->v_a_dprod() >= 0.0f)
    {
        float Fpom[2];
        float Apom[2];
        get_Fb_whl_acc(Fpom, Apom);
        p_rbsolver->addFb(Fpom, Apom);
        get_Fb_whr_acc(Fpom, Apom);
        p_rbsolver->addFb(Fpom, Apom);
        p_b_acc = true;

        p_brake_i = 0.f;
    } else {
        go_brake();
        p_b_acc = false;
    }
}

void Car2D::go_backward()
{
    p_b_brake = false;
    if (p_rbsolver->v_a_dprod() <= 0.0f)
    {
        float Fpom[2];
        float Apom[2];
        get_Fb_whl_rev(Fpom, Apom);
        p_rbsolver->addFb(Fpom, Apom);
        get_Fb_whr_rev(Fpom, Apom);
        p_rbsolver->addFb(Fpom, Apom);
        p_b_acc = true;

        p_brake_i = 0.f;
    } else {
        go_brake();
        p_b_acc = false;
    }
}

void Car2D::go_zero()
{
    p_b_brake = false;
    // nic
    p_b_acc = false;
    p_brake_i = 0.f;
}

void Car2D::go_brake()
{
    cpy(p_v_prev, p_rbsolver->p_v);
    p_av_prev = p_rbsolver->p_av;
    p_b_brake = true;
    p_b_acc = false;
    p_brake_i += 2*p_rbsolver->p_timesync->p_T;
    if (p_brake_i > 1.f)
        p_brake_i = 1.f;

    float brake_F = p_rbsolver->p_m*5.f*(g_front_slip_ratio+g_back_slip_ratio);

    float v_pom[2];
    cpy(v_pom, p_rbsolver->p_v);
    norm(v_pom);
    lmul(v_pom, -brake_F);
    safe(v_pom);

    p_rbsolver->addF(v_pom);
}

void Car2D::brake_end()
{
    if (p_b_brake && dprod(p_v_prev, p_rbsolver->p_v) <= 0.f)
        zerov(p_rbsolver->p_v);
    if (p_b_brake && p_av_prev*p_rbsolver->p_av <= 0.f)
        p_rbsolver->p_av = 0.f;
}

float Car2D::add_sl_F()
{
    float Fpom[2];
    float Apom[2];

    float sl_gain_pom = get_Fb_whfl_sl(Fpom, Apom);
    p_rbsolver->addFb(Fpom, Apom);

    sl_gain_pom = std::max(sl_gain_pom, get_Fb_whfr_sl(Fpom, Apom));
    p_rbsolver->addFb(Fpom, Apom);

    sl_gain_pom = std::max(sl_gain_pom, get_Fb_whbl_sl(Fpom, Apom));
    p_rbsolver->addFb(Fpom, Apom);

    sl_gain_pom = std::max(sl_gain_pom, get_Fb_whbr_sl(Fpom, Apom));
    p_rbsolver->addFb(Fpom, Apom);

    return sl_gain_pom;
    // síla tření, použití pro zvuk - předělat
    float ret = sl_gain_pom/50000.f;
    if (ret < 0.3f)
        ret = (ret-0.2)*3;
    else if (ret > 1.f)
        ret = 1.f;
    if (ret < 0.f)
        ret = 0.f;
    return ret;
}

void Car2D::steer_left()
{
    if (p_whl_a_i < 0)
        p_whl_a_i = 0;
    p_whl_a_i += 0.8*p_rbsolver->p_timesync->p_T;
    if (p_whl_a_i > 1.f)
        p_whl_a_i = 1.f;
    p_whr_a_i = p_whl_a_i;
    if (p_whl_a_ren_i < p_whl_a_i)
    {
        p_whl_a_ren_i += 8*p_rbsolver->p_timesync->p_T;
        if (p_whl_a_ren_i > p_whl_a_i)
            p_whl_a_ren_i = p_whl_a_i;
    }
    steer_aux();
}

void Car2D::steer_right()
{
    if (p_whl_a_i > 0)
        p_whl_a_i = 0;
    p_whl_a_i -= 0.8*p_rbsolver->p_timesync->p_T;
    if (p_whl_a_i < -1.f)
        p_whl_a_i = -1.f;
    p_whr_a_i = p_whl_a_i;
    if (p_whl_a_ren_i > p_whl_a_i)
    {
        p_whl_a_ren_i -= 8*p_rbsolver->p_timesync->p_T;
        if (p_whl_a_ren_i < p_whl_a_i)
            p_whl_a_ren_i = p_whl_a_i;
    }
    steer_aux();
}

void Car2D::steer_center()
{
    p_whl_a_i = 0;
    p_whr_a_i = p_whl_a_i;
    if (p_whl_a_ren_i > 0.f)
    {
        p_whl_a_ren_i -= 8*p_rbsolver->p_timesync->p_T;
        if (p_whl_a_ren_i < 0.f)
            p_whl_a_ren_i = 0.f;
    }
    else if (p_whl_a_ren_i < 0.f)
    {
        p_whl_a_ren_i += 8*p_rbsolver->p_timesync->p_T;
        if (p_whl_a_ren_i > 0.f)
            p_whl_a_ren_i = 0.f;
    }
    steer_aux();
}

void Car2D::steer_aux()
{
    float whl_a_max = (-logf((len(p_rbsolver->p_v)*0.02+0.2)*0.8)+0.56)*0.3+0.25;
    if (whl_a_max < 0.25)
        whl_a_max = 0.25;
    whl_a_max -= 0.125;
    whl_a_max *= M_PI*0.25;
    p_whl_a = p_whl_a_i*whl_a_max;
    p_whr_a = p_whr_a_i*whl_a_max;
    p_whl_a_ren = whl_a_max*p_whl_a_ren_i;
}

float get_acc_aux0(float v)
{
    if (v > 40)
        return 0;
    if (v > 28)
        return 0.6*1.5;
    if (v > 18)
        return 1*1.5;
    if (v > 10)
        return 1.3*1.5;
    if (v > 1)
        return 5*1.5;
    return 10;
}

float Car2D::get_acc_aux()
{
    return get_acc_aux0(len(p_rbsolver->p_v))*1.2;
}

float Car2D::get_rev_aux()
{
    float v = len(p_rbsolver->p_v);
    if (v > 10)
        return 0;
    if (v > 1)
        return 3;
    return 10;
}

void Car2D::get_Fb_whl_acc(float F[2], float A[2]) // získání síly zadního kola při sešlápnutém plynu
{
    F[0] = p_rbsolver->p_m*get_acc_aux()*0.5;
    F[1] = 0;
    A[0] = p_whb_x;
    A[1] = p_whb_y;
}

void Car2D::get_Fb_whr_acc(float F[2], float A[2])
{
    F[0] = p_rbsolver->p_m*get_acc_aux()*0.5; // zrychlení je zhruba 10 m.s^-1
    F[1] = 0;
    A[0] = p_whb_x;
    A[1] = -p_whb_y;
}

void Car2D::get_Fb_whl_rev(float F[2], float A[2]) // získání síly zadního kola při sešlápnutém plynu
{
    F[0] = -p_rbsolver->p_m*get_rev_aux()*0.5;
    F[1] = 0;
    A[0] = p_whb_x;
    A[1] = p_whb_y;
}

void Car2D::get_Fb_whr_rev(float F[2], float A[2])
{
    F[0] = -p_rbsolver->p_m*get_rev_aux()*0.5;
    F[1] = 0;
    A[0] = p_whb_x;
    A[1] = -p_whb_y;
}

float Car2D::get_Fb_wh_sl_aux(float F[2], const float A[2], float sl_ratio1, float wh_a)
{
    float tm[2];
    p_rbsolver->get_tm(tm);
    float r[2];
    RBf::rot(r, tm, A);
    RBf::perp(r);
    RBf::lmul(r, p_rbsolver->p_av);
    RBf::ladd(r, p_rbsolver->p_v); // v' = v + av*r
    typedef float floatv[2];
    floatv& v = r;
    float va = atan2(v[1], v[0]); // úhel rychlosti
    float wa = p_rbsolver->p_ax+wh_a; // úhel kol
    float da = wa-va; // rozdíl úhlů
    float sa = sinf(da); // sin da

    float delka = sa*sqrtf(RBf::len(v))*p_rbsolver->p_m*p_rbsolver->p_am/100;

    F[0] = -sinf(wh_a)*delka;
    F[1] = cosf(wh_a)*delka;

    float delka_F = len(F);
    float sl_F = p_rbsolver->p_m/4*10*sl_ratio1;
    if (delka_F > sl_F)
    {
        ldiv(F, delka_F);
        lmul(F, sl_F);
    }

    return std::max(0.0f, delka_F-sl_F);
}

float Car2D::get_Fb_whfl_sl(float F[2], float A[2]) // získání síly zatáčení levého předního kola (v potaz se bere předchozí rychlost)
{
    A[0] = p_whf_x;
    A[1] = p_whf_y;
    return get_Fb_wh_sl_aux(F, A, g_front_slip_ratio, p_whl_a);
}

float Car2D::get_Fb_whfr_sl(float F[2], float A[2])
{
    A[0] = p_whf_x;
    A[1] = -p_whf_y;
    return get_Fb_wh_sl_aux(F, A, g_front_slip_ratio, p_whr_a);
}

float Car2D::get_Fb_whbl_sl(float F[2], float A[2]) // získání síly zatáčení levého předního kola (v potaz se bere předchozí rychlost)
{
    A[0] = p_whb_x;
    A[1] = p_whb_y;
    return get_Fb_wh_sl_aux(F, A, g_back_slip_ratio);
}

float Car2D::get_Fb_whbr_sl(float F[2], float A[2])
{
    A[0] = p_whb_x;
    A[1] = -p_whb_y;
    return get_Fb_wh_sl_aux(F, A, g_back_slip_ratio);
}

float Car2D::get_engine_pitch()
{
    if (!p_b_acc && !p_b_brake)
        return p_engine_pitch;

    float ret = 1.0f;
    float v = len(p_rbsolver->p_v);

    if (p_rbsolver->v_a_dprod() < 0.0f)
    {
        v /= 10.f;
        v *= 0.4f;
        v += 1.f;
        ret = v;
        if (ret > 1.4f)
            ret = 1.4f;
        p_engine_pitch = ret;
        return v;
    }

    if (v > 40)
    {
        ret = 1.4;
    }
    else if (v > 28)
    {
        v -= 28.f;
        v /= (40.f-28.f);
        v *= 0.4f;
        v += 1.f;
        ret = v;
    }
    else if (v > 18)
    {
        v -= 18.f;
        v /= (28.f-18.f);
        v *= 0.4f;
        v += 1.f;
        ret = v;
    }
    else if (v > 10)
    {
        v -= 10.f;
        v /= (18.f-10.f);
        v *= 0.4f;
        v += 1.f;
        ret = v;
    } else {
    v /= 10.f;
    v *= 0.4f;
    v += 1.f;
        ret = v;
    }
    if (ret > 1.4f)
        ret = 1.4f;
    p_engine_pitch = ret;
    return ret;
}
