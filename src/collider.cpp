#include "platform.h"

#include "collider.h"
//#include <GL/gl.h>
#include <cstdlib>

#include <algorithm>

using namespace RBf;

// odrazivost při kolizích
float g_c_e = 0.1f;

void Collider::init(float gs, float r, const T3dm* t3dm, RBSolver** rbos, unsigned int rbos_sz)
{

    p_gs = gs;
    p_r = r;
    p_r_m2 = p_r*2.f;
    p_r_m2_p2 = p_r_m2*p_r_m2;

    p_rbos = rbos;

    p_rbos_sz = rbos_sz;

    bbox_create(p_bbox, p_bbox_sz, p_cen, t3dm);

    if (p_rbos_sz == 0)
    {
        p_base[0] = 0.f;
        p_base[1] = 0.f;
        p_top[0] = 0.f;
        p_top[1] = 0.f;
    } else {
        p_base[0] = p_top[0] = p_bbox[0].x[0];
        p_base[1] = p_top[1] = p_bbox[0].x[1];
        for (unsigned int i = 0; i != p_bbox_sz; ++i)
        {
            p_base[0] = std::min(p_base[0], p_bbox[i].x[0]); // i*4
            p_base[1] = std::min(p_base[1], p_bbox[i].x[1]);
            p_top[0]  = std::max(p_top [0], p_bbox[i].x[0]);
            p_top[1]  = std::max(p_top [1], p_bbox[i].x[1]);
        }
        p_base[0] -= p_r;
        p_base[1] -= p_r;
        p_top[0] += p_r;
        p_top[1] += p_r;
    }
    p_sz[0] = (unsigned int)ceil((p_top[0] - p_base[0]) / p_gs);
    p_sz[1] = (unsigned int)ceil((p_top[1] - p_base[1]) / p_gs);
    if (p_sz[0] == 0)
        p_sz[0] = 1;
    if (p_sz[1] == 0)
        p_sz[1] = 1;

    p_colg.resize(p_sz[0]*p_sz[1]);

    unsigned int pos[2];
    for (pos[0] = 0; pos[0] != p_sz[0]; ++pos[0])
    {
        for (pos[1] = 0; pos[1] != p_sz[1]; ++pos[1])
        {
            create_grid(pos,
                p_colg[pos[0]+pos[1]*p_sz[0]].lines,
                p_colg[pos[0]+pos[1]*p_sz[0]].points,
                p_colg[pos[0]+pos[1]*p_sz[0]].lines_sz,
                p_colg[pos[0]+pos[1]*p_sz[0]].points_sz);
        }
    }
}

void Collider::test()
{
    // transformace všech objektů podle pozice a úhlu
    for (unsigned int o_i = 0; o_i != p_rbos_sz; ++o_i) // inicializace všech objektů, jejich transformace do globálních souřadnic
    {
        RBSolver &rbo = *(p_rbos[o_i]);
        rbo.p_bcolnow = false;
        float tm[2];
        rot(tm, rbo.p_ax); // tm = {cosf(a), sinf(a)}
        for (unsigned int i = 0; i != rbo.p_bbox_sz; ++i)
        {
            rot(rbo.p_bbox_rot[i].v, tm, rbo.p_bbox[i].v);
            rot(rbo.p_bbox_rot[i].v1, tm, rbo.p_bbox[i].v1);
            rot(rbo.p_bbox_rot[i].n1, tm, rbo.p_bbox[i].n1);
            rot(rbo.p_bbox_rot[i].x, tm, rbo.p_bbox[i].x);
            ladd(rbo.p_bbox_rot[i].x, rbo.p_x);
        }
    }

    // test kolizí objektů s mapou
    for (unsigned int o_i = 0; o_i != p_rbos_sz; ++o_i)
    {
        // kolize s mapou
        RBSolver &rbo = *(p_rbos[o_i]);

        // odstranění zbytečných testů (nehybný objekt)
        if (!rbo.p_bcolprev && equals(rbo.p_x, rbo.p_x_prev) && rbo.p_ax == rbo.p_ax_prev)
            continue;

        // zjištění pozice v kolizní mřížce
        int colg_i = get_subg(rbo.p_x);
        if (colg_i != -1)
        {
            bool bfound = false;
            float normalv[2] = {1.f, 0.f}; // normála nárazu
            float pointc[2] = {0.f, 0.f}; // bod nárazu
            float posunn = 0.f; // posun ve směru normály (?), inicializace jen kvůli warningu
            const ColliderGrid &colg = p_colg[colg_i];

            bool  testA_bcoll = false; // je kolize
            float testA_normalv[2] = {1.f, 0.f}; // normála nárazu
            float testA_pointc[2] = {0.f, 0.f};
            float testA_prunik_vzd_min = HUGE_VAL;//1.f/0.f; // nejmenší vzdálenost průniku bodu do tělesa je kladná

            // procházejí se všechny body mapy
            for (unsigned int i = 0; i != colg.points_sz; ++i)
            {
                const BBox& map_vert = *(colg.points[i]); // aktuální bod mapy
                unsigned int j = 0;
                // procházejí se všechny úsečky bboxu
                while (j != rbo.p_bbox_sz)
                {   // bod musí být na správné straně _VŠECH_ úseček (objekt je konvexní), tzn. uvnitř
                    if (!(safe(distn(map_vert.x, rbo.p_bbox_rot[j].x, rbo.p_bbox_rot[j].v1)) >= 0.001f)) // není uvnitř
                        break;
                    ++j;
                }
                if (j == rbo.p_bbox_sz/* bod je uvnitř objektu */)
                {
                    // projdou se všechny linky bboxu
                    for (unsigned int j = 0; j != rbo.p_bbox_sz; ++j)
                    {
                        const BBox& rb_line = rbo.p_bbox_rot[j]; // aktuální úsečka tělesa
                        float testA_prunik_vzd = distn(map_vert.x, rb_line.x, rb_line.v1); // vzdálenost, kam vnikl bod do tělesa
                        // pokud bod mapy patří k dané úsečce tělesa
                        if (testA_prunik_vzd > 0.f && testA_prunik_vzd < testA_prunik_vzd_min &&
                            distn(map_vert.x, rb_line.x, rb_line.n1) <= 0.f &&
                            distn(map_vert.x, rb_line.next->x, rb_line.next->n1) >= 0.f)
                        {
                            // ještě musí mít úsečka tělesa průsečík se záporným normálovým vektorem bodu mapy
                            float test_int3_vysl[2] = {0.f, 0.f};
                            intersect3(test_int3_vysl, rb_line.x, rb_line.v, map_vert.x, map_vert.n1);
                            if (test_int3_vysl[0] >= 0.f && test_int3_vysl[0] <= 1.f && test_int3_vysl[1] <= 0.f)
                            {
                                cpy( testA_pointc,  map_vert.x);
                                perp(testA_normalv, rb_line.v1);
                                testA_prunik_vzd_min = testA_prunik_vzd;
                                testA_bcoll = true;
                            }
                        }
                    }
                }
            } // end for (i)

            bool  testB_bcoll = false; // je kolize
            float testB_normalv[2] = {1.f, 0.f}; // normála nárazu
            float testB_pointc[2] = {0.f, 0.f};
            float testB_prunik_vzd_max = -1.f; // nejmenší vzdálenost průniku bodu do tělesa je kladná

            if (testA_bcoll)
            {
                bfound = true;
                posunn = -testA_prunik_vzd_min;
                cpy(pointc, testA_pointc);
                cpy(normalv, testA_normalv);
            }// else // projdou se všechny úsečky mapy
            for (unsigned int i = 0; i != colg.lines_sz; ++i)
            {
                const BBox& map_line = *(colg.lines[i]);
                // linka se s objektem testuje, jen pokud je střed objektu nalevo od jejího směrového vektoru ("uvnitř")
                float f_pom0 = 0.f;
                if ((f_pom0 = distn(rbo.p_x, map_line.x, map_line.v1)) >= 0.f)
                {
                    // projdou se všechny body objektu
                    for (unsigned int j = 0; j != rbo.p_bbox_sz; ++j)
                    {
                        const BBox& rb_vert = rbo.p_bbox_rot[j];
                        float testB_prunik_vzd = -distn(rb_vert.x, map_line.x, map_line.v1); // bod proniká směrem ven z mapy, takže průnik směrem do tělesa je záporný
                        if (testB_prunik_vzd > 0.f && testB_prunik_vzd > testB_prunik_vzd_max &&
                            distn(rb_vert.x, map_line.x, map_line.n1) <= 0.f &&
                            distn(rb_vert.x, map_line.next->x, map_line.next->n1) >= 0.f)
                        {
                            float test_int3_vysl[2] = {0.f, 0.f};
                            intersect3(test_int3_vysl, map_line.x, map_line.v, rb_vert.x, rb_vert.n1);
                            if (test_int3_vysl[0] >= 0.f && test_int3_vysl[0] <= 1.f && test_int3_vysl[1] >= 0.f)
                            {
                                testB_pointc[0] = rb_vert.x[0]+rb_vert.n1[0]*test_int3_vysl[1];
                                testB_pointc[1] = rb_vert.x[1]+rb_vert.n1[1]*test_int3_vysl[1];
                                perp(testB_normalv, map_line.v1);
                                testB_prunik_vzd_max = testB_prunik_vzd;
                                testB_bcoll = true;
                            }
                        }
                    }
                }
            }
            if (testB_bcoll)
            {
                bfound = true;
                posunn = -testB_prunik_vzd_max;
                cpy(pointc, testB_pointc);
                cpy(normalv, testB_normalv);
            }

            if (testA_bcoll && testB_bcoll)
            {
                bfound = true;
                if (testB_prunik_vzd_max+0.001 >= testA_prunik_vzd_min)
                {
                    posunn = -testB_prunik_vzd_max;
                    cpy(pointc, testB_pointc);
                    cpy(normalv, testB_normalv);
                } else {
                    posunn = -testA_prunik_vzd_min;
                    cpy(pointc, testA_pointc);
                    cpy(normalv, testA_normalv);
                }
            }

            if (bfound)
            {
                float posun[2];
                mul(posun, normalv, -posunn+0.001);
                safe(posun);
                ladd(rbo.p_x, posun);

                float ap[2], v_ap[2];
                rbo.get_vp(v_ap, ap, pointc);


                // skalární součin okamžité rychlosti bodu kolize a normály
                if (dprod(v_ap, normalv) <= 0.f)
                {
                    float c_e = g_c_e;
                    float dprod_ap_n = dprod(ap, normalv);
                    float c_j = -(1.f+c_e)*dprod(v_ap, normalv) / (len2(normalv)/rbo.p_m+(dprod_ap_n*dprod_ap_n)/rbo.p_am);

                    if (o_i < p_players)
                    {
                        p_sound_crash->play(c_j);
                    }

                    float v2[2];
                    mul(v2, normalv, c_j);
                    ldiv(v2, rbo.p_m);
                    safe(v2);

                    float v_tecna[2];
                    float v_normalova[2];
                    mul(v_normalova, normalv, dprod(normalv, rbo.p_v));
                    sub(v_tecna, rbo.p_v, v_normalova);

                    // toto tady bylo původně - eště trochu změnit
                    damp_v(v_tecna, 0.008f*c_j/* součinitel smykového tření */, 0.01/* časový úsek */);

                    add(rbo.p_v, v_tecna, v_normalova);

                    ladd(rbo.p_v, v2);

                    //

                    rbo.p_av += safe(dprod(ap, normalv)*c_j/rbo.p_am);
                }
                rbo.p_bcolnow = true;
            }
        } // end if (colg_i != -1)
    }




    //zjišťování kolizí objektů navzájem
    for (unsigned int o_i = 0; o_i != p_rbos_sz; ++o_i)
    {
        // kolize s mapou asi těžko
        RBSolver &rbo = *(p_rbos[o_i]);
        for (unsigned int o_i2 = o_i+1; o_i2 < p_rbos_sz; ++o_i2) // kolize s ostatními objekty
        {
            RBSolver &rbo2 = *(p_rbos[o_i2]);

            if (!rbo.p_bcolnow && !rbo.p_bcolprev && equals(rbo.p_x, rbo.p_x_prev) && rbo.p_ax == rbo.p_ax_prev
                && !rbo2.p_bcolnow && !rbo2.p_bcolprev && equals(rbo2.p_x, rbo2.p_x_prev) && rbo2.p_ax == rbo2.p_ax_prev)
            {
                continue;
            } else {
            }

            float diff_prac[2]; // vzdálenost objektů
            sub(diff_prac, rbo2.p_x, rbo.p_x);
            if (diff_prac[0] >= -p_r_m2 && diff_prac[0] <= p_r_m2 &&
                diff_prac[1] >= -p_r_m2 && diff_prac[1] <= p_r_m2 &&
                len2(diff_prac) <= p_r_m2_p2) // mohlo dojít ke kolizi
            {
                bool bintersect = false;

                for (unsigned int i = 0; i != rbo.p_bbox_sz; ++i)
                    rbo.p_bbox_rot[i].bnconv = 0; // není průsečík

                for (unsigned int i = 0; i != rbo2.p_bbox_sz; ++i)
                    rbo2.p_bbox_rot[i].bnconv = 0; // není průsečík

                for (unsigned int i = 0; i != rbo.p_bbox_sz; ++i)
                {
                    for (unsigned int j = 0; j != rbo2.p_bbox_sz; ++j)
                    {
                        if (!(rbo.p_bbox_rot[i].bnconv && rbo2.p_bbox_rot[j].bnconv))
                        {
                            float int_res[2];
                            intersect3(int_res, rbo.p_bbox_rot[i].x, rbo.p_bbox_rot[i].v, rbo2.p_bbox_rot[j].x, rbo2.p_bbox_rot[j].v);
                            if (isintersect(int_res))
                            {
                                rbo.p_bbox_rot[i].bnconv = 1;
                                rbo2.p_bbox_rot[j].bnconv = 1;
                                bintersect = true;
                            }
                        }
                    }
                }
                // nalezení kolizních údajů
                bool bfound = false;
                float normalv[2] = {0};
                float pointc[2];
                float posunn = 0.f; // inicializace jen kvůli warningu
                bool bfound2 = false;
                float normalv2[2] = {0};
                float pointc2[2] = {0};
                float posunn2 = 0.f; // inicializace jen kvůli warningu
                if (bintersect)
                {
                    for (unsigned int i = 0; i != rbo.p_bbox_sz; ++i)
                    {
                        // každý bod se otestuje na přítomnost v objektu 2
                        unsigned int j = 0;
                        while (j != rbo2.p_bbox_sz)
                        {
                            if (!(distn(rbo.p_bbox_rot[i].x, rbo2.p_bbox_rot[j].x, rbo2.p_bbox_rot[j].v1) > 0.001f)) // není uvnitř
                                break;
                            ++j;
                        }
                        if (j == rbo2.p_bbox_sz)
                        {
                            for (unsigned int j = 0; j != rbo2.p_bbox_sz; ++j)
                            {
                                if (rbo2.p_bbox_rot[j].bnconv)
                                {
                                    float posunn_prac = -distn(rbo.p_bbox_rot[i].x, rbo2.p_bbox_rot[j].x, rbo2.p_bbox_rot[j].v1);
                                    if (posunn_prac < 0.f && (!bfound || posunn_prac > posunn))
                                    {
                                        perp(normalv, rbo2.p_bbox_rot[j].v1);
                                        neg(normalv);
                                        cpy(pointc, rbo.p_bbox_rot[i].x);
                                        posunn = posunn_prac;
                                        bfound = true;
                                    }
                                }
                            }
                        }
                    }
                    for (unsigned int i = 0; i != rbo2.p_bbox_sz; ++i)
                    {
                        unsigned int j = 0;
                        while (j != rbo.p_bbox_sz)
                        {
                            if (!(distn(rbo2.p_bbox_rot[i].x, rbo.p_bbox_rot[j].x, rbo.p_bbox_rot[j].v1) > 0.001f)) // není uvnitř
                                break;
                            ++j;
                        }
                        if (j == rbo.p_bbox_sz)
                        {
                            for (unsigned int j = 0; j != rbo.p_bbox_sz; ++j)
                            {
                                if (rbo.p_bbox_rot[j].bnconv)
                                {
                                    float posunn_prac = -distn(rbo2.p_bbox_rot[i].x, rbo.p_bbox_rot[j].x, rbo.p_bbox_rot[j].v1);
                                    if (posunn_prac < 0.f && (!bfound2 || posunn_prac > posunn2))
                                    {
                                        perp(normalv2, rbo.p_bbox_rot[j].v1);
                                        cpy(pointc2, rbo2.p_bbox_rot[i].x);
                                        posunn2 = posunn_prac;
                                        bfound2 = true;
                                    }
                                }
                            }
                        }
                    }
                    if (bfound2)
                    {
                        if (!bfound)
                        {
                            cpy(normalv, normalv2);
                            cpy(pointc, pointc2);
                            posunn = posunn2;
                            bfound = true;
                        } else {
                            if (posunn2 < posunn)
                            {
                                cpy(normalv, normalv2);
                                cpy(pointc, pointc2);
                                posunn = posunn2;
                            }
                        }
                    }
                }

                if (bfound)
                {
                    // odsun (půl na půl), změna rychlostí
                    float posun[2];
                    mul(posun, normalv, -posunn*0.5f+0.0005);
                    safe(posun);
                    ladd(rbo.p_x, posun);
                    lsub(rbo2.p_x, posun);

                    float ap[2], v_ap[2];
                    rbo.get_vp(v_ap, ap, pointc);

                    float bp[2], v_bp[2];
                    rbo2.get_vp(v_bp, bp, pointc);

                    float v_ab[2];
                    sub(v_ab, v_ap, v_bp);

                    if (dprod(v_ab, normalv) <= 0.f)
                    {
                        float c_e = g_c_e;
                        float dprod_ap_n = dprod(ap, normalv);
                        float dprod_bp_n = dprod(bp, normalv);
                        float c_j = -(1.f+c_e)*dprod(v_ab, normalv) / (len2(normalv)*(1.f/rbo.p_m+1.f/rbo2.p_m)+(dprod_ap_n*dprod_ap_n)/rbo.p_am+(dprod_bp_n*dprod_bp_n)/rbo2.p_am);

                        if (o_i < p_players || o_i2 < p_players)
                        {
                            p_sound_crash->play(c_j);
                        }

                        float v2[2];
                        mul(v2, normalv, c_j);
                        ldiv(v2, rbo.p_m);
                        safe(v2);
                        ladd(rbo.p_v, v2);

                        rbo.p_av += safe(dprod(ap, normalv)*c_j/rbo.p_am);

                        mul(v2, normalv, -c_j);
                        ldiv(v2, rbo2.p_m);
                        safe(v2);
                        ladd(rbo2.p_v, v2);

                        rbo2.p_av += safe(dprod(bp, normalv)*(-c_j)/rbo2.p_am);
                    }

                    rbo.p_bcolnow = true;
                    rbo2.p_bcolnow = true;
                }
            }
        }
    }
    // aktualizace informací o přítomnosti kolize v předchozím kroku
    for (unsigned int o_i = 0; o_i != p_rbos_sz; ++o_i)
    {
        RBSolver &rbo = *(p_rbos[o_i]);
        rbo.p_bcolprev = rbo.p_bcolnow;
    }
}

void Collider::create_grid(const unsigned int pos[2],
        std::vector<const BBox*> &ilines, std::vector<const BBox*> &ipoints, unsigned int &lines_sz, unsigned int &points_sz, bool pass2)
{
    if (pass2)
    {
        ilines.resize(lines_sz);
        ipoints.resize(points_sz);
    }
    unsigned int lsz = 0;
    unsigned int psz = 0;
    for (unsigned int i = 0; i != p_bbox_sz; ++i)
    {
        if (test_line(pos, i))
        {
            if (pass2)
            {
                ilines[lsz] = p_bbox.data()+i;
            }
            ++lsz;
        }
        if (test_point(pos, i))
        {
            if (pass2)
            {
                ipoints[psz] = p_bbox.data()+i;
            }
            ++psz;
        }
    }
    lines_sz = lsz;
    points_sz = psz;
    if (!pass2)
        create_grid(pos, ilines, ipoints, lines_sz, points_sz, true);
}

bool Collider::test_line(const unsigned int pos[2], unsigned int iline)
{
    const float& x = p_bbox[iline].x[0];
    const float& y = p_bbox[iline].x[1];

    const float& x2 = p_bbox[iline].next->x[0];
    const float& y2 = p_bbox[iline].next->x[1];

    float xmin = p_base[0]+pos[0]*p_gs-p_r;
    float xmax = p_base[0]+(pos[0]+1)*p_gs+p_r;
    float ymin = p_base[1]+pos[1]*p_gs-p_r;
    float ymax = p_base[1]+(pos[1]+1)*p_gs+p_r;
    if (x <= xmax && x >= xmin && y <= ymax && y >= ymin)
        return true;
    if (x2 <= xmax && x2 >= xmin && y2 <= ymax && y2 >= ymin)
        return true;
    float i_result;
    float i_result2;

    float u_x[2] = {p_gs+p_r*2.f, 0.f};
    float u_y[2] = {0.f, p_gs+p_r*2.f};

    float a0[2] = {xmin, ymin};
    float a1[2] = {xmin, ymax};
    float a2[2] = {xmax, ymin};

    float b0[2];
    float b1[2];
    float b2[2];
    sub(b0, p_bbox[iline].x, a0);
    sub(b1, p_bbox[iline].x, a1);
    sub(b2, p_bbox[iline].x, a2);

    i_result = intersect(u_x, b0, p_bbox[iline].v);
    i_result2 = intersect2(u_x, b0, p_bbox[iline].v);
    if (i_result >= 0.f && i_result <= 1.f && i_result2 >= 0.f && i_result2 <= 1.f) return true;

    i_result = intersect(u_y, b0, p_bbox[iline].v);
    i_result2 = intersect2(u_y, b0, p_bbox[iline].v);
    if (i_result >= 0.f && i_result <= 1.f && i_result2 >= 0.f && i_result2 <= 1.f) return true;

    i_result = intersect(u_x, b1, p_bbox[iline].v);
    i_result2 = intersect2(u_x, b1, p_bbox[iline].v);
    if (i_result >= 0.f && i_result <= 1.f && i_result2 >= 0.f && i_result2 <= 1.f) return true;

    i_result = intersect(u_y, b2, p_bbox[iline].v);
    i_result2 = intersect2(u_y, b2, p_bbox[iline].v);
    if (i_result >= 0.f && i_result <= 1.f && i_result2 >= 0.f && i_result2 <= 1.f) return true;

    return false;
}

bool Collider::test_point(const unsigned int pos[2], unsigned int ipoint)
{
    if (p_bbox[ipoint].bnconv == 0)
        return false;
    const float& x = p_bbox[ipoint].x[0];
    const float& y = p_bbox[ipoint].x[1];
    float xmin = p_base[0]+pos[0]*p_gs-p_r;
    float xmax = p_base[0]+(pos[0]+1)*p_gs+p_r;
    float ymin = p_base[1]+pos[1]*p_gs-p_r;
    float ymax = p_base[1]+(pos[1]+1)*p_gs+p_r;
    if (x <= xmax && x >= xmin && y <= ymax && y >= ymin)
        return true;
    return false;
}

int Collider::get_subg(const float ppos[2]) const
{
    float pos[2];
    pos[0] = ppos[0] - p_base[0];
    pos[1] = ppos[1] - p_base[1];
    int i_pos[2] = {(int)floor(pos[0]/p_gs), (int)floor(pos[1]/p_gs)};
    if (i_pos[0] < 0 || i_pos[0] >= int(p_sz[0]) || i_pos[1] < 0 || i_pos[1] >= int(p_sz[1]))
        return -1;
    return i_pos[0]+i_pos[1]*p_sz[0]; // x+y*w
}

