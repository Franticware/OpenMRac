#include "3dm.h"
#include "gbuff_in.h"
#include "cstring1.h"
#include "glm1.h"

#include <cstdio>
#include <cstring>
#include <cmath>
#include <algorithm>

void O3dm::initBuf()
{
    GLuint tmpBuf;
    glGenBuffers(1, &tmpBuf);
    p_buf = tmpBuf;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, p_buf);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, p_i.size() * sizeof(GLushort), p_i.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void T3dm::initBuf()
{
    GLuint tmpBuf;
    glGenBuffers(1, &tmpBuf);
    p_buf = tmpBuf;
    glBindBuffer(GL_ARRAY_BUFFER, p_buf);
    glBufferData(GL_ARRAY_BUFFER, p_v.size() * sizeof(float), p_v.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

int T3dm::getgidobj(unsigned int gid) const
{
    for (unsigned int i = 0; i != p_o.size(); ++i)
    {
        if (p_o[i].p_gi == gid)
            return i;
    }
    return -1;
}

void T3dm::scale(float aspect)
{
    unsigned int groups_sz = 0;
    unsigned int verts_sz = 0;
    for (unsigned int i = 0; i != p_o.size(); ++i)
    {
        O3dm& object = p_o[i];
        if (object.p_gi+1 > groups_sz)
            groups_sz = object.p_gi+1;
        for (unsigned int j = 0; j != object.p_i.size(); ++j)
            if (static_cast<unsigned int>(object.p_i[j])+1 > verts_sz)
                verts_sz = object.p_i[j]+1;
    }
    for (unsigned int i = 0; i != groups_sz*3; ++i)
        p_cen[i] *= aspect;
    for (size_t i = 0; i != verts_sz; ++i)
    {
        for (size_t j = (size_t)T3dmA::Pos0; j <= (size_t)T3dmA::Pos2; ++j)
        {
            p_v[i*(size_t)T3dmA::Count+j] *= aspect;
        }
    }
}

void T3dm::load(const char* fname, const char** o_names)
{
    clear(); // vymazání předchozího
    if (!gbuff_in.f_open(fname, "r"))
        return;
    unsigned int vertexnum = 0; // celkový počet objektů
    char buff[1024]; // buffer pro načtení řádků
    char buff2[1024]; // buffer pro porovnání názvu objektu
    while (gbuff_in.fgets(buff, 1024)) // sečtení všech vertexů
    {
        unsigned int vertexnum1 = 0;
        if (sscanf(buff, "v %u", &vertexnum1) == 1)
            vertexnum += vertexnum1;
    }
    gbuff_in.rewind();
    //p_v_sz = vertexnum;
    p_v.clear();
    p_v.resize(vertexnum * (size_t)T3dmA::Count, 0.f);
    std::vector<unsigned char> nf(vertexnum, 0); // normal flag (flag společné normály)
    unsigned int objectnum = 0, texturenum = 0; // počet objektů, textur
    unsigned int object_i = 0; // index aktuálního objektu
    bool b_otnum = false; // načten počet objektů a textur
    bool b_eof = false; // konec souboru
    std::vector<unsigned char> mainflags; // flag hlavního objektu

    std::vector<unsigned int> vertnums; // pole počtu bodů pro objekty (pro zpracování object names)

    unsigned int o_names_num = 0; // počet jmen (minimálně 1)
    if (o_names) // zjištění počtu jmen
        for (o_names_num = 0; o_names[o_names_num] != 0; ++o_names_num);
    o_names_num = std::max(o_names_num, (unsigned int)1);
    p_cen.clear();
    p_cen.resize(o_names_num*3, 0.f); // alokace pole středů hlavních objektů
    std::vector<float> centers; // středy objektů
    unsigned int points_i0 = 0; // mezivýpočet hodnoty počtu bodů
    unsigned int points_n = 0; // proměnná pro načtení počtu bodů
    unsigned int faces_n = 0; // proměnná pro načtení počtu pložek
    unsigned int points_i0_prev = 0; // spolupráce s points_i0 ve for
    float point_prev[3] = {0}; // uchování předchozího bodu (pro body se společnými normálami, kde je uvedeno pouze UV textury)

    while (!b_eof && gbuff_in.fgets(buff, 1024)) // načítání souboru po řádcích (vpodstatě přečtení 1. řádku a řádků s "o " na začátku)
    {

        if (sscanf(buff, "3dm %u %u", &objectnum, &texturenum) == 2) // načtení počtu objektů a textur
        {
            b_otnum = true; // je už načtený počet objektů a textur?
            p_o.resize(objectnum); // alokace objektů
            centers.resize(objectnum*3); // alokace pole pro středy jednotlivých objektů
            vertnums.resize(objectnum); // alokace pole počtů vertexů v objektech pro zpracování names

            mainflags.resize(objectnum, 0); // alokace pole flagů hlavních objektů

            p_m.clear();
            p_m.resize(texturenum+1);

            for (unsigned int i = 0; i != texturenum; ++i) // načtení názvů textur
            {
                if (!gbuff_in.fgets(buff, 1024)) { b_eof = true; break; }
                // textury se zahazují (dodělat)
                p_m[i+1] = buff;
                for (size_t j = p_m[i+1].size(); j != 0; --j)
                {
                    if (!isSpace(p_m[i+1][j-1]))
                        break;
                    p_m[i+1].resize(j-1);
                }
            }
        }
        else if (b_otnum) // v případě načtení počtu objektů a textur
        {
            if (buff[0] == 'o' && buff[1] == ' ') // když řádek začíná "o "
            {
                p_o[object_i].p_gi = 0; // počáteční nastavení "jména"/čísla skupiny na 0
                if (o_names) // rozdělení podle jmen objektů - dynamický model
                {
                    for (unsigned int i = 0; i != strlen(buff); ++i) // ukončení na znaku nového řádku
                    {
                        if (buff[i] == '\n')
                        {
                            buff[i] = 0;
                            break;
                        }
                    }
                    for (unsigned int i = 0; i != o_names_num; ++i) // zjištění čísla skupiny objektu a priority (hlavní/vedlejší) podle názvu
                    {
                        strcpy(buff2, buff+2); // v buff2 bude jméno skupiny z o_names
                        for (unsigned int j0 = 0; j0 < strlen(buff2); ++j0)
                        {
                        if (isSpace(buff2[j0]))
                            buff2[j0] = 0;
                        }
                        // ošetření "" v o_names
                        if (strlen(o_names[i]) != 0 && strcmp(o_names[i], buff2) == 0) // přesný název
                        {
                            mainflags[object_i] = 1;
                            p_o[object_i].p_gi = i; // nastavení čísla skupiny
                            for (unsigned int j = 0; j != object_i; ++j)
                            {
                                if (p_o[j].p_gi==p_o[object_i].p_gi&&mainflags[j]) // pokud už hlavní objekt ve skupině je, další je vedlejší
                                {
                                    mainflags[object_i] = 0;
                                    break;
                                }
                            }
                        }
                        // ošetření "" v o_names
                        else if (strlen(o_names[i]) != 0 && strlen(buff+2) > strlen(o_names[i])) // pokud je začátek názvu 2. objektu stejný jako jméno, musí být string delší
                        {
                            buff2[strlen(o_names[i])] = 0;
                            if (strcmp(o_names[i], buff2) == 0) // stejný jenom začátek názvu
                                p_o[object_i].p_gi = i;
                        }
                    }
                }
                gbuff_in.fgets(buff, 1024); // načtení středu a čísla textury objektu
                if (sscanf(buff, "%f %f %f %u", &(centers[object_i*3]),
                    &(centers[object_i*3+1]), &(centers[object_i*3+2]),
                    &(p_o[object_i].p_m)) != 4)
                {
                }
                gbuff_in.fgets(buff, 1024);
                if (sscanf(buff, "v %u", &points_n) != 1)
                {
                }
                points_i0_prev = points_i0; // od předchozího počtu začíná for načítající vertexy
                points_i0 += points_n; // počet načtených vertexů

                vertnums[object_i] = points_i0; // zapsání hodnoty počtu do pole počtů vertexů u objektů (postupné přičítání počtu)

                for (unsigned int i = points_i0_prev; i != points_i0; ++i) // načtení vertexů
                {
                    gbuff_in.fgets(buff, 1024);
                    if (*buff == ' ') // řádek začíná mezerou, je na něm jen UV, vertex a normála je společná s předchozím
                    {
                        p_v[i*(size_t)T3dmA::Count+(size_t)T3dmA::Pos0] = point_prev[0];
                        p_v[i*(size_t)T3dmA::Count+(size_t)T3dmA::Pos1] = point_prev[1];
                        p_v[i*(size_t)T3dmA::Count+(size_t)T3dmA::Pos2] = point_prev[2];
                        if (sscanf(buff, " %f %f", &(p_v[i*(size_t)T3dmA::Count+(size_t)T3dmA::Tex0]), &(p_v[i*(size_t)T3dmA::Count+(size_t)T3dmA::Tex1])) != 2)
                        {
                        }
                        nf[i] = 1;
                    } else { // řádek obsahuje vertex a UV
                        if (sscanf(buff, "%f %f %f %f %f",
                                   &(p_v[i*(size_t)T3dmA::Count+(size_t)T3dmA::Pos0]),
                                   &(p_v[i*(size_t)T3dmA::Count+(size_t)T3dmA::Pos1]),
                                   &(p_v[i*(size_t)T3dmA::Count+(size_t)T3dmA::Pos2]),
                                   &(p_v[i*(size_t)T3dmA::Count+(size_t)T3dmA::Tex0]),
                                   &(p_v[i*(size_t)T3dmA::Count+(size_t)T3dmA::Tex1])) != 5)
                        {
                        }
                        point_prev[0] = p_v[i*(size_t)T3dmA::Count+(size_t)T3dmA::Pos0];
                        point_prev[1] = p_v[i*(size_t)T3dmA::Count+(size_t)T3dmA::Pos1];
                        point_prev[2] = p_v[i*(size_t)T3dmA::Count+(size_t)T3dmA::Pos2];
                    }
                    if (!mainflags[object_i]) // posunutí vedlejšího objektu na střed
                    {
                        p_v[i*(size_t)T3dmA::Count+(size_t)T3dmA::Pos0] += centers[object_i*3+0];
                        p_v[i*(size_t)T3dmA::Count+(size_t)T3dmA::Pos1] += centers[object_i*3+1];
                        p_v[i*(size_t)T3dmA::Count+(size_t)T3dmA::Pos2] += centers[object_i*3+2];
                    }
                }
                gbuff_in.fgets(buff, 1024); // načtení počtu pložek
                if (sscanf(buff, "f %u", &faces_n) != 1)
                {
                }
                p_o[object_i].p_i.resize(faces_n*3); // alokace pole indexů
                for (unsigned int i = 0; i != faces_n; ++i) // načtení indexů vertexů plošek
                {
                    gbuff_in.fgets(buff, 1024);
                    unsigned int index0 = 0, index1 = 0, index2 = 0;
                    if (sscanf(buff, "%u %u %u", &index0,
                        &index1, &index2) != 3)
                    {
                    }
                    p_o[object_i].p_i[i*3] = index0;
                    p_o[object_i].p_i[i*3+1] = index1;
                    p_o[object_i].p_i[i*3+2] = index2;

                    p_o[object_i].p_i[i*3  ] += points_i0_prev;
                    p_o[object_i].p_i[i*3+1] += points_i0_prev;
                    p_o[object_i].p_i[i*3+2] += points_i0_prev;
                }
                for (unsigned int i = 0; i != faces_n; ++i)
                {
                    uint16_t* p_i = p_o[object_i].p_i.data();
                    glm::vec3 pos[3];
                    for (int m = 0; m != 3; ++m)
                    {
                        pos[m] = glm::make_vec3(p_v.data()+p_i[i*3+m]*(size_t)T3dmA::Count+(size_t)T3dmA::Pos0);
                    }
                    glm::vec2 tex[3];
                    for (int m = 0; m != 3; ++m)
                    {
                        tex[m] = glm::make_vec2(p_v.data()+p_i[i*3+m]*(size_t)T3dmA::Count+(size_t)T3dmA::Tex0);
                    }
                    const glm::vec3 dir0 = pos[1]-pos[0];
                    const glm::vec3 dir1 = pos[2]-pos[0];
                    const glm::vec3 normal = glm::cross(dir0, dir1); // výpočet normály plošky (nenormalizované, velikost podle obsahu plošky)

                    const glm::vec2 deltaTex0 = tex[1]-tex[0];
                    const glm::vec2 deltaTex1 = tex[2]-tex[0];
                    const float tanBitanR = 1.0f/(deltaTex0.x*deltaTex1.y-deltaTex0.y*deltaTex1.x);
                    const glm::vec3 tangent = (dir0*deltaTex1.y-dir1*deltaTex0.y)*tanBitanR;
                    const glm::vec3 bitangent = (dir1*deltaTex0.x-dir0*deltaTex1.x)*tanBitanR;

                    for (unsigned int c = 0; c != 3; ++c) // zopakování pro každý index vertexu plo?ky
                    {
                        unsigned int j = p_i[i*3+c]; // j <- aktuální index vertexu
                        while (j != 0 && nf[j]) // nalezení prvního indexu v případě společné normály
                            --j;

                        do { // přičtení normály všem společným vrcholům (počet vrcholů = 0...n)
                            p_v[j*(size_t)T3dmA::Count+(size_t)T3dmA::Norm0] += normal.x;
                            p_v[j*(size_t)T3dmA::Count+(size_t)T3dmA::Norm1] += normal.y;
                            p_v[j*(size_t)T3dmA::Count+(size_t)T3dmA::Norm2] += normal.z;

                            p_v[j*(size_t)T3dmA::Count+(size_t)T3dmA::Tan0] += tangent.x;
                            p_v[j*(size_t)T3dmA::Count+(size_t)T3dmA::Tan1] += tangent.y;
                            p_v[j*(size_t)T3dmA::Count+(size_t)T3dmA::Tan2] += tangent.z;

                            p_v[j*(size_t)T3dmA::Count+(size_t)T3dmA::Bitan0] += bitangent.x;
                            p_v[j*(size_t)T3dmA::Count+(size_t)T3dmA::Bitan1] += bitangent.y;
                            p_v[j*(size_t)T3dmA::Count+(size_t)T3dmA::Bitan2] += bitangent.z;

                            ++j;
                        } while (j != vertexnum && nf[j] == 1);
                    }
                }
                ++object_i; // zvětšení hodnoty indexu objektu
            }
        }
    }
    gbuff_in.fclose();
    for (unsigned int i = 0; i != o_names_num;++i) // přiřazení souřadnic středů hlavních objektů poli středů (číslování pole je podle názvů)
    {
        for (unsigned int j = 0; j != objectnum; ++j)
        {
            if (p_o[j].p_gi == i && mainflags[j])
            {
                p_cen[i*3  ] = centers[j*3  ];
                p_cen[i*3+1] = centers[j*3+1];
                p_cen[i*3+2] = centers[j*3+2];
                break;
            }
        }
    }
    unsigned int vertnum_prev = 0;
    for (unsigned int i = 0; i != objectnum; ++i) // transformace vedlejších objektů podle středů hlavních objektů
    {
        if (!mainflags[i])
        {
            for (unsigned int j = vertnum_prev; j != vertnums[i]; ++j)
            {
                p_v[j*(size_t)T3dmA::Count+(size_t)T3dmA::Pos0] -= p_cen[p_o[i].p_gi*3  ];
                p_v[j*(size_t)T3dmA::Count+(size_t)T3dmA::Pos1] -= p_cen[p_o[i].p_gi*3+1];
                p_v[j*(size_t)T3dmA::Count+(size_t)T3dmA::Pos2] -= p_cen[p_o[i].p_gi*3+2];
            }
        }
        vertnum_prev = vertnums[i];
    }
    for (unsigned int i = 0; i != vertexnum; ++i) // normalizace normál
    {
        glm::vec3 normal = glm::normalize(glm::make_vec3(p_v.data()+i*(size_t)T3dmA::Count+(size_t)T3dmA::Norm0));
        glm::assign1(p_v.data()+i*(size_t)T3dmA::Count+(size_t)T3dmA::Norm0, normal);

        glm::vec3 tangent = glm::normalize(glm::make_vec3(p_v.data()+i*(size_t)T3dmA::Count+(size_t)T3dmA::Tan0));
        glm::assign1(p_v.data()+i*(size_t)T3dmA::Count+(size_t)T3dmA::Tan0, tangent);

        glm::vec3 bitangent = glm::normalize(glm::make_vec3(p_v.data()+i*(size_t)T3dmA::Count+(size_t)T3dmA::Bitan0));
        glm::assign1(p_v.data()+i*(size_t)T3dmA::Count+(size_t)T3dmA::Bitan0, bitangent);
    }

    for (O3dm& o : p_o)
    {
        o.initBuf();
    }
}
