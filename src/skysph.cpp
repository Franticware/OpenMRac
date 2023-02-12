#include "skysph.h"
#include "gamemng.h"
#include <cmath>
#include "gl1.h"

/* orientace úhlu */
/*
    +------- x
    |
    |
    |
    z -> ang
*/

void Skysph::init(Gamemng* gamemng, float r_prm, float ang_prm, int h, int v) // úhel ve stupních
{
    p_gamemng = gamemng;

    vert.clear();
    tris.clear();

    float ah = ang_prm/180.f*M_PI;
    float av = M_PI*0.25f;
    light_pos[0] = sinf(ah)*cosf(av);
    light_pos[1] = sinf(av);
    light_pos[2] = cosf(ah)*cosf(av);
    light_pos[3] = 0.f;
    r = r_prm;
    ang = ang_prm;
    size = v*(h)*4;
    for (int y = 0; y != v; ++y)
    {
        for (int x = 0; x != h+1; ++x)
        {
            float uhel_h = ang/180.f*M_PI+M_PI*2.f/float(h)*float(x)+M_PI;
            float uhel_v = (-M_PI/2.f)/10.f+(M_PI/2.f)*1.1/float(v)*float(y);
            vert.push_back(sinf(uhel_h)*cosf(uhel_v)*r);
            vert.push_back(sinf(uhel_v)*r);
            vert.push_back(cosf(uhel_h)*cosf(uhel_v)*r);
            vert.push_back(float(x)/float(h));
            vert.push_back(float(uhel_v/M_PI*2.f*2.f));
            int y1 = y+1;
            int x1 = (x+1);//%h;
            if (x != h)
            {
                if (y1 == v)
                {
                    tris.push_back(v*(h+1));
                    tris.push_back(x1+y*(h+1));
                    tris.push_back(x+y*(h+1));
                } else {
                    tris.push_back(x+y1*(h+1));
                    tris.push_back(x1+y1*(h+1));
                    tris.push_back(x1+y*(h+1));

                    tris.push_back(x+y1*(h+1));
                    tris.push_back(x1+y*(h+1));
                    tris.push_back(x+y*(h+1));
                }
            }
        }
    }
    vert.push_back(0.f);
    vert.push_back(r);
    vert.push_back(0.f);
    vert.push_back(0.5f);
    vert.push_back(1.f);
}

void Skysph::render(const glm::mat4& sky_mat)
{
    p_gamemng->p_shadermng.use(ShaderId::Tex);
    p_gamemng->p_shadermng.set(ShaderUniMat4::ModelViewMat, sky_mat);

    glDepthRangef(1, 1); // f is important! glDepthRange with double parameters does not work in GLES2!
    glDepthFunc(GL_LEQUAL);

    glBindTexture(GL_TEXTURE_2D, tex_sky); checkGL();

    glEnableVertexAttribArray((GLuint)ShaderAttrib::Pos);
    glVertexAttribPointer((GLuint)ShaderAttrib::Pos, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, vert.data());

    glEnableVertexAttribArray((GLuint)ShaderAttrib::Tex);
    glVertexAttribPointer((GLuint)ShaderAttrib::Tex, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, vert.data() + 3);

    glDrawElements(GL_TRIANGLES, size, GL_UNSIGNED_SHORT, tris.data()); checkGL();

    glDisableVertexAttribArray((GLuint)ShaderAttrib::Pos);
    glDisableVertexAttribArray((GLuint)ShaderAttrib::Tex);

    glDepthFunc(GL_LESS);
    glDepthRangef(0, 1);
}
