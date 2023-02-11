#include "matmng.h"
#include "cstring1.h"
#include "pict2.h"
#include "bits.h"
#include "gbuff_in.h"
#include "load_texture.h"
#include "gamemng.h"

#include <algorithm>
#include <cmath>

void Rendermng::set_oc(const float frustum[6], const T3dm& t3dm)
{
    p_octocube.p_r = 0.f;
    float bbox_x[2] = {0.f, 0.f};
    float bbox_y[2] = {0.f, 0.f};
    float bbox_z[2] = {0.f, 0.f};
    bool bfirst = true;
    for (unsigned int i = 0; i != t3dm.p_o.size(); ++i)
    {
        for (unsigned int j = 0; j != t3dm.p_o[i].p_i.size(); ++j)
        {
            if (t3dm.p_o[i].p_gi != 1)
            {
                if (bfirst)
                {
                    bbox_x[0] = bbox_x[1] = t3dm.p_v[t3dm.p_o[i].p_i[j]*(size_t)T3dmA::Count+(size_t)T3dmA::Pos0]+t3dm.p_cen[t3dm.p_o[i].p_gi*3+0];
                    bbox_y[0] = bbox_y[1] = t3dm.p_v[t3dm.p_o[i].p_i[j]*(size_t)T3dmA::Count+(size_t)T3dmA::Pos1]+t3dm.p_cen[t3dm.p_o[i].p_gi*3+1];
                    bbox_z[0] = bbox_z[1] = t3dm.p_v[t3dm.p_o[i].p_i[j]*(size_t)T3dmA::Count+(size_t)T3dmA::Pos2]+t3dm.p_cen[t3dm.p_o[i].p_gi*3+2];
                    bfirst = false;
                } else {
                    bbox_x[0] = std::min(bbox_x[0], t3dm.p_v[t3dm.p_o[i].p_i[j]*(size_t)T3dmA::Count+(size_t)T3dmA::Pos0]+t3dm.p_cen[t3dm.p_o[i].p_gi*3+0]);
                    bbox_x[1] = std::max(bbox_x[1], t3dm.p_v[t3dm.p_o[i].p_i[j]*(size_t)T3dmA::Count+(size_t)T3dmA::Pos0]+t3dm.p_cen[t3dm.p_o[i].p_gi*3+0]);
                    bbox_y[0] = std::min(bbox_y[0], t3dm.p_v[t3dm.p_o[i].p_i[j]*(size_t)T3dmA::Count+(size_t)T3dmA::Pos1]+t3dm.p_cen[t3dm.p_o[i].p_gi*3+1]);
                    bbox_y[1] = std::max(bbox_y[1], t3dm.p_v[t3dm.p_o[i].p_i[j]*(size_t)T3dmA::Count+(size_t)T3dmA::Pos1]+t3dm.p_cen[t3dm.p_o[i].p_gi*3+1]);
                    bbox_z[0] = std::min(bbox_z[0], t3dm.p_v[t3dm.p_o[i].p_i[j]*(size_t)T3dmA::Count+(size_t)T3dmA::Pos2]+t3dm.p_cen[t3dm.p_o[i].p_gi*3+2]);
                    bbox_z[1] = std::max(bbox_z[1], t3dm.p_v[t3dm.p_o[i].p_i[j]*(size_t)T3dmA::Count+(size_t)T3dmA::Pos2]+t3dm.p_cen[t3dm.p_o[i].p_gi*3+2]);
                }
            }
        }
    }
    if (bfirst)
    {
        return;
    }
    p_boctocube = true;

    p_octocube.p_cen[0] = (bbox_x[0]+bbox_x[1])*0.5f;
    p_octocube.p_cen[1] = (bbox_y[0]+bbox_y[1])*0.5f;
    p_octocube.p_cen[2] = (bbox_z[0]+bbox_z[1])*0.5f;
    float xdim = bbox_x[1] - bbox_x[0];
    float ydim = bbox_y[1] - bbox_y[0];
    float zdim = bbox_z[1] - bbox_z[0];
    p_octocube.p_r = sqrtf(xdim*xdim + ydim*ydim + zdim*zdim)*0.5f;
    p_octocube.p_base = &p_octocube_base;
    p_octocube_base.init_frustum(frustum);
}

void Rendermng::init(Gamemng* gamemng, const T3dm* t3dm, const Matmng* matmng, Octopus* octopus)
{
    p_t3dm = t3dm;
    p_matmng = matmng;
    p_octopus = octopus;
    p_gamemng = gamemng;
}

void Rendermng::render_o_pass1(const float* modelview_matrix)
{
    if (p_octopus)
        p_octopus->render_pass1(modelview_matrix);
    else if (p_boctocube)
        b_visible = p_octocube.test(modelview_matrix);
}

void Rendermng::render_o_pass1_lim(const float* modelview_matrix, unsigned int face_limit)
{
    if (p_octopus)
        p_octopus->render_pass1_lim(modelview_matrix, face_limit);
    else if (p_boctocube)
        b_visible = p_octocube.test(modelview_matrix);
}

void Rendermng::render_o_pass_s3()
{
    if (!isVisible())
        return;

    p_gamemng->p_shadermng.use(ShaderId::ColorTex);
    p_gamemng->p_shadermng.set(ShaderUniInt::AlphaDiscard, (GLint)0);

    glEnableVertexAttribArray((GLuint)ShaderAttrib::Pos); checkGL();
    glEnableVertexAttribArray((GLuint)ShaderAttrib::Tex); checkGL();

    glVertexAttribPointer((GLuint)ShaderAttrib::Pos, 3, GL_FLOAT, GL_FALSE, sizeof(float)*(size_t)T3dmA::Count, p_t3dm->p_v.data()+(size_t)T3dmA::Pos0);
    glVertexAttribPointer((GLuint)ShaderAttrib::Tex, 2, GL_FLOAT, GL_FALSE, sizeof(float)*(size_t)T3dmA::Count, p_t3dm->p_v.data()+(size_t)T3dmA::Tex0);

    unsigned int l = 0;
    for (unsigned int i = 0; i != p_matmng->p_mat.size(); ++i)
    {
        const Mat& material = p_matmng->p_mat[i];

        if (material.texture)
        {
            glBindTexture(GL_TEXTURE_2D, material.texture); checkGL();
        }
        else
        {
            glBindTexture(GL_TEXTURE_2D, p_gamemng->p_whitetex); checkGL();
        }

        if (material.special == 3)
        {
            glEnable(GL_POLYGON_OFFSET_FILL);

            checkGL();
            glPolygonOffset(-2.f, -2.f); checkGL();

            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); checkGL();
            glEnable(GL_BLEND); checkGL();
            glDepthMask(GL_FALSE); checkGL();
            glVertexAttrib4f((GLuint)ShaderAttrib::Color, 0, 0, 0, 0.7f); checkGL();

            while (l != p_t3dm->p_o.size() && p_t3dm->p_o[l].p_m == i)
            {
                // sem přidat transformace
                if (p_t3dm->p_o[l].p_gi != 1)
                {
                    glDrawElements(GL_TRIANGLES, p_t3dm->p_o[l].p_i.size(), GL_UNSIGNED_SHORT, p_t3dm->p_o[l].p_i.data()); checkGL();
                }
                ++l;
            }

            glDisable(GL_BLEND); checkGL();
            glDisable(GL_POLYGON_OFFSET_FILL); checkGL();
            glDepthMask(GL_TRUE); checkGL();
        } else {
            while (l != p_t3dm->p_o.size() && p_t3dm->p_o[l].p_m == i)
                ++l;
        }

    }
    glDisableVertexAttribArray((GLuint)ShaderAttrib::Pos); checkGL();
    glDisableVertexAttribArray((GLuint)ShaderAttrib::Tex); checkGL();
}

void Rendermng::render_o_pass2(const glm::mat4& m)
{
    if (!isVisible())
        return;
    glEnableVertexAttribArray((GLuint)ShaderAttrib::Pos);
    glEnableVertexAttribArray((GLuint)ShaderAttrib::Normal);
    glEnableVertexAttribArray((GLuint)ShaderAttrib::Tex);
    glVertexAttribPointer((GLuint)ShaderAttrib::Pos, 3, GL_FLOAT, GL_FALSE, sizeof(float)*(size_t)T3dmA::Count, p_t3dm->p_v.data()+(size_t)T3dmA::Pos0);
    glVertexAttribPointer((GLuint)ShaderAttrib::Normal, 3, GL_FLOAT, GL_FALSE, sizeof(float)*(size_t)T3dmA::Count, p_t3dm->p_v.data()+(size_t)T3dmA::Norm0);
    glVertexAttribPointer((GLuint)ShaderAttrib::Tex, 2, GL_FLOAT, GL_FALSE, sizeof(float)*(size_t)T3dmA::Count, p_t3dm->p_v.data()+(size_t)T3dmA::Tex0);
    unsigned int k = 0;
    for (unsigned int i = 0; i != p_matmng->p_mat.size(); ++i)
    {
        const Mat& material = p_matmng->p_mat[i];
        if (material.texture)
        {
            glBindTexture(GL_TEXTURE_2D, material.texture); checkGL();
        }
        else
        {
            glBindTexture(GL_TEXTURE_2D, p_gamemng->p_whitetex); checkGL();
        }

        //if (material.balpha_test) setStandardAlphaTest(true);
        if (!material.blighting)
        {
            glVertexAttrib4fv((GLuint)ShaderAttrib::Color, material.color);
            p_gamemng->p_shadermng.use(ShaderId::ColorTex);
            p_gamemng->p_shadermng.set(ShaderUniInt::AlphaDiscard, (GLint)material.balpha_test);
        }
        else
        {
            if (material.benv_map)
            {
                p_gamemng->p_shadermng.use(ShaderId::Car);
            }
            else
            {
                p_gamemng->p_shadermng.use(ShaderId::LightTex);
                p_gamemng->p_shadermng.set(ShaderUniInt::AlphaDiscard, (GLint)material.balpha_test);
            }
        }
        if (material.bboth_side)
        {
            glDisable(GL_CULL_FACE); checkGL();
        }

        // TADY JE TO GRÓ
        if (p_octopus != NULL)
        {
            if (material.special == 0)
            {
                for (unsigned int j = 0; j != p_octopus->p_vw_sz; ++j)
                {
                    glDrawElements(GL_TRIANGLES, p_octopus->p_vw[j]->p_mi[i].p_sz, GL_UNSIGNED_SHORT, p_octopus->p_vw[j]->p_mi[i].p_i.data()); checkGL();
                }
            }
        } else { // vykreslování objektu bez octopusu
            if (material.special == 0)
            {
                while (k != p_t3dm->p_o.size() && p_t3dm->p_o[k].p_m == i)
                {
                    // sem přidat transformace
                    if (p_t3dm->p_o[k].p_gi != 1)
                    {
                        if (p_transf)
                        {
                            glm::mat4 m2 = m * p_transf->mult_mwmx(p_t3dm->p_o[k].p_gi);
                            p_gamemng->p_shadermng.set(ShaderUniMat4::ModelViewMat, m2);
                        }
                        glDrawElements(GL_TRIANGLES, p_t3dm->p_o[k].p_i.size(), GL_UNSIGNED_SHORT, p_t3dm->p_o[k].p_i.data()); checkGL();
                        if (p_transf)
                        {
                            p_gamemng->p_shadermng.set(ShaderUniMat4::ModelViewMat, m);
                        }
                    }
                    ++k;
                }
            } else if (material.special == 1) {

                p_gamemng->p_shadermng.use(ShaderId::CarTop);

                unsigned int l = k;
                glDisable(GL_CULL_FACE);
                while (l != p_t3dm->p_o.size() && p_t3dm->p_o[l].p_m == i)
                {
                    glDrawElements(GL_TRIANGLES, p_t3dm->p_o[l].p_i.size(), GL_UNSIGNED_SHORT, p_t3dm->p_o[l].p_i.data()); checkGL();
                    ++l;
                }
                glEnable(GL_CULL_FACE);
                k = l;
            } else { // přeskočení jiných typů materiálů než 0 a 1
                unsigned int l = k;
                while (l != p_t3dm->p_o.size() && p_t3dm->p_o[l].p_m == i)
                {
                    ++l;
                }
                k = l;
            }
        }
        if (material.bboth_side)
        {
            glEnable(GL_CULL_FACE); checkGL();
        }
    }
    glDisableVertexAttribArray((GLuint)ShaderAttrib::Pos);
    glDisableVertexAttribArray((GLuint)ShaderAttrib::Normal);
    glDisableVertexAttribArray((GLuint)ShaderAttrib::Tex);
}

void Rendermng::render_o_pass_glassTint(const glm::mat4& m)
{
    if (!isVisible())
        return;
    p_gamemng->p_shadermng.set(ShaderUniMat4::ModelViewMat, m);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnableVertexAttribArray((GLuint)ShaderAttrib::Pos);
    glEnableVertexAttribArray((GLuint)ShaderAttrib::Normal);
    glEnableVertexAttribArray((GLuint)ShaderAttrib::Tex);
    glVertexAttribPointer((GLuint)ShaderAttrib::Pos, 3, GL_FLOAT, GL_FALSE, sizeof(float)*(size_t)T3dmA::Count, p_t3dm->p_v.data()+(size_t)T3dmA::Pos0);
    glVertexAttribPointer((GLuint)ShaderAttrib::Normal, 3, GL_FLOAT, GL_FALSE, sizeof(float)*(size_t)T3dmA::Count, p_t3dm->p_v.data()+(size_t)T3dmA::Norm0);
    glVertexAttribPointer((GLuint)ShaderAttrib::Tex, 2, GL_FLOAT, GL_FALSE, sizeof(float)*(size_t)T3dmA::Count, p_t3dm->p_v.data()+(size_t)T3dmA::Tex0);
    unsigned int k = 0;
    for (unsigned int i = 0; i != p_matmng->p_mat.size(); ++i)
    {
        const Mat& material = p_matmng->p_mat[i];
        if (material.texture)
        {
            glBindTexture(GL_TEXTURE_2D, material.texture); checkGL();
        }
        else
        {
            glBindTexture(GL_TEXTURE_2D, p_gamemng->p_whitetex); checkGL();
        }
        if (p_octopus == NULL)
        {
            if (material.special == 1) {
                p_gamemng->p_shadermng.use(ShaderId::GlassTint);
                unsigned int l = k;
                while (l != p_t3dm->p_o.size() && p_t3dm->p_o[l].p_m == i)
                {
                    glDrawElements(GL_TRIANGLES, p_t3dm->p_o[l].p_i.size(), GL_UNSIGNED_SHORT, p_t3dm->p_o[l].p_i.data()); checkGL();
                    ++l;
                }
                k = l;
            } else { // přeskočení jiných typů materiálů než 0 a 1
                unsigned int l = k;
                while (l != p_t3dm->p_o.size() && p_t3dm->p_o[l].p_m == i)
                {
                    ++l;
                }
                k = l;
            }
        }
    }
    glDisableVertexAttribArray((GLuint)ShaderAttrib::Pos);
    glDisableVertexAttribArray((GLuint)ShaderAttrib::Normal);
    glDisableVertexAttribArray((GLuint)ShaderAttrib::Tex);
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
}

void Rendermng::render_o_pass_glassReflection(const glm::mat4& m)
{
    if (!isVisible())
        return;
    p_gamemng->p_shadermng.set(ShaderUniMat4::ModelViewMat, m);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glEnableVertexAttribArray((GLuint)ShaderAttrib::Pos);
    glEnableVertexAttribArray((GLuint)ShaderAttrib::Normal);
    glEnableVertexAttribArray((GLuint)ShaderAttrib::Tex);
    glVertexAttribPointer((GLuint)ShaderAttrib::Pos, 3, GL_FLOAT, GL_FALSE, sizeof(float)*(size_t)T3dmA::Count, p_t3dm->p_v.data()+(size_t)T3dmA::Pos0);
    glVertexAttribPointer((GLuint)ShaderAttrib::Normal, 3, GL_FLOAT, GL_FALSE, sizeof(float)*(size_t)T3dmA::Count, p_t3dm->p_v.data()+(size_t)T3dmA::Norm0);
    glVertexAttribPointer((GLuint)ShaderAttrib::Tex, 2, GL_FLOAT, GL_FALSE, sizeof(float)*(size_t)T3dmA::Count, p_t3dm->p_v.data()+(size_t)T3dmA::Tex0);
    unsigned int k = 0;
    for (unsigned int i = 0; i != p_matmng->p_mat.size(); ++i)
    {
        const Mat& material = p_matmng->p_mat[i];
        if (material.texture)
        {
            glBindTexture(GL_TEXTURE_2D, material.texture); checkGL();
        }
        else
        {
            glBindTexture(GL_TEXTURE_2D, p_gamemng->p_whitetex); checkGL();
        }
        if (p_octopus == NULL)
        {
            if (material.special == 1) {
                p_gamemng->p_shadermng.use(ShaderId::GlassReflection);
                unsigned int l = k;
                while (l != p_t3dm->p_o.size() && p_t3dm->p_o[l].p_m == i)
                {
                    glDrawElements(GL_TRIANGLES, p_t3dm->p_o[l].p_i.size(), GL_UNSIGNED_SHORT, p_t3dm->p_o[l].p_i.data()); checkGL();
                    ++l;
                }
                k = l;
            } else { // přeskočení jiných typů materiálů než 0 a 1
                unsigned int l = k;
                while (l != p_t3dm->p_o.size() && p_t3dm->p_o[l].p_m == i)
                {
                    ++l;
                }
                k = l;
            }
        }
    }
    glDisableVertexAttribArray((GLuint)ShaderAttrib::Pos);
    glDisableVertexAttribArray((GLuint)ShaderAttrib::Normal);
    glDisableVertexAttribArray((GLuint)ShaderAttrib::Tex);
    glDisable(GL_BLEND);
}

void Rendermng::render_o_pass_s2()
{
    glEnable(GL_BLEND); checkGL();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); checkGL();

    if (p_boctocube && !b_visible)
        return;
    glEnableVertexAttribArray((GLuint)ShaderAttrib::Pos);
    glEnableVertexAttribArray((GLuint)ShaderAttrib::Normal);
    glEnableVertexAttribArray((GLuint)ShaderAttrib::Tex);
    glVertexAttribPointer((GLuint)ShaderAttrib::Pos, 3, GL_FLOAT, GL_FALSE, sizeof(float)*(size_t)T3dmA::Count,p_t3dm->p_v.data()+(size_t)T3dmA::Pos0);
    glVertexAttribPointer((GLuint)ShaderAttrib::Normal, 3, GL_FLOAT, GL_FALSE, sizeof(float)*(size_t)T3dmA::Count, p_t3dm->p_v.data()+(size_t)T3dmA::Norm0);
    glVertexAttribPointer((GLuint)ShaderAttrib::Tex, 2, GL_FLOAT, GL_FALSE, sizeof(float)*(size_t)T3dmA::Count,p_t3dm->p_v.data()+(size_t)T3dmA::Tex0);
    for (unsigned int i = 0; i != p_matmng->p_mat.size(); ++i)
    {
        const Mat& material = p_matmng->p_mat[i];
        if (material.texture)
        {
            glBindTexture(GL_TEXTURE_2D, material.texture); checkGL();
        }
        else
        {
            glBindTexture(GL_TEXTURE_2D, p_gamemng->p_whitetex); checkGL();
        }
        if (!material.blighting)
        {
            p_gamemng->p_shadermng.use(ShaderId::ColorTex);
            p_gamemng->p_shadermng.set(ShaderUniInt::AlphaDiscard, (GLint)0);
            glVertexAttrib4fv((GLuint)ShaderAttrib::Color, material.color); checkGL();
        }
        else
        {
            p_gamemng->p_shadermng.set(ShaderUniInt::AlphaDiscard, (GLint)0);
            p_gamemng->p_shadermng.use(ShaderId::LightTex);
        }

        // TADY JE TO GRÓ
        if (p_octopus != NULL)
        {
            if (material.special == 2)
            {
                for (unsigned int j = 0; j != p_octopus->p_vw_sz; ++j)
                {
                    glDrawElements(GL_TRIANGLES, p_octopus->p_vw[j]->p_mi[i].p_sz, GL_UNSIGNED_SHORT, p_octopus->p_vw[j]->p_mi[i].p_i.data()); checkGL();
                }
            }
        }
    }
    glDisable(GL_BLEND); checkGL();
    glDisableVertexAttribArray((GLuint)ShaderAttrib::Pos);
    glDisableVertexAttribArray((GLuint)ShaderAttrib::Normal);
    glDisableVertexAttribArray((GLuint)ShaderAttrib::Tex);
}

void Mat::default_mat()
{
    color[0] = color[1] = color[2] = color[3] = 1.f;
    blighting = true;
}

void Mat::load(const char* fname)
{
    default_mat();
    if (*fname == 0)
    {
        return;
    }

    if (!gbuff_in.f_open(fname, "r"))
        return;
    char buff[1024];
    if (gbuff_in.fgets(buff, 1024))
    {
        uncomment(buff);
        blighting = strstrtok1(buff, "lighting");
        bmipmap = strstrtok1(buff, "mipmap");
        balpha_test = strstrtok1(buff, "alpha_test");
        bboth_side = strstrtok1(buff, "both_side");
        benv_map = strstrtok1(buff, "env_map");
    } else {
        gbuff_in.fclose();
        return;
    }
    if (gbuff_in.fgets(buff, 1024))
    {
        uncomment(buff);
        char delim[128] = {0};
        int j = 0;
        for (int i = 0; i != 128; ++i)
        {
            if (isSpace(i))
            {
                delim[j] = i;
                ++j;
            }
        }
        delim[j] = 0;
        char* tok = 0;
        tok = strtok(buff, delim);
        if (tok)
            strncpy(texd_name, tok, 255);
        tok = strtok(NULL, delim);
        if (tok)
            strncpy(texa_name, tok, 255);
        if (strlen(texd_name) == 1 && *texd_name == '0')
            *texd_name = 0;
        if (strlen(texa_name) == 1 && *texa_name == '0')
            *texa_name = 0;
    } else {
        gbuff_in.fclose();
        return;
    }
    if (gbuff_in.fgets(buff, 1024))
    {
        uncomment(buff);
        if (sscanf(buff, "%f %f %f %f", color, color+1, color+2, color+3) != 4)
        {
            color[0] = color[1] = color[2] = color[3] = 1.f;
        }
    } else {
        gbuff_in.fclose();
        return;
    }
    if (gbuff_in.fgets(buff, 1024))
    {
        uncomment(buff);
        if (sscanf(buff, "%d", &special) != 1)
        {
            special = 0;
        }
    } else {
        gbuff_in.fclose();
        return;
    }
    gbuff_in.fclose();
}

inline float maxfloat(float f1, float f2)
{
    return (f1 > f2) ? f1 : f2;
}

inline float minfloat(float f1, float f2)
{
    return (f1 < f2) ? f1 : f2;
}

void Matmng::load(const T3dm* t3dm)
{
    p_t3dm = t3dm;
    //p_m_sz = p_t3dm->p_m.size();
    p_mat.resize(p_t3dm->p_m.size());
    char buff[256] = {0};
    for (unsigned int i = 0; i != p_mat.size(); ++i)
    {
        if (!p_t3dm->p_m[i].empty())
        {
            strncat1(buff, p_t3dm->p_m[i].c_str(), ".3mt", 256);
            p_mat[i].load(buff);
        } else {
            p_mat[i].load("");
        }
    }
    // načíst textury
    for (unsigned int i = 0; i != p_mat.size(); ++i)
    {
        bool bsame = false;
        for (unsigned int j = 0; j != i; ++j)
        {
            if (strcmp(p_mat[i].texd_name, p_mat[j].texd_name) == 0 &&
                strcmp(p_mat[i].texa_name, p_mat[j].texa_name) == 0 &&
                p_mat[i].bmipmap == p_mat[j].bmipmap)
            {
                p_mat[i].texture = p_mat[j].texture;
                bsame = true;
                break;
            }
        }

        if (*(p_mat[i].texd_name) && !bsame)
        {
            if (*(p_mat[i].texa_name)) // 32 bit
            {
                Pict2 pict;
                if (strSuff(p_mat[i].texd_name, ".jpg"))
                {
                    gbuff_in.f_open(p_mat[i].texd_name, "rb");
                    pict.loadjpeg(gbuff_in.fbuffptr(), gbuff_in.fbuffsz());
                    gbuff_in.fclose();
                }
                else
                {
                    gbuff_in.f_open(p_mat[i].texd_name, "rb");
                    pict.loadpng(gbuff_in.fbuffptr(), gbuff_in.fbuffsz());
                    gbuff_in.fclose();
                }
                Pict2 picta;
                bool forceMipmap = false;
                if (strSuff(p_mat[i].texa_name, ".jpg"))
                {
                    gbuff_in.f_open(p_mat[i].texa_name, "rb");
                    picta.loadjpeg(gbuff_in.fbuffptr(), gbuff_in.fbuffsz());
                    gbuff_in.fclose();
                }
                else
                {
                    gbuff_in.f_open(p_mat[i].texa_name, "rb");
                    picta.loadpng(gbuff_in.fbuffptr(), gbuff_in.fbuffsz());
                    gbuff_in.fclose();
                    if (g_multisampleMode && strcmp(p_mat[i].texa_name, "stromy0a.png") == 0)
                    {
                        forceMipmap = true;
                        for (int i = 0; i != picta.w() * picta.h() * 4; ++i)
                        {
                            float oldValue = picta.px()[i] / 255.f;
                            float newValue = ((oldValue - 0.5) * 1.4 + 0.5) * 255.f;
                            if (newValue > 255.f) newValue = 255.f;
                            else if (newValue < 0.f) newValue = 0.f;
                            picta.px()[i] = newValue;
                        }
                    }
                }
                pict.r2a(picta);
                p_mat[i].texture = load_texture(pict, p_mat[i].bmipmap || forceMipmap);
            } else { // 24 bit
                Pict2 pict;
                if (strSuff(p_mat[i].texd_name, ".jpg"))
                {
                    gbuff_in.f_open(p_mat[i].texd_name, "rb");
                    pict.loadjpeg(gbuff_in.fbuffptr(), gbuff_in.fbuffsz());
                    gbuff_in.fclose();
                }
                else
                {
                    gbuff_in.f_open(p_mat[i].texd_name, "rb");
                    pict.loadpng(gbuff_in.fbuffptr(), gbuff_in.fbuffsz());
                    gbuff_in.fclose();
                }
                p_mat[i].texture = load_texture(pict, p_mat[i].bmipmap);
            }
        }
        else if (*(p_mat[i].texa_name) && !bsame) // 8-bit alpha
        {
            Pict2 picta;
            if (strSuff(p_mat[i].texa_name, ".jpg"))
            {
                gbuff_in.f_open(p_mat[i].texa_name, "rb");
                picta.loadjpeg(gbuff_in.fbuffptr(), gbuff_in.fbuffsz());
                gbuff_in.fclose();
            }
            else
            {
                gbuff_in.f_open(p_mat[i].texa_name, "rb");
                picta.loadpng(gbuff_in.fbuffptr(), gbuff_in.fbuffsz());
                gbuff_in.fclose();
            }
            picta.r2a();
            p_mat[i].texture = load_texture(picta, p_mat[i].bmipmap);
        }
    }
}
