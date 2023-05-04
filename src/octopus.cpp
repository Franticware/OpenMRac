#include "glhelpers1.h"
#include "octopus.h"

#include <algorithm>
#include <GL/gl.h>
#include "triboxint.h"
#include <cmath>

Octocube::Octocube() : p_mi(0), p_up(0), p_r(0), p_a_2(0), p_lev(0), p_base(0)
{
    for (int i = 0; i != 8; ++i)
    {
        p_sub[i] = 0;
    }
    for (int i = 0; i != 3; ++i)
    {
        p_cen[i] = 0;
        p_cen_t[i] = 0;
    }
}

void Octocube::count_maxsub(unsigned int* count) // spočtení maximálního množství subdivide pro velikost pole pro uložení viditelných částí
{
    if (p_sub[0] == 0 && p_sub[1] == 0 && p_sub[2] == 0 && p_sub[3] == 0
        && p_sub[4] == 0 && p_sub[5] == 0 && p_sub[6] == 0 && p_sub[7] == 0)
    {
        ++(*count);
    } else {
        for (unsigned int i = 0; i != 8; ++i)
        {
            if (p_sub[i] != 0)
                p_sub[i]->count_maxsub(count);
        }
    }
}

int Octocube::render_pass1(const float modelview_matrix[16]) // základní funkce pro zjištění viditelných částí
{
    int results[8] = {0};
    switch (test(modelview_matrix)) // test viditelnosti dané části
    {
    case 0:
        return 0; // pokud je tento bbox mimo, předá se výsledek 0 nadřazené úrovni
    case 1:
        return 1; // pokud je tento bbox vidět úplně, předá se vásledek 1 pro zpracování nadřazené úrovni
    case 2: // pokud je vidět jen část bboxu, zjistí se, jak je to s podřazenými částmi
        for (unsigned int i = 0; i != 8; ++i)
        {
            if (p_sub[i]) // pokud podřazená část existuje, otestuje se viditelnost
                results[i] = p_sub[i]->render_pass1(modelview_matrix);
        }
        if ((p_sub[0] == 0 || (results[0] == 1)) &&
            (p_sub[1] == 0 || (results[1] == 1)) &&
            (p_sub[2] == 0 || (results[2] == 1)) &&
            (p_sub[3] == 0 || (results[3] == 1)) &&
            (p_sub[4] == 0 || (results[4] == 1)) &&
            (p_sub[5] == 0 || (results[5] == 1)) &&
            (p_sub[6] == 0 || (results[6] == 1)) &&
            (p_sub[7] == 0 || (results[7] == 1)))
            return 1; // pokud jsou vidět všechny existující podřazené bboxy uvnitř viewportu, může se vykreslit celý aktuální bbox vcelku (všechno, co obsahuje, je vidět)
        // pokud jsme tady, některá podřazená část není vůbec vidět
        for (unsigned int i = 0; i != 8; ++i)
        {
            if (p_sub[i] != 0 && results[i]) // všechny existující podřazené a viditelné bboxy se zařadí pro vykreslení
            {
                p_base->p_vw[p_base->p_vw_sz] = p_sub[i];
                ++p_base->p_vw_sz;
            }
        }
    }
    return 0; // podřazené části už byly zařazeny, nadřazenému bboxu se předá výsledek 0, který znamená, že bbox není úplně vidět
}

void Octocube::render_pass2() // vykreslení všech částí patřících octocube po jednotlivých materiálech
{
    for (unsigned int i = 0; i != p_base->p_m_sz; ++i)
    {
        if (p_mi[i].p_sz)
        {
            glDrawElements(GL_TRIANGLES, p_mi[i].p_sz, GL_UNSIGNED_SHORT, p_mi[i].p_i); checkGL(); afterDrawcall();
        }
    }
}

int Octocube::test(const float modelview_matrix[16]) // 0 - mimo, 1 - úplně, 2 - část
{
    p_cen_t[0] = p_cen[0]*modelview_matrix[0]+p_cen[1]*modelview_matrix[4]+p_cen[2]*modelview_matrix[8 ]+modelview_matrix[12];
    p_cen_t[1] = p_cen[0]*modelview_matrix[1]+p_cen[1]*modelview_matrix[5]+p_cen[2]*modelview_matrix[9 ]+modelview_matrix[13];
    p_cen_t[2] = p_cen[0]*modelview_matrix[2]+p_cen[1]*modelview_matrix[6]+p_cen[2]*modelview_matrix[10]+modelview_matrix[14];

    float s_l = 0.f;
    float s_r = 0.f;
    float s_b = 0.f;
    float s_t = 0.f;
    float s_n = 0.f;
    float s_f = 0.f;

    s_n = -p_cen_t[2]-p_base->p_near;
    if (s_n < -p_r) return 0;
    s_f = p_base->p_far+p_cen_t[2];
    if (s_f < -p_r) return 0;
    s_b = (p_base->p_bottom[0]*p_cen_t[2]-p_base->p_bottom[1]*p_cen_t[1]);
    if (s_b < -p_r) return 0;
    s_t = (p_base->p_top[1]*p_cen_t[1]-p_base->p_top[0]*p_cen_t[2]);
    if (s_t < -p_r) return 0;
    s_l = (p_base->p_left[0]*p_cen_t[2]-p_base->p_left[1]*p_cen_t[0]);
    if (s_l < -p_r) return 0;
    s_r = (p_base->p_right[1]*p_cen_t[0]-p_base->p_right[0]*p_cen_t[2]);
    if (s_r < -p_r) return 0;

    if (s_n > p_r &&
        s_f > p_r &&
        s_b > p_r &&
        s_t > p_r &&
        s_l > p_r &&
        s_r > p_r)
        return 1;
    //
    return 2;
}

void Octocube::subdiv()
{
    // záhadný komentář
    // zjistit počet indexů a level, pokud jsou hodnoty mimo hranice, vrátit se

    unsigned int up_trisnum = 0;
    for (unsigned int i = 0; i != p_base->p_m_sz; ++i) // zjištění počtu trojúhelníků v rodičovské úrovni, pokud jich tato úroveň nemá dost, nebude se dál dělit
        up_trisnum += p_mi[i].p_sz/3;
    if (up_trisnum < p_base->p_min_tris) // pokud jsme v úrovni, kde už je málo trojúhelníků, nemá cenu dál dělit úrovně
        return;

    unsigned int sublev = p_lev+1; // výpočet čísla úrovně
    if (sublev > p_base->p_max_lev) // pokud existuje příliš mnoho úrovní, další jejich tvorba skončí
        return;

    float sub_a_2 = p_a_2*0.5f;
    float cen0[3] = {p_cen[0]-sub_a_2, p_cen[1]-sub_a_2, p_cen[2]-sub_a_2};
    float cen1[3] = {p_cen[0]+sub_a_2, p_cen[1]-sub_a_2, p_cen[2]-sub_a_2};
    float cen2[3] = {p_cen[0]-sub_a_2, p_cen[1]+sub_a_2, p_cen[2]-sub_a_2};
    float cen3[3] = {p_cen[0]+sub_a_2, p_cen[1]+sub_a_2, p_cen[2]-sub_a_2};
    float cen4[3] = {p_cen[0]-sub_a_2, p_cen[1]-sub_a_2, p_cen[2]+sub_a_2};
    float cen5[3] = {p_cen[0]+sub_a_2, p_cen[1]-sub_a_2, p_cen[2]+sub_a_2};
    float cen6[3] = {p_cen[0]-sub_a_2, p_cen[1]+sub_a_2, p_cen[2]+sub_a_2};
    float cen7[3] = {p_cen[0]+sub_a_2, p_cen[1]+sub_a_2, p_cen[2]+sub_a_2};
    float* centers[8] = {cen0, cen1, cen2, cen3, cen4, cen5, cen6, cen7};
    float subr[3] = {sub_a_2+0.0001f, sub_a_2+0.0001f, sub_a_2+0.0001f};
    for (unsigned int i = 0; i != 8; ++i)
    {
        bool bcreated = false;
        Octocube* oc_ptr = 0;
        for (unsigned int j = 0; j != p_base->p_m_sz; ++j)
        {
            unsigned int sz = 0;
            // spočítat indexy příslušného materiálu
            for (unsigned int k = 0; k != p_mi[j].p_sz/3; ++k)
            {
                unsigned int iv0 = p_mi[j].p_i[k*3];
                unsigned int iv1 = p_mi[j].p_i[k*3+1];
                unsigned int iv2 = p_mi[j].p_i[k*3+2];
                float v0[3] = {p_base->p_t3dm->p_v[iv0*3], p_base->p_t3dm->p_v[iv0*3+1], p_base->p_t3dm->p_v[iv0*3+2]};
                float v1[3] = {p_base->p_t3dm->p_v[iv1*3], p_base->p_t3dm->p_v[iv1*3+1], p_base->p_t3dm->p_v[iv1*3+2]};
                float v2[3] = {p_base->p_t3dm->p_v[iv2*3], p_base->p_t3dm->p_v[iv2*3+1], p_base->p_t3dm->p_v[iv2*3+2]};
                if (triboxint(centers[i], subr, v0, v1, v2)) // pokud je uvnitř krychle
                    sz += 3;
            }

            if (sz != 0 && !bcreated)
            {
                bcreated = true;
                p_sub[i] = new Octocube;
                oc_ptr = p_sub[i];
                // inicializace - dodělat
                memcpy(oc_ptr->p_cen, centers[i], sizeof(float)*3);
                oc_ptr->p_a_2 = sub_a_2;
                oc_ptr->p_r = p_r*0.5f;
                oc_ptr->p_mi = new Octocm[p_base->p_m_sz];
                oc_ptr->p_lev = sublev;
                oc_ptr->p_base = p_base;
                oc_ptr->p_face_num = 0;
            }
            if (sz != 0)
            {
                p_sub[i]->p_mi[j].p_sz = sz;
                p_sub[i]->p_mi[j].p_i = new unsigned short[sz];
                p_sub[i]->p_face_num += sz;
                // zkopírovat příslušné indexy daného materiálu
                for (unsigned int k = 0, l = 0; k != p_mi[j].p_sz/3; ++k)
                {
                    unsigned int iv0 = p_mi[j].p_i[k*3];
                    unsigned int iv1 = p_mi[j].p_i[k*3+1];
                    unsigned int iv2 = p_mi[j].p_i[k*3+2];
                    float v0[3] = {p_base->p_t3dm->p_v[iv0*3], p_base->p_t3dm->p_v[iv0*3+1], p_base->p_t3dm->p_v[iv0*3+2]};
                    float v1[3] = {p_base->p_t3dm->p_v[iv1*3], p_base->p_t3dm->p_v[iv1*3+1], p_base->p_t3dm->p_v[iv1*3+2]};
                    float v2[3] = {p_base->p_t3dm->p_v[iv2*3], p_base->p_t3dm->p_v[iv2*3+1], p_base->p_t3dm->p_v[iv2*3+2]};
                    if (triboxint(centers[i], subr, v0, v1, v2)) // pokud je uvnitř krychle
                    {
                        p_sub[i]->p_mi[j].p_i[l] = iv0;
                        ++l;
                        p_sub[i]->p_mi[j].p_i[l] = iv1;
                        ++l;
                        p_sub[i]->p_mi[j].p_i[l] = iv2;
                        ++l;
                    }
                }
            }
        }

        if (bcreated)
        {
            oc_ptr->subdiv();
        }
    }
}

Octocube::~Octocube()
{
    delete[] p_mi;
    for (unsigned int i = 0; i != 8; ++i)
        delete p_sub[i];
}

namespace Oct_f
{

void norm2(float* f)
{
    float len = sqrtf(f[0]*f[0]+f[1]*f[1]);
    f[0] /= len;
    f[1] /= len;
}

}

void Octopus::init_frustum(const float frustum[6])
{
    p_left[0]   = frustum[0];
    p_left[1]   = -1.f;
    Oct_f::norm2(p_left);
    p_right[0]  = frustum[1];
    p_right[1]  = -1.f;
    Oct_f::norm2(p_right);
    p_bottom[0] = frustum[2];
    p_bottom[1] = -1.f;
    Oct_f::norm2(p_bottom);
    p_top[0]    = frustum[3];
    p_top[1]    = -1.f;
    Oct_f::norm2(p_top);
    p_near      = frustum[4];
    p_far       = frustum[5];
}

void Octopus::init(const float frustum[6], const T3dm& t3dm, unsigned int min_tris, unsigned int max_lev)
{
    init_frustum(frustum);

    p_oc = new Octocube;
    p_t3dm = &t3dm;
    p_m_sz = p_t3dm->p_m_sz; // (ne nezbytně) získání vlastní hodnoty počtu materiálů v modelu

    float xmin = 0.f, xmax = 0.f, ymin = 0.f, ymax = 0.f, zmin = 0.f, zmax = 0.f;
    bool bbnds = false; // jsou inicializovány hranice? nebo spíš obsahují hodnoty hranic už nějaké platné hodnoty?

    p_oc->p_mi = new Octocm[p_m_sz]; // proměnné v poli jsou inicializovány
    p_oc->p_base = this;
    for (unsigned int i = 0; i != p_t3dm->p_sz; ++i)
    {
        O3dm &objekt = p_t3dm->p_o[i];
        if (objekt.p_gi == 0) // do octopu se zařadí jen skupina číslo 0
        {
            p_oc->p_mi[objekt.p_m].p_sz += objekt.p_sz/3*3;
            for (unsigned int j = 0; j != objekt.p_sz/3*3; ++j)
            {
                if (!bbnds)
                {
                    xmin = xmax = p_t3dm->p_v[objekt.p_i[j]*3];
                    ymin = ymax = p_t3dm->p_v[objekt.p_i[j]*3+1];
                    zmin = zmax = p_t3dm->p_v[objekt.p_i[j]*3+2];
                    bbnds = true;
                } else {
                    xmin = std::min(xmin, p_t3dm->p_v[objekt.p_i[j]*3]);
                    ymin = std::min(ymin, p_t3dm->p_v[objekt.p_i[j]*3+1]);
                    zmin = std::min(zmin, p_t3dm->p_v[objekt.p_i[j]*3+2]);
                    xmax = std::max(xmax, p_t3dm->p_v[objekt.p_i[j]*3]);
                    ymax = std::max(ymax, p_t3dm->p_v[objekt.p_i[j]*3+1]);
                    zmax = std::max(zmax, p_t3dm->p_v[objekt.p_i[j]*3+2]);
                }
            }
        }
    }
    float xsz = xmax - xmin;
    float ysz = ymax - ymin;
    float zsz = zmax - zmin;
    float szmax = std::max(xsz, std::max(ysz, zsz));
    p_oc->p_a_2 = szmax/2.f;
    p_oc->p_cen[0] = xmin+p_oc->p_a_2;
    p_oc->p_cen[1] = ymin+p_oc->p_a_2;
    p_oc->p_cen[2] = zmin+p_oc->p_a_2;
    p_oc->p_r = p_oc->p_a_2*1.732050807568877f; // poloměr bounding sphere se spočítá jako polovina hrany bboxu * sqrt(3)
    p_oc->p_face_num = 0;
    for (unsigned int i = 0; i != p_m_sz; ++i)
    {
        unsigned int l = 0;
        p_oc->p_mi[i].p_i = new unsigned short[p_oc->p_mi[i].p_sz];
        for (unsigned int j = 0; j != p_t3dm->p_sz; ++j)
        {
            O3dm &objekt = p_t3dm->p_o[j];
            if (objekt.p_gi == 0 && objekt.p_m == i) // do octopu se zařazují jen objekty s group number 0
            {
                for (unsigned int k = 0; k != objekt.p_sz/3*3; ++k)
                {
                    p_oc->p_mi[i].p_i[l] = objekt.p_i[k];
                    ++l;
                }
            }
        }
        p_oc->p_face_num += l;
    }
    p_oc->p_lev = 0;
    p_min_tris = min_tris;
    p_max_lev = max_lev;
    p_oc->subdiv(); // vytvoření podřazených bboxů
    unsigned int vw_sz = 0;
    p_oc->count_maxsub(&vw_sz);
    p_vw = new Octocube*[vw_sz];
}

typedef const Octocube* Octocube_ptr;

bool zsort_comp(const Octocube_ptr &o1, const Octocube_ptr &o2)
{
    return o1->p_cen_t[2] > o2->p_cen_t[2];
}

void Octopus::render_pass1(const float modelview_matrix[16]) // začátek testování bboxů
{
    p_vw_sz = 0;
    if (p_oc->render_pass1(modelview_matrix) == 1) // pokud je už základní bbox vidět celý, přidá se do pole jen ten, v opačném případě se do pole pro vykreslování přidají až (některé) podřazené části
    {
        p_vw[p_vw_sz] = p_oc;
        ++p_vw_sz;
    }
    std::sort(p_vw, p_vw+p_vw_sz, zsort_comp);
}

void Octopus::render_pass1_lim(const float modelview_matrix[16], unsigned int face_limit) // začátek testování bboxů
{
    if (face_limit == 0)
    {
        render_pass1(modelview_matrix);
        return;
    }
    p_vw_sz = 0;
    if (p_oc->render_pass1(modelview_matrix) == 1) // pokud je už základní bbox vidět celý, přidá se do pole jen ten, v opačném případě se do pole pro vykreslování přidají až (některé) podřazené části
    {
        p_vw[p_vw_sz] = p_oc;
        ++p_vw_sz;
    }
    std::sort(p_vw, p_vw+p_vw_sz, zsort_comp);
    unsigned int vw_sz_pom = 0;
    unsigned int face_num = 0;
    for (unsigned int i = 0; i != p_vw_sz; ++i)
    {
        face_num += p_vw[i]->p_face_num;
        if (face_num > face_limit) // pocet facu s aktualni octocube >= face_limit
        {
            vw_sz_pom = i+1;
            break;
        }
    }
    p_vw_sz = vw_sz_pom;
}

void Octopus::render_pass2() // vykreslení všech octocube v seznamu
{
    glEnableClientState(GL_VERTEX_ARRAY); checkGL();
    glEnableClientState(GL_NORMAL_ARRAY); checkGL();
    glEnableClientState(GL_TEXTURE_COORD_ARRAY); checkGL();
    glVertexPointer(3,GL_FLOAT,0,p_t3dm->p_v); checkGL();
    glNormalPointer(GL_FLOAT,0,p_t3dm->p_n); checkGL();
    glTexCoordPointer(2,GL_FLOAT,0,p_t3dm->p_t); checkGL();

    for (unsigned int i = 0; i != p_vw_sz; ++i)
    {
        p_vw[i]->render_pass2();
    }

    glDisableClientState(GL_TEXTURE_COORD_ARRAY); checkGL();
    glDisableClientState(GL_NORMAL_ARRAY); checkGL();
    glDisableClientState(GL_VERTEX_ARRAY); checkGL();
}

#if 0 // unused
void Octopus::render_level(unsigned int lev) // (pracovní, a proto komplikované) vykreslení jen určité viditelné úrovně
{
    glEnableClientState(GL_VERTEX_ARRAY); checkGL();
    glEnableClientState(GL_NORMAL_ARRAY); checkGL();
    glEnableClientState(GL_TEXTURE_COORD_ARRAY); checkGL();
    glVertexPointer(3,GL_FLOAT,0,p_t3dm->p_v); checkGL();
    glNormalPointer(GL_FLOAT,0,p_t3dm->p_n); checkGL();
    glTexCoordPointer(2,GL_FLOAT,0,p_t3dm->p_t); checkGL();

    p_oc->render_level_pom(lev);

    glDisableClientState(GL_TEXTURE_COORD_ARRAY); checkGL();
    glDisableClientState(GL_NORMAL_ARRAY); checkGL();
    glDisableClientState(GL_VERTEX_ARRAY); checkGL();
}

void Octocube::render_level_pom(unsigned int lev) // pomocná funkce k předchozí
{
    if (p_lev == lev)
    {
        float color[3] = {(rand()%3)*0.5f, (rand()%3)*0.5f, (rand()%3)*0.5f};
        glColor3fv(color); checkGL();
        for (unsigned int i = 0; i != p_base->p_m_sz; ++i)
        {
            if (p_mi[i].p_sz)
            {
                glDrawElements(GL_TRIANGLES, p_mi[i].p_sz, GL_UNSIGNED_SHORT, p_mi[i].p_i); checkGL();
            }
        }
    } else {
        for (unsigned int i = 0; i != 8; ++i)
        {
            if (p_sub[i])
                p_sub[i]->render_level_pom(lev);
        }
    }
}
#endif

Octopus::~Octopus()
{
    delete p_oc; // smazání základního bboxu (v destruktoru tohoto bboxu se mažou podřazené bboxy)
    delete[] p_vw; // smazání pole pro ukládání viditelných objektů
}
