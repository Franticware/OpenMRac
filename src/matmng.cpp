#include "matmng.h"

#include "cstring1.h"
#include "pict2.h"
#include "glhelpers1.h"
#include "bits.h"
#include "gbuff_in.h"
#include "load_texture.h"

#include <algorithm>
#include <cmath>

int g_hq_textures = 0;

void Rendermng::set_oc(const float frustum[6], const T3dm& t3dm)
{
    p_octocube.p_r = 0.f;
    float bbox_x[2] = {0.f, 0.f};
    float bbox_y[2] = {0.f, 0.f};
    float bbox_z[2] = {0.f, 0.f};
    bool bfirst = true;
    for (unsigned int i = 0; i != t3dm.p_sz; ++i)
    {
        for (unsigned int j = 0; j != t3dm.p_o[i].p_sz; ++j)
        {
            if (t3dm.p_o[i].p_gi != 1)
            {
                if (bfirst)
                {
                    bbox_x[0] = bbox_x[1] = t3dm.p_v[t3dm.p_o[i].p_i[j]*3+0]+t3dm.p_cen[t3dm.p_o[i].p_gi*3+0];
                    bbox_y[0] = bbox_y[1] = t3dm.p_v[t3dm.p_o[i].p_i[j]*3+1]+t3dm.p_cen[t3dm.p_o[i].p_gi*3+1];
                    bbox_z[0] = bbox_z[1] = t3dm.p_v[t3dm.p_o[i].p_i[j]*3+2]+t3dm.p_cen[t3dm.p_o[i].p_gi*3+2];
                    bfirst = false;
                } else {
                    bbox_x[0] = std::min(bbox_x[0], t3dm.p_v[t3dm.p_o[i].p_i[j]*3+0]+t3dm.p_cen[t3dm.p_o[i].p_gi*3+0]);
                    bbox_x[1] = std::max(bbox_x[1], t3dm.p_v[t3dm.p_o[i].p_i[j]*3+0]+t3dm.p_cen[t3dm.p_o[i].p_gi*3+0]);
                    bbox_y[0] = std::min(bbox_y[0], t3dm.p_v[t3dm.p_o[i].p_i[j]*3+1]+t3dm.p_cen[t3dm.p_o[i].p_gi*3+1]);
                    bbox_y[1] = std::max(bbox_y[1], t3dm.p_v[t3dm.p_o[i].p_i[j]*3+1]+t3dm.p_cen[t3dm.p_o[i].p_gi*3+1]);
                    bbox_z[0] = std::min(bbox_z[0], t3dm.p_v[t3dm.p_o[i].p_i[j]*3+2]+t3dm.p_cen[t3dm.p_o[i].p_gi*3+2]);
                    bbox_z[1] = std::max(bbox_z[1], t3dm.p_v[t3dm.p_o[i].p_i[j]*3+2]+t3dm.p_cen[t3dm.p_o[i].p_gi*3+2]);
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

void Rendermng::init(const T3dm* t3dm, const Matmng* matmng, Octopus* octopus)
{
    p_t3dm = t3dm;
    p_matmng = matmng;
    p_octopus = octopus;
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

void Rendermng::render_o_pass3()
{
    if (!isVisible())
        return;
    glEnableClientState(GL_VERTEX_ARRAY); checkGL();
    glEnableClientState(GL_NORMAL_ARRAY); checkGL();
    glVertexPointer(3,GL_FLOAT,0,p_t3dm->p_v); checkGL();
    glNormalPointer(GL_FLOAT, 0, p_t3dm->p_n); checkGL();
    unsigned int k = 0;
    for (unsigned int i = 0; i != p_matmng->p_m_sz; ++i)
    {
        Mat& material = p_matmng->p_mat[i];

        if (material.special == 1)
        {
            glDisable(GL_LIGHTING); checkGL();
            glDisable(GL_TEXTURE_2D); checkGL();
            setStandardAlphaTest(false);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); checkGL();
            glEnable(GL_BLEND); checkGL();
            glColor4f(0, 0, 0, 0.6); checkGL();

            unsigned int l = k;

            while (l != p_t3dm->p_sz && p_t3dm->p_o[l].p_m == i)
            {
                // sem přidat transformace
                if (p_t3dm->p_o[l].p_gi != 1)
                {
                    glDrawElements(GL_TRIANGLES, p_t3dm->p_o[l].p_sz, GL_UNSIGNED_SHORT, p_t3dm->p_o[l].p_i); checkGL(); afterDrawcall();
                }
                ++l;
            }
            glDisable(GL_BLEND); checkGL();
            glEnable(GL_TEXTURE_2D); checkGL();
            glEnable(GL_LIGHTING); checkGL();
        }

        if (material.benv_map)
        {
/*
#if USE_CUBEMAP
            glColor3f(g_envmap_shininess, g_envmap_shininess, g_envmap_shininess); checkGL();
            glEnable(GL_BLEND); checkGL();
            glBlendFunc(GL_ONE, GL_ONE); checkGL();
            glDisable(GL_LIGHTING); checkGL();
            glDisable(GL_TEXTURE_2D); checkGL();
            glEnable(GL_TEXTURE_CUBE_MAP_ARB); checkGL();
            glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, p_skycmtex); checkGL();
            glDepthFunc(GL_EQUAL); checkGL();
            glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_ARB); checkGL();
            glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_ARB); checkGL();
            glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_ARB); checkGL();
            glEnable(GL_TEXTURE_GEN_S); checkGL();
            glEnable(GL_TEXTURE_GEN_T); checkGL();
            glEnable(GL_TEXTURE_GEN_R); checkGL();
            while (k != p_t3dm->p_sz && p_t3dm->p_o[k].p_m == i)
            {
                // sem přidat transformace
                if (p_t3dm->p_o[k].p_gi != 1)
                {
                    glDrawElements(GL_TRIANGLES, p_t3dm->p_o[k].p_sz, GL_UNSIGNED_SHORT, p_t3dm->p_o[k].p_i); checkGL();
                }
                ++k;
            }
            glDepthFunc(GL_LESS); checkGL();
            glEnable(GL_TEXTURE_2D); checkGL();
            glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, 0); checkGL();
            glDisable(GL_TEXTURE_CUBE_MAP_ARB); checkGL();
            glEnable(GL_LIGHTING); checkGL();
            glDisable(GL_BLEND); checkGL();
            glDisable(GL_TEXTURE_GEN_S); checkGL();
            glDisable(GL_TEXTURE_GEN_T); checkGL();
            glDisable(GL_TEXTURE_GEN_R); checkGL();
#else
            glColor3f(g_envmap_shininess, g_envmap_shininess, g_envmap_shininess); checkGL();
            glEnable(GL_BLEND); checkGL();
            glBlendFunc(GL_ONE, GL_ONE); checkGL();
            glDisable(GL_LIGHTING); checkGL();
            glEnable(GL_TEXTURE_2D); checkGL();
            glBindTexture(GL_TEXTURE_2D, p_skycmtex); checkGL();
            glDepthFunc(GL_EQUAL); checkGL();
            glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
            glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
            glEnable(GL_TEXTURE_GEN_S); checkGL();
            glEnable(GL_TEXTURE_GEN_T); checkGL();
            while (k != p_t3dm->p_sz && p_t3dm->p_o[k].p_m == i)
            {
                // sem přidat transformace
                if (p_t3dm->p_o[k].p_gi != 1)
                {
                    glDrawElements(GL_TRIANGLES, p_t3dm->p_o[k].p_sz, GL_UNSIGNED_SHORT, p_t3dm->p_o[k].p_i); checkGL();
                }
                ++k;
            }
            glDepthFunc(GL_LESS); checkGL();
            glBindTexture(GL_TEXTURE_2D, 0); checkGL();
            glEnable(GL_LIGHTING); checkGL();
            glDisable(GL_BLEND); checkGL();
            glDisable(GL_TEXTURE_GEN_S); checkGL();
            glDisable(GL_TEXTURE_GEN_T); checkGL();
#endif
*/
        }
        else
        {
            while (k != p_t3dm->p_sz && p_t3dm->p_o[k].p_m == i)
                ++k;
        }
    }
    glDisableClientState(GL_VERTEX_ARRAY); checkGL();
    glDisableClientState(GL_NORMAL_ARRAY); checkGL();
}

void Rendermng::render_o_pass_s3()
{
    if (!isVisible())
        return;
    glEnableClientState(GL_VERTEX_ARRAY); checkGL();
    glVertexPointer(3,GL_FLOAT,0,p_t3dm->p_v); checkGL();
    glEnableClientState(GL_TEXTURE_COORD_ARRAY); checkGL();
    glTexCoordPointer(2,GL_FLOAT,0,p_t3dm->p_t); checkGL();
    unsigned int l = 0;
    for (unsigned int i = 0; i != p_matmng->p_m_sz; ++i)
    {
        Mat& material = p_matmng->p_mat[i];

        glBindTexture(GL_TEXTURE_2D, material.texture); checkGL();

        if (material.special == 3)
        {
            glEnable(GL_POLYGON_OFFSET_EXT);
            checkGL();
            glPolygonOffset(-2.f, -2.f); checkGL();

            glDisable(GL_LIGHTING); checkGL();
            setStandardAlphaTest(false);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); checkGL();
            glEnable(GL_BLEND); checkGL();
            glDepthMask(GL_FALSE); checkGL();
            glColor4f(0, 0, 0, 0.7f); checkGL();

            while (l != p_t3dm->p_sz && p_t3dm->p_o[l].p_m == i)
            {
                // sem přidat transformace
                if (p_t3dm->p_o[l].p_gi != 1)
                {
                    glDrawElements(GL_TRIANGLES, p_t3dm->p_o[l].p_sz, GL_UNSIGNED_SHORT, p_t3dm->p_o[l].p_i); checkGL(); afterDrawcall();
                }
                ++l;
            }

            glDisable(GL_BLEND); checkGL();
            glDisable(GL_POLYGON_OFFSET_EXT); checkGL();
            glEnable(GL_LIGHTING); checkGL();
            glDepthMask(GL_TRUE); checkGL();
        } else {
            while (l != p_t3dm->p_sz && p_t3dm->p_o[l].p_m == i)
                ++l;
        }

    }
    glDisableClientState(GL_VERTEX_ARRAY); checkGL();
    glDisableClientState(GL_TEXTURE_COORD_ARRAY); checkGL();
}

void Rendermng::render_o_pass2()
{
    if (!isVisible())
        return;
    glEnableClientState(GL_VERTEX_ARRAY); checkGL();
    if (p_matmng->p_bstatic_light)
    {
        glDisable(GL_LIGHTING); checkGL();
        glEnableClientState(GL_COLOR_ARRAY); checkGL();
        glColorPointer(4, GL_FLOAT, 0, p_matmng->p_vcolor); checkGL();
    } else {
        glEnable(GL_LIGHTING); checkGL();
        glEnableClientState(GL_NORMAL_ARRAY); checkGL();
    }
    glEnableClientState(GL_TEXTURE_COORD_ARRAY); checkGL();
    glVertexPointer(3,GL_FLOAT,0,p_t3dm->p_v); checkGL();
    glNormalPointer(GL_FLOAT, 0, p_t3dm->p_n); checkGL();
    glTexCoordPointer(2,GL_FLOAT,0,p_t3dm->p_t); checkGL();
    unsigned int k = 0;
    for (unsigned int i = 0; i != p_matmng->p_m_sz; ++i)
    {
        Mat& material = p_matmng->p_mat[i];
        glBindTexture(GL_TEXTURE_2D, material.texture); checkGL();
        if (material.balpha_test)
            setStandardAlphaTest(true);
        if (!material.blighting)
        {
            if (p_matmng->p_bstatic_light)
            {
                glDisableClientState(GL_COLOR_ARRAY); checkGL();
            } else {
                glDisable(GL_LIGHTING); checkGL();
            }
            glColor4fv(material.color); checkGL();
        }
        if (material.bboth_side && !p_matmng->p_bstatic_light)
        {
            glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE); checkGL();
            glDisable(GL_CULL_FACE); checkGL();
        }

        // TADY JE TO GRÓ
        if (p_octopus != NULL)
        {
            if (material.special == 0)
            {
                for (unsigned int j = 0; j != p_octopus->p_vw_sz; ++j)
                {
                    glDrawElements(GL_TRIANGLES, p_octopus->p_vw[j]->p_mi[i].p_sz, GL_UNSIGNED_SHORT, p_octopus->p_vw[j]->p_mi[i].p_i); checkGL(); afterDrawcall();
                }
            }
        } else { // vykreslování objektu bez octopusu
            if (material.special == 0)
            {
                while (k != p_t3dm->p_sz && p_t3dm->p_o[k].p_m == i)
                {
                    // sem přidat transformace

                    if (p_t3dm->p_o[k].p_gi != 1)
                    {
                        if (p_transf)
                        {
                            glPushMatrix(); checkGL();
                            p_transf->mult_mwmx(p_t3dm->p_o[k].p_gi);
                        }
                        glDrawElements(GL_TRIANGLES, p_t3dm->p_o[k].p_sz, GL_UNSIGNED_SHORT, p_t3dm->p_o[k].p_i); checkGL(); afterDrawcall();
                        if (p_transf)
                        {
                            glPopMatrix(); checkGL();
                        }
                    }
                    ++k;
                }
            } else if (material.special == 1) { // if material.special == 1
                unsigned int l = k;
                while (l != p_t3dm->p_sz && p_t3dm->p_o[l].p_m == i)
                {
                    glDrawElements(GL_TRIANGLES, p_t3dm->p_o[l].p_sz, GL_UNSIGNED_SHORT, p_t3dm->p_o[l].p_i); checkGL(); afterDrawcall();
                    ++l;
                }
                l = k;
                //if (!material.blighting) // disable lighting
                {
                    if (p_matmng->p_bstatic_light)
                    {
                        glDisableClientState(GL_COLOR_ARRAY); checkGL();
                    } else {
                        glDisable(GL_LIGHTING); checkGL();
                    }
                }
                glFrontFace(GL_CW); checkGL();
                glColor4f(0, 0, 0, 1); checkGL();
                while (l != p_t3dm->p_sz && p_t3dm->p_o[l].p_m == i)
                {
                    glDrawElements(GL_TRIANGLES, p_t3dm->p_o[l].p_sz, GL_UNSIGNED_SHORT, p_t3dm->p_o[l].p_i); checkGL(); afterDrawcall();
                    ++l;
                }
                glFrontFace(GL_CCW); checkGL();

                if (p_matmng->p_bstatic_light)
                {
                    glEnableClientState(GL_COLOR_ARRAY); checkGL();
                } else {
                    glEnable(GL_LIGHTING); checkGL();
                }

                k = l;
            } else { // přeskočení jiných typů materiálů než 0 a 1
                unsigned int l = k;
                while (l != p_t3dm->p_sz && p_t3dm->p_o[l].p_m == i)
                {
                    ++l;
                }

                k = l;
            }
        }

        if (material.bboth_side && !p_matmng->p_bstatic_light)
        {
            glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE); checkGL();
            glEnable(GL_CULL_FACE); checkGL();
        }
        if (material.bboth_side && p_matmng->p_bstatic_light)
        {
                glFrontFace(GL_CW); checkGL();
                glColorPointer(4, GL_FLOAT, 0, p_matmng->p_vcolor_back); checkGL();
                for (unsigned int j = 0; j != p_octopus->p_vw_sz; ++j)
                {
                    glDrawElements(GL_TRIANGLES, p_octopus->p_vw[j]->p_mi[i].p_sz, GL_UNSIGNED_SHORT, p_octopus->p_vw[j]->p_mi[i].p_i); checkGL(); afterDrawcall();
                }
                glColorPointer(4, GL_FLOAT, 0, p_matmng->p_vcolor); checkGL();
                glFrontFace(GL_CCW); checkGL();
        }

        if (material.balpha_test)
            setStandardAlphaTest(false);
        if (!material.blighting)
        {
            if (p_matmng->p_bstatic_light)
            {
                glEnableClientState(GL_COLOR_ARRAY); checkGL();
            } else {
                glEnable(GL_LIGHTING); checkGL();
            }
        }
    }
    glDisableClientState(GL_VERTEX_ARRAY); checkGL();
    if (p_matmng->p_bstatic_light)
    {
        glDisableClientState(GL_COLOR_ARRAY); checkGL();
    }
    else
    {
        glDisableClientState(GL_NORMAL_ARRAY); checkGL();
    }
    glDisableClientState(GL_TEXTURE_COORD_ARRAY); checkGL();
}

void Rendermng::render_o_pass_s2()
{
    if (p_boctocube && !b_visible)
        return;
    glEnableClientState(GL_VERTEX_ARRAY); checkGL();
    if (p_matmng->p_bstatic_light)
    {
        glDisable(GL_LIGHTING); checkGL();
        glEnableClientState(GL_COLOR_ARRAY); checkGL();
        glColorPointer(4, GL_FLOAT, 0, p_matmng->p_vcolor); checkGL();
    } else {
        glEnable(GL_LIGHTING); checkGL();
        glEnableClientState(GL_NORMAL_ARRAY); checkGL();
    }
    glEnableClientState(GL_TEXTURE_COORD_ARRAY); checkGL();
    glVertexPointer(3,GL_FLOAT,0,p_t3dm->p_v); checkGL();
    glNormalPointer(GL_FLOAT, 0, p_t3dm->p_n); checkGL();
    glTexCoordPointer(2,GL_FLOAT,0,p_t3dm->p_t); checkGL();

    glEnable(GL_BLEND); checkGL();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); checkGL();

    for (unsigned int i = 0; i != p_matmng->p_m_sz; ++i)
    {
        Mat& material = p_matmng->p_mat[i];
        glBindTexture(GL_TEXTURE_2D, material.texture); checkGL();
        if (!material.blighting)
        {
            if (p_matmng->p_bstatic_light)
            {
                glDisableClientState(GL_COLOR_ARRAY); checkGL();
            } else {
                glDisable(GL_LIGHTING); checkGL();
            }
            glColor4fv(material.color); checkGL();
        }

        // TADY JE TO GRÓ
        if (p_octopus != NULL)
        {
            if (material.special == 2)
            {
                for (unsigned int j = 0; j != p_octopus->p_vw_sz; ++j)
                {
                    glDrawElements(GL_TRIANGLES, p_octopus->p_vw[j]->p_mi[i].p_sz, GL_UNSIGNED_SHORT, p_octopus->p_vw[j]->p_mi[i].p_i); checkGL(); afterDrawcall();
                }
            }
        }

        if (!material.blighting)
        {
            if (p_matmng->p_bstatic_light)
            {
                glEnableClientState(GL_COLOR_ARRAY); checkGL();
            } else {
                glEnable(GL_LIGHTING); checkGL();
            }
        }
    }
    glDisable(GL_BLEND); checkGL();

    glDisableClientState(GL_VERTEX_ARRAY); checkGL();
    if (p_matmng->p_bstatic_light)
    {
        glDisableClientState(GL_COLOR_ARRAY); checkGL();
    }
    else
    {
        glDisableClientState(GL_NORMAL_ARRAY); checkGL();
    }
    glDisableClientState(GL_TEXTURE_COORD_ARRAY); checkGL();
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

    bhiquality = strcmp(fname, "stromy0a.png.3mt") == 0 || strcmp(fname, "asphalt.jpg.3mt") == 0;
    if (g_hq_textures)
    {
        bhiquality = true;
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

void Matmng::load(const T3dm* t3dm, const float* ambcolor, const float* diffcolor, const float* lightpos)
{
    p_t3dm = t3dm;
    p_m_sz = p_t3dm->p_m_sz;
    p_mat = new Mat[p_m_sz];
    char buff[256] = {0};
    for (unsigned int i = 0; i != p_m_sz; ++i)
    {
        if (*(p_t3dm->p_m[i]))
        {
            strncat1(buff, p_t3dm->p_m[i], ".3mt", 256);
            p_mat[i].load(buff);
            //printf("%s\n", buff);
        } else {
            p_mat[i].load("");
        }
    }
    if (diffcolor && ambcolor && lightpos) // je zadáno světlo pro výpočet osvětlení
    {
        p_bstatic_light = true;
        p_vcolor = new float[p_t3dm->p_v_sz*4];
        p_vcolor_back = new float[p_t3dm->p_v_sz*4];
        for (unsigned int i = 0; i != p_t3dm->p_v_sz; ++i)
        {
            float nl = lightpos[0]*p_t3dm->p_n[i*3]+lightpos[1]*p_t3dm->p_n[i*3+1]+lightpos[2]*p_t3dm->p_n[i*3+2];
            for (unsigned int j = 0; j != 3; ++j)
            {
                p_vcolor[i*4+j] = minfloat(maxfloat(ambcolor[j]+maxfloat(diffcolor[j]*nl, 0.f), 0.f), 1.f);
            }
            p_vcolor[i*4+3] = 1.f;
            for (unsigned int j = 0; j != 3; ++j)
            {
                p_vcolor_back[i*4+j] = minfloat(maxfloat(ambcolor[j]+maxfloat(diffcolor[j]*(-nl), 0.f), 0.f), 1.f);
            }
            p_vcolor_back[i*4+3] = 1.f;
        }
        //I=c_a I_a +c_d I_d (NL) // +c_s I_s (VR)^n // specular u statického osvětlení nebude
    } else {
        p_bstatic_light = false;
    }
    // načíst textury
    for (unsigned int i = 0; i != p_m_sz; ++i)
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
                else if (strSuff(p_mat[i].texd_name, ".png"))
                {
                    gbuff_in.f_open(p_mat[i].texd_name, "rb");
                    pict.loadpng(gbuff_in.fbuffptr(), gbuff_in.fbuffsz());
                    gbuff_in.fclose();
                }
                else
                {
                    pict.loaderr();
                }

                Pict2 picta;
                bool forceMipmap = false;
                if (strSuff(p_mat[i].texa_name, ".jpg"))
                {
                    gbuff_in.f_open(p_mat[i].texa_name, "rb");
                    picta.loadjpeg(gbuff_in.fbuffptr(), gbuff_in.fbuffsz());
                    gbuff_in.fclose();
                }
                else if (strSuff(p_mat[i].texa_name, ".png"))
                {
                    gbuff_in.f_open(p_mat[i].texa_name, "rb");
                    picta.loadpng(gbuff_in.fbuffptr(), gbuff_in.fbuffsz());
                    gbuff_in.fclose();
                }
                else
                {
                    pict.loaderr();
                }

                pict.r2a(picta);
                if (p_mat[i].bhiquality)
                {
                    pict.scale(256, 256);
                }
                else
                {
                    pict.scale(128, 128);
                }
                pict.pack16();
                p_mat[i].texture = load_texture(pict, p_mat[i].bmipmap || forceMipmap);
            } else { // 24 bit
                Pict2 pict;
                if (strSuff(p_mat[i].texd_name, ".jpg"))
                {
                    gbuff_in.f_open(p_mat[i].texd_name, "rb");
                    pict.loadjpeg(gbuff_in.fbuffptr(), gbuff_in.fbuffsz());
                    gbuff_in.fclose();
                }
                else if (strSuff(p_mat[i].texd_name, ".png"))
                {
                    gbuff_in.f_open(p_mat[i].texd_name, "rb");
                    pict.loadpng(gbuff_in.fbuffptr(), gbuff_in.fbuffsz());
                    gbuff_in.fclose();
                }
                else if (strSuff(p_mat[i].texd_name, ".omg"))
                {
                    gbuff_in.f_open(p_mat[i].texd_name, "rb");
                    pict.loadomg(gbuff_in.fbuffptr(), gbuff_in.fbuffsz());
                    gbuff_in.fclose();
                }
                else
                {
                    pict.loaderr();
                }

                if (p_mat[i].bhiquality)
                {
                    pict.scale(256, 256);
                }
                else
                {
                    pict.scale(128, 128);
                }
                pict.pack16();
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
            else if (strSuff(p_mat[i].texa_name, ".png"))
            {
                gbuff_in.f_open(p_mat[i].texa_name, "rb");
                picta.loadpng(gbuff_in.fbuffptr(), gbuff_in.fbuffsz());
                gbuff_in.fclose();
            }
            else
            {
                picta.loaderr();
            }
            picta.r2a();
            if (p_mat[i].bhiquality)
            {
                picta.scale(256, 256);
            }
            else
            {
                picta.scale(128, 128);
            }
            picta.pack16();
            p_mat[i].texture = load_texture(picta, p_mat[i].bmipmap);
        }
    }
}
