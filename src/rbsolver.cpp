#include "rbsolver.h"
#include <algorithm>

using namespace RBf;

void TimeSync::init(float T)
{
    p_T = T;
    p_prevT = 0.f;
}

void TimeSync::add_deltaT(float deltaT)
{
    p_prevT += deltaT;
}

bool TimeSync::step()
{
    if (p_prevT >= p_T)
    {
        p_prevT -= p_T;
        return true;
    }
    return false;
}

void RBSolver::get_vp(float* vp, float* pp, const float* pt) const
{
    sub(pp, pt, p_x);
    perp(pp);
    mul(vp, pp, p_av);
    ladd(vp, p_v);
}

void damp_v(float* p_v, float f/*součinitel smykového tření*/, float p_T)
{
    float a = f*10;
    float dv = a*p_T;
    if (len2(p_v) > dv*dv)
    {
        float v[2];
        norm(v, p_v);
        lmul(v, dv);
        safe(v);
        lsub(p_v, v);
    } else {
        zerov(p_v);
    }
}

void RBSolver::damp_v(float f/*součinitel smykového tření*/)
{
    float a = f*10;
    float dv = a*p_timesync->p_T;
    if (len2(p_v) > dv*dv)
    {
        float v[2];
        norm(v, p_v);
        lmul(v, dv);
        safe(v);
        lsub(p_v, v);
    } else {
        zerov(p_v);
    }
}

void RBSolver::damp_av(float f, float r)
{
    float Ft = f*p_m*10;
    float Mt = 0.66667*Ft*r;
    float aa = Mt/p_am;
    float d_av = aa*p_timesync->p_T;
    if (fabs(p_av) > d_av)
    {
        if (p_av >= 0.f)
            p_av -= d_av;
        else
            p_av += d_av;
    } else {
        p_av = 0.f;
    }
}

struct BBox_ln {
    unsigned int b_ln;
    unsigned int i0,i1;
};

// přidat podporu ibound - DONE
// zpřehlednit kód, m_o a m_o2 předělat z referencí na ukazatele!
void bbox_create(std::vector<BBox> &bbox, unsigned int& size, float* center, const T3dm* t3dm)
{
    int i_tst = t3dm->getgidobj(1);
    if (t3dm->p_o.size() == 0 || i_tst == -1)
    {
        bbox.clear();
        size = 0;
        return;
    }
    const O3dm& m_o = t3dm->p_o[i_tst];
    if (m_o.p_i.size() == 0)
    {
        bbox.clear();
        size = 0;
        return;
    }
    unsigned int ln_sz = m_o.p_i.size()/3*3;
    unsigned int ln_sz2 = 0;

    int i_tst2 = t3dm->getgidobj(3); // ibound
    bool bobj2 = true;
    if (t3dm->p_o.size() == 0 || i_tst2 == -1)
    {
        bobj2 = false;
    }
    const O3dm& m_o2 = t3dm->p_o[bobj2?i_tst2:i_tst]; // i_tst je tam proto, aby tam něco bylo
    if (m_o2.p_i.size() == 0)
    {
        bobj2 = false;
    }
    if (bobj2)
        ln_sz2 = m_o2.p_i.size()/3*3;
    {
        std::vector<BBox_ln> b_ln(ln_sz+ln_sz2);
        for (unsigned int i = 0; i != ln_sz; ++i)
        {
            b_ln[i].b_ln = 1;
            // indexy jednotlivých linek
            b_ln[i].i0 = m_o.p_i[i];
            if (i%3 == 2)
                b_ln[i].i1 = m_o.p_i[i-2];
            else
                b_ln[i].i1 = m_o.p_i[i+1];
        }
        for (unsigned int i = 0; i != ln_sz2; ++i)
        {
            b_ln[i+ln_sz].b_ln = 1;
            // indexy jednotlivých linek
            b_ln[i+ln_sz].i1 = m_o2.p_i[i];
            if (i%3 == 2)
                b_ln[i+ln_sz].i0 = m_o2.p_i[i-2];
            else
                b_ln[i+ln_sz].i0 = m_o2.p_i[i+1];
        }
        ln_sz += ln_sz2;

        for (unsigned int i = 0; i != ln_sz; ++i)
        {
            float f_i0[2] = {t3dm->p_v[(size_t)T3dmA::Count*b_ln[i].i0+2], t3dm->p_v[(size_t)T3dmA::Count*b_ln[i].i0]};
            float f_i1[2] = {t3dm->p_v[(size_t)T3dmA::Count*b_ln[i].i1+2], t3dm->p_v[(size_t)T3dmA::Count*b_ln[i].i1]};
            for (unsigned int j = 0; j != ln_sz; ++j)
            {
                float f_i0_2[2] = {t3dm->p_v[(size_t)T3dmA::Count*b_ln[j].i0+2], t3dm->p_v[(size_t)T3dmA::Count*b_ln[j].i0]};
                float f_i1_2[2] = {t3dm->p_v[(size_t)T3dmA::Count*b_ln[j].i1+2], t3dm->p_v[(size_t)T3dmA::Count*b_ln[j].i1]};

                if (equals(f_i0, f_i0_2) && b_ln[i].i0 > b_ln[j].i0)
                    b_ln[i].i0 = b_ln[j].i0;
                if (equals(f_i0, f_i1_2) && b_ln[i].i0 > b_ln[j].i1)
                    b_ln[i].i0 = b_ln[j].i1;
                if (equals(f_i1, f_i0_2) && b_ln[i].i1 > b_ln[j].i0)
                    b_ln[i].i1 = b_ln[j].i0;
                if (equals(f_i1, f_i1_2) && b_ln[i].i1 > b_ln[j].i1)
                    b_ln[i].i1 = b_ln[j].i1;
            }
        }
        for (unsigned int i = 0; i != ln_sz; ++i) { // vyhození neokrajových linek
            for (unsigned int j = 0; j != ln_sz; ++j) {
                if (b_ln[i].i0 == b_ln[j].i1 && b_ln[i].i1 == b_ln[j].i0) // linka je tam dvakrát
                {
                    b_ln[i].b_ln = 0;
                    break;
                }
            }
        }
        size = 0;
        for (unsigned int i = 0; i != ln_sz; ++i) // zjištění počtu nevyhozených linek
        {
            if (b_ln[i].b_ln)
            {
                ++size;
            }
        }
        bbox.resize(size);

        center[0] = t3dm->p_cen[5];
        center[1] = t3dm->p_cen[3];

        for (unsigned int i = 0, j = 0; i != ln_sz; ++i) // zjištění souřadnic okrajových linek
        {
            if (b_ln[i].b_ln)
            {
                bbox[j].x[0] = t3dm->p_v[(size_t)T3dmA::Count*b_ln[i].i0+(size_t)T3dmA::Pos2];
                bbox[j].x[1] = t3dm->p_v[(size_t)T3dmA::Count*b_ln[i].i0+(size_t)T3dmA::Pos0];

                ladd(bbox[j].x, center); // přepočet do absolutních souřadnic podle středu objektu

                bbox[j].v [0]  = 0.f;
                bbox[j].v [1]  = 1.f;
                bbox[j].v1[0]  = 0.f;
                bbox[j].v1[1]  = 1.f;
                bbox[j].n1[0]  = 1.f;
                bbox[j].n1[1]  = 0.f;
                bbox[j].bnconv = 0;

                bbox[j].next = 0;//bbox;
                for (unsigned int k = 0, l = 0; k != ln_sz; ++k)
                {
                    if (b_ln[k].b_ln)
                    {
                        if (b_ln[i].i1 == b_ln[k].i0)
                        {
                            bbox[j].next = bbox.data()+l;
                            break;
                        }
                        ++l;
                    }
                }
                ++j;
            }
        }
    }

    for (unsigned int i = 0; i != size; ++i) // zjištění, zda navazují (ukazatelem) všechny linky
    {
        if (bbox[i].next == 0)
        {
            bbox[i].next = bbox.data();
        }
    }

    for (unsigned int i = 0; i != size; ++i) // výpočet směrových vektorů linek
    {
        bbox[i].v[0] = bbox[i].next->x[0] - bbox[i].x[0];
        bbox[i].v[1] = bbox[i].next->x[1] - bbox[i].x[1];
        norm_s(bbox[i].v1, bbox[i].v);
    }
    for (unsigned int i = 0; i != size; ++i) // výpočet os úhlů, osy míří dovnitř objektu
    {
        add(bbox[i].next->n1, bbox[i].v1, bbox[i].next->v1);
        perp(bbox[i].next->n1);
        norm_s(bbox[i].next->n1);
        if (dist(bbox[i].x, bbox[i].next->x, bbox[i].next->v1) < 0.f) // vrchol je nekonvexní
        {
            bbox[i].next->bnconv = 1;
        }
    }
}

void RBSolver::init(const float* x, float ax, const float* v, float av, float m, float am, const TimeSync* timesync, const T3dm* t3dm)
{
    p_x0[0] = p_x[0] = x[0];
    p_x0[1] = p_x[1] = x[1];
    p_v[0] = v[0];
    p_v[1] = v[1];
    p_ax0 = p_ax = ax;
    p_av = av;
    p_m = m;
    p_am = am;
    p_bupdm = true;
    p_timesync = timesync;
    zerov(p_F);
    p_aF = 0.f;
    float center[2];
    bbox_create(p_bbox, p_bbox_sz, center, t3dm);
    p_bbox_rot.resize(p_bbox_sz);
    for (unsigned int i = 0; i != p_bbox_sz; ++i)
    {
        p_bbox_rot[i].next = p_bbox_rot.data()+(p_bbox[i].next-p_bbox.data()); // nic moc kód
    }
    p_bcolprev = true;
}

void RBSolver::addF(const float* F, const float* A)
{
    ladd(p_F, F);
    if (A)
    {
        p_aF += len(F)*safe(dist(p_x, A, F));
    }
}

float RBSolver::v_a_dprod()
{
    float ang_vec[2] = {cosf(p_ax), sinf(p_ax)};
    return dprod(p_v, ang_vec);
}

void RBSolver::addFb(const float* F, const float* A)
{
    if (p_bupdm) // pokud rotační matice (jenom hodnota sin a cos alfa) ještě není vytvořena
    {
        p_bupdm = false;
        rot(p_tm, p_ax); // vytvoř rotační matici
    }
    float F1[2]; // transformovaná síla
    rot(F1, p_tm, F); // transformace síly
    ladd(p_F, F1);
    if (A) // pokud síla nepůsobí v těžišti
    {
        float b_x[] = {0, 0};
        p_aF += len(F)*safe(dist(b_x, A, F));
    }
}

void RBSolver::update(bool bstep)
{
    p_bupdm = true;
    if (bstep)
    {
        cpy(p_x_prev, p_x);
        p_ax_prev = p_ax;

        div(p_a, p_F, p_m); // p_a = p_F / p_m
        p_aa = p_aF/p_am;
        ladd(p_v, mul(p_a, p_timesync->p_T)); // p_v += p_a * p_T;
        p_av += p_aa*p_timesync->p_T; // úhlová rychlost += úhlové zrychlení * T
        ladd(p_x, mul(p_v, p_timesync->p_T)); // p_x += p_v * p_T;
        p_ax += p_av*p_timesync->p_T;

        zerov(p_F);
        p_aF = 0;
    } else {
        float a0[2];
        div(a0, p_F, p_m); // p_a = p_F / p_m
        float aa0 = p_aF/p_am;
        float v0[2];
        add(v0, p_v, mul(a0, p_timesync->p_prevT)); // p_v += p_a * p_T;
        float av0 = p_av+aa0*p_timesync->p_prevT; // úhlová rychlost += úhlové zrychlení * T
        add(p_x0, p_x, mul(v0, p_timesync->p_prevT)); // p_x += p_v * p_T;
        p_ax0 = p_ax+av0*p_timesync->p_prevT;

        zerov(p_F);
        p_aF = 0;
    }
}

void RBSolver::get_tm(float* tm)
{
    if (p_bupdm)
    {
        p_bupdm = false;
        rot(p_tm, p_ax); // vytvoř rotační matici
    }
    cpy(tm, p_tm);
}

