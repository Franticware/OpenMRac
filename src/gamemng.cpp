#include "gamemng.h"
#include "cstring1.h"
#include "rand1.h"
#include "gbuff_in.h"
#include "pict2.h"
#include "load_texture.h"
#include <cmath>
#include <algorithm>
#include <cstdio>

#include <SDL2/SDL.h>

#include "soundmng.h"

#include "glm1.h"

SDL_Window* gameWindow = 0;

int ge_bpass1 = 1;

float g_state0_time = 0.f;

int g_freecam = 0;

extern std::vector<ALuint> global_al_buffers;

// minimální počet trojúhelníků potřebný k tomu, aby proběhlo subdivide
unsigned int map_octopus_min_tris = 150;
float far_plane = 80.f;

//////////// funkce pro Carcam

void Carcam::init(float r, float y, float ang, float h_ang, const float* angp, const float* ang0p, const float* pos0, const TimeSync* timesync, const Collider* collider)
{
    p_ang = p_ang0 = ang;
    p_r = r;
    p_y = y;
    p_h_ang = h_ang;
    p_ang_base = angp;
    p_ang0_base = ang0p;
    p_pos0_base = pos0;
    p_timesync = timesync;
    p_mindist = 0.1f;

    p_collider = collider;
}

void Carcam::update(bool bstep)
{
    float speed = 1.5;
    if (bstep) // přepočet kamery o časový krok
    {
        if (p_ang < *p_ang_base)
        {
            p_ang += speed*sqrtf(*p_ang_base-p_ang)*p_timesync->p_T;
            if (p_ang > *p_ang_base)
                p_ang = *p_ang_base;
        }
        else if (p_ang > *p_ang_base)
        {
            p_ang -= speed*sqrtf(p_ang-*p_ang_base)*p_timesync->p_T;
            if (p_ang < *p_ang_base)
                p_ang = *p_ang_base;
        }
        if (p_ang > *p_ang_base+M_PI)
            p_ang -= 2.f*M_PI;
        else if (p_ang < *p_ang_base-M_PI)
            p_ang += 2.f*M_PI;
    } else { // interpolace přes vypočtenou rychlost podle polohy času uvnitř kroku
        if (p_ang < *p_ang0_base)
        {
            p_ang0 = p_ang+speed*sqrtf(*p_ang0_base-p_ang)*p_timesync->p_prevT;
            if (p_ang0 > *p_ang0_base)
                p_ang0 = *p_ang0_base;
        }
        else if (p_ang > *p_ang0_base)
        {
            p_ang0 = p_ang-speed*sqrtf(p_ang-*p_ang0_base)*p_timesync->p_prevT;
            if (p_ang0 < *p_ang0_base)
                p_ang0 = *p_ang0_base;
        }

        p_r0 = p_r; // zmenšit podle kolizí

        float r0_pom = p_r0+p_mindist;
        float vect0_base[2] = {-cosf(-p_ang0), sinf(-p_ang0)}; // jednotkový vektor
        float vect0[2] = {vect0_base[0]*r0_pom, vect0_base[1]*r0_pom};

        float pos0_base_kolmy_posun = 0.15f;
        float pos0_base_pom0[2] = {p_pos0_base[0]+vect0_base[1]*pos0_base_kolmy_posun, p_pos0_base[1]-vect0_base[0]*pos0_base_kolmy_posun};
        float pos0_base_pom1[2] = {p_pos0_base[0]-vect0_base[1]*pos0_base_kolmy_posun, p_pos0_base[1]+vect0_base[0]*pos0_base_kolmy_posun};
        float* pos0_base_pom[2] = {pos0_base_pom0, pos0_base_pom1};

        bool bintersect = false;
        float intersect_i = 0.f;
        int segm_n = int(floor(r0_pom/(p_collider->p_r*2.f)))+1;
        if (segm_n < 1) segm_n = 1;
        int colg_i_prev = -1;
        for (int i = 0; i != segm_n; ++i)
        {
            float r0_m = (float(i)+0.5f)*(r0_pom)/float(segm_n); // poloměr středu pro kolizní čtverec
            float stredg[2] = {vect0_base[0]*r0_m+p_pos0_base[0], vect0_base[1]*r0_m+p_pos0_base[1]};
            int colg_i = p_collider->get_subg(stredg);
            if (colg_i != -1 && colg_i_prev != colg_i)
            {
                const ColliderGrid &colg = p_collider->p_colg[colg_i];
                // hledají se průsečíky přímek objektu s přímkami mapy a zároveň se hledá nejvzdálenější bod mimo mapu
                for (unsigned int i = 0; i != colg.lines_sz; ++i)
                {
                    for (int j = 0; j != 2; ++j)
                    {
                        float int_res[2]; // výsledek testu průsečíku
                        RBf::intersect3(int_res, colg.lines[i]->x, colg.lines[i]->v, pos0_base_pom[j], vect0);

                        if (isintersect(int_res))
                        {
                            if (!bintersect)
                            {
                                intersect_i = int_res[1];
                            } else {
                                intersect_i = std::min(intersect_i, int_res[1]);
                            }
                            bintersect = true;
                        }
                    }
                }
            }
            colg_i_prev = colg_i;
        }

        if (bintersect)
        {
            r0_pom *= intersect_i;

            p_r0 = r0_pom - p_mindist;
        }
    }
}

glm::mat4 Carcam::transf()
{
    glm::mat4 ret = glm::rotate(glm::mat4(1.f), glm::radians(-p_h_ang), glm::vec3(1.f, 0.f, 0.f));
    ret = glm::translate(ret, glm::vec3(0, 0, -p_r0));
    ret = glm::rotate(ret, glm::radians(-p_ang0*57.29577951308232f+180.f), glm::vec3(0.f, 1.f, 0.f));
    ret = glm::translate(ret, glm::vec3(-p_pos0_base[1], -p_y, -p_pos0_base[0]));
    return ret;
}

//////////// end Carcam

bool playerstate_comp(const Playerstate& a, const Playerstate& b)
{
    if (a.state_finish)
    {
        if (!b.state_finish)
            return true;
        if (a.race_time < b.race_time)
            return true;
    }
    else if (!b.state_finish)
    {
        if (a.lap_i_max > b.lap_i_max) // pokud žádný z obou hráčů nedokončil, lepší je ten, který _dokončil_ víc kol; přednost má ten, který má nižší číslo
            return true;
    }
    return false;
}

void Gamemng::quit_race()
{
    for (unsigned int i = 0; i != p_players; ++i)
        p_sound_car[i].stop();
}

void Gamemng::end_race()
{
    p_state = 2;
    for (unsigned int i = 0; i != p_players; ++i)
        p_sound_car[i].stop();
    p_results.position.puts(0, "\n\n\n");
    p_results.time.puts(0, "\n\n\n");
    p_results.best.puts(0, "\n\n\n");

    Playerstate playerstate[4];
    std::copy(p_playerstate, p_playerstate+p_players, playerstate);
    std::stable_sort(playerstate, playerstate+p_players, playerstate_comp);

    float time_no_1 = 0.f;

    for (unsigned int i = 0; i != p_players; ++i)
    {
        if (time_no_1 == 0.f)
        {
            time_no_1 = playerstate[i].best_time;
        } else {
            if (playerstate[i].best_time != 0.f)
            {
                if (playerstate[i].best_time < time_no_1)
                {
                    time_no_1 = playerstate[i].best_time;
                }
            }
        }
    }

    for (unsigned int i = 0; i != p_players; ++i)
    {
        char buff[256] = {0};
        if (playerstate[i].state_finish)
        {
            snprintf(buff, 255, "%u. Player %d", i+1, playerstate[i].player+1);
            float font_color[4] = {1, 1, 1, 1};
            p_results.position.set_color(i, font_color);
        }
        else
        {
            snprintf(buff, 255, "DNF Player %d", playerstate[i].player+1);
            float font_color[4] = {0.6, 0.6, 0.6, 1};
            p_results.position.set_color(i, font_color);
        }
        p_results.position.puts(i, buff);
        if (playerstate[i].state_finish)
            p_results.time.puts(i, time_m_s(playerstate[i].race_time));
        if (playerstate[i].best_time != 0.f)
            p_results.best.puts(i, time_m_s(playerstate[i].best_time));
        if (playerstate[i].best_time == time_no_1)
        {
            float font_color[4] = {1, 1, 1, 1};
            p_results.best.set_color(i, font_color);
        } else {
            float font_color[4] = {0.6, 0.6, 0.6, 1};
            p_results.best.set_color(i, font_color);
        }
    }
}

void Gamemng::set_far(int far1)
{
    int far_max = 10;
    if (far1 < 0) far1 = 0; else if (far1 > far_max) far1 = far_max;
    p_far = far1;

    float far0 = 60.f;
    float far10 = 200.f;

    float ffar = far0 + (far10-far0)*far1/float(far_max);
    p_frust[3] = ffar;
    set_proj_mtrx();
    float frustum[6] = {-p_frust[0], p_frust[0], -p_frust[1], p_frust[1], p_frust[2], p_frust[3]};
    if (p_players == 2)
    {
        frustum[0] *= 2.f;
        frustum[1] *= 2.f;
    }
    p_map_oct->init_frustum(frustum);
    for (std::vector<Mapobj>::iterator it = p_mapobjs.begin(); it != p_mapobjs.end(); ++it)
        it->rendermng->p_octocube_base.init_frustum(frustum);
    for (unsigned int i = 0; i != p_players; ++i)
        p_carrendermng[i].p_octocube_base.init_frustum(frustum);
    if (p_isGhost) // rendering
    {
        for (int i = 0; i != 2; ++i)
        {
            p_ghostrendermng[i].p_octocube_base.init_frustum(frustum);
        }
    }
}

void Gamemng::init_sound()
{
    p_sound_crash = std::make_unique<Sound_crash>();
    p_sound_crash->init(p_sound_game_static.p_hit_stream); // nulové *p_audiodevice je ošéfované uvnitř
    p_sound_crash->p_global_volume = &p_global_volume;
}

void Gamemng::unset_scissor()
{
    glDisable(GL_SCISSOR_TEST); checkGL();
    p_shadermng.set(ShaderUniMat4::ProjMat, p_proj_mtrx0);
    p_proj_mtrx_active = p_proj_mtrx0;
}

void Gamemng::set_scissor(int player)
{
    if (p_players > 1)
    {
        glEnable(GL_SCISSOR_TEST); checkGL();
    }
    switch (p_players)
    {
    case 2:
        if (player == 0)
        {
            glScissor(0, 0, p_viewport[0], p_scissor[1]); checkGL();
        } else { // if (player == 1)
            glScissor(0, p_scissor[1], p_viewport[0], p_scissor[1]); checkGL();
        }
        break;
    case 3:
    case 4:
        switch (player)
        {
        case 0:
            glScissor(0, 0, p_scissor[0], p_scissor[1]); checkGL();
            break;
        case 1:
            glScissor(0, p_scissor[1], p_scissor[0], p_scissor[1]); checkGL();
            break;
        case 2:
            glScissor(p_scissor[0], 0, p_scissor[0], p_scissor[1]); checkGL();
            break;
        case 3:
            glScissor(p_scissor[0], p_scissor[1], p_scissor[0], p_scissor[1]); checkGL();
            break;
        }
    }

    p_shadermng.set(ShaderUniMat4::ProjMat, p_proj_mtrx[player]);
    p_proj_mtrx_active = p_proj_mtrx[player];
}

void Gamemng::init(const char* maps_def, const char* objs_def, const char* cars_def, const char* skies_def)
{
    p_shadermng.init();


    /*glGenTextures(1, &p_whitetex);
    p_whitetex.*/


    {
        uint32_t white_pix[256];
        glGenTextures(1, &p_whitetex); checkGL();
        memset(white_pix, 0xff, 256 * 4);
        glBindTexture(GL_TEXTURE_2D, p_whitetex); checkGL();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, white_pix); checkGL();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); checkGL();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); checkGL();
        glBindTexture(GL_TEXTURE_2D, 0); checkGL();
    }



    GLint viewport_pom[4];
    glGetIntegerv(GL_VIEWPORT, viewport_pom); checkGL();
    p_viewport[0] = viewport_pom[2];
    p_viewport[1] = viewport_pom[3];
    p_scissor[0] = p_viewport[0]/2;
    p_scissor[1] = p_viewport[1]/2;
    p_aspect = float(p_viewport[0])/float(p_viewport[1]);
    p_aspect = std::max(p_aspect, 1.25f);
    p_aspect = std::min(p_aspect, 16.f/9.f);
    p_wide169 = (1.6f + 16.f/9.f)*0.5f < p_aspect;

    float ang_h_base = 70.f  * 0.0174532925199433f; // horizontální základ úhlu
    float frust_h_base = tanf(ang_h_base*0.5f);
    float asp_base = 4.f/3.f;
    float s_base = frust_h_base*frust_h_base/asp_base;
    p_frust[0] = sqrtf(s_base*p_aspect);
    p_frust[1] = p_frust[0]/p_aspect;
    p_frust[2] = 0.1f;
    p_frust[3] = far_plane;

    p_fpscoord[1] = 0.767039/p_frust[0]*15.f;
    p_fpscoord[0] = -p_aspect*p_fpscoord[1];

    char buff[1024] = {0};
    if (gbuff_in.f_open(maps_def, "r"))
    {
        while (gbuff_in.fgets(buff, 1024))
        {
            uncomment(buff);
            if (!strempty(buff))
            {
                Gamemap gamemap;
                gamemap.filename[255] = '\0';
                gamemap.filename[0] = '\0'; // asi zbytečné, nulování v konstruktoru
                gamemap.filename_tex[0] = gamemap.filename_tex[255] = 0;
                gamemap.name[0] = gamemap.name[255] = 0;
                sscanf(buff, "%255s %f %f %255s %255s", gamemap.filename, &(gamemap.light_ah), &(gamemap.light_av),
                    gamemap.filename_tex, gamemap.name);
                if (strcmp(gamemap.filename, "/*") == 0) // zahození zbytku souboru
                    break;
                for (unsigned int i = 0; i < strlen(gamemap.name); ++i)
                {
                    if (gamemap.name[i] == '_')
                        gamemap.name[i] = ' ';
                }
                p_maps.push_back(gamemap);
            }
        }
        gbuff_in.fclose();
    }

    if (gbuff_in.f_open(objs_def, "r"))
    {
        while (gbuff_in.fgets(buff, 1024))
        {
            uncomment(buff);
            if (!strempty(buff))
            {
                Gameobj gameobj;
                gameobj.filename[0] = gameobj.filename[255] = '\0'; // nevím, jestli je to nutné
                sscanf(buff, "%255s %f %f %f", gameobj.filename, &(gameobj.m), &(gameobj.r), &(gameobj.f));
                p_objs.push_back(std::move(gameobj));
            }
        }
        gbuff_in.fclose();
    }

    if (gbuff_in.f_open(cars_def, "r"))
    {
        while (gbuff_in.fgets(buff, 1024))
        {
            uncomment(buff);
            if (!strempty(buff))
            {
                Gamecar gamecar;
                gamecar.filename[0] = gamecar.filename[255] = '\0';
                gamecar.filename_cmo[0] = gamecar.filename_cmo[255] = '\0';
                gamecar.fname_sample_engine0[0] = gamecar.fname_sample_engine0[255] = 0;
                gamecar.fname_sample_engine1[0] = gamecar.fname_sample_engine1[255] = 0;
                gamecar.name[0] = gamecar.name[255] = 0;
                sscanf(buff, "%255s %255s %255s %255s %f %f %f %f %f %f %f %255s", gamecar.filename, gamecar.filename_cmo,
                    gamecar.fname_sample_engine0, gamecar.fname_sample_engine1, &gamecar.engine1_pitch,
                    &gamecar.exhaust_position[0], &gamecar.exhaust_position[1], &gamecar.exhaust_position[2],
                    &gamecar.exhaust_direction[0], &gamecar.exhaust_direction[1], &gamecar.exhaust_direction[2],
                    gamecar.name);
                glm::normalize(gamecar.exhaust_direction);
                for (unsigned int i = 0; i < strlen(gamecar.name); ++i)
                {
                    if (gamecar.name[i] == '_')
                        gamecar.name[i] = ' ';
                }
                p_cars.push_back(gamecar);
            }
        }
        gbuff_in.fclose();
        for (unsigned int i = 0; i != p_cars.size(); ++i) {
            gbuff_in.f_open(p_cars[i].fname_sample_engine0, "rb");
            alGenBuffers(1, &(p_cars[i].p_engine0_sample)); global_al_buffers.push_back(p_cars[i].p_engine0_sample);
            swapArrayLE16(gbuff_in.fbuffptr(), gbuff_in.fbuffsz());
            tweakLoop(gbuff_in.fbuffptr(), gbuff_in.fbuffsz());
            alBufferData(p_cars[i].p_engine0_sample, AL_FORMAT_MONO16, gbuff_in.fbuffptr(), gbuff_in.fbuffsz(), 22050);
            gbuff_in.fclose();

            gbuff_in.f_open(p_cars[i].fname_sample_engine1, "rb");
            alGenBuffers(1, &(p_cars[i].p_engine1_sample)); global_al_buffers.push_back(p_cars[i].p_engine1_sample);
            swapArrayLE16(gbuff_in.fbuffptr(), gbuff_in.fbuffsz());
            tweakLoop(gbuff_in.fbuffptr(), gbuff_in.fbuffsz());
            alBufferData(p_cars[i].p_engine1_sample, AL_FORMAT_MONO16, gbuff_in.fbuffptr(), gbuff_in.fbuffsz(), 22050);
            gbuff_in.fclose();
        }
    }

    for (unsigned int i = 0; i != p_cars.size(); ++i)
    {
        if (gbuff_in.f_open(p_cars[i].filename_cmo, "r"))
        {
            if (gbuff_in.fgets(buff, 1024))
            {
                uncomment(buff);
                unsigned int sz_mods;
                sscanf(buff, "%d %d", &(p_cars[i].sz_names), &sz_mods);
                if (p_cars[i].sz_names > 0)
                    --p_cars[i].sz_names;
                p_cars[i].pict_tex.clear();
                p_cars[i].pict_tex.resize(sz_mods);
                p_cars[i].names.clear();
                if (p_cars[i].sz_names > 0)
                {
                    p_cars[i].names.resize((p_cars[i].sz_names)*p_cars[i].pict_tex.size());
                }
                for (unsigned int j = 0; j != p_cars[i].pict_tex.size(); ++j)
                    p_cars[i].pict_tex[j].tex = 0;
                int j = 0;
                while (gbuff_in.fgets(buff, 1024) && j != int(p_cars[i].pict_tex.size()))
                {
                    uncomment(buff);
                    int start_char = 0;
                    float car_color[3] = {0.6, 0.6, 0.6};
                    sscanf(buff, "%f %f %f%n", car_color+0, car_color+1, car_color+2, &start_char);
                    p_cars[i].pict_tex[j].color[0] = car_color[0];
                    p_cars[i].pict_tex[j].color[1] = car_color[1];
                    p_cars[i].pict_tex[j].color[2] = car_color[2];
                    p_cars[i].pict_tex[j].color[3] = 1.f;
                    if (!strempty(buff))
                    {
                        char * pch = strtok(buff+start_char, " \n\t\r");
                        int k = 0;
                        while (pch != NULL && k != int(p_cars[i].sz_names+1))
                        {
                            if (k >= 1)
                            {
                                p_cars[i].names[j*p_cars[i].sz_names+k-1] = pch;
                            }
                            else {
                                strncpy(p_cars[i].pict_tex[j].fname, pch, 255);
                            } // načti texturu pch

                            pch = strtok(NULL, " \n\t\r");
                            ++k;
                        }
                        ++j;
                    }
                }
            }
            gbuff_in.fclose();
        }
    }

    if (gbuff_in.f_open(skies_def, "r"))
    {
        while (gbuff_in.fgets(buff, 1024))
        {
            uncomment(buff);
            if (!strempty(buff))
            {
                Gamesky gamesky;
                gamesky.sky_tex[0] = gamesky.sky_tex[255] = '\0';
                gamesky.skycm_tex[0] = gamesky.skycm_tex[255] = '\0';
                sscanf(buff, "%255s %255s %f %f %f %f %f %f", gamesky.sky_tex, gamesky.skycm_tex,
                    gamesky.light_amb, gamesky.light_amb+1, gamesky.light_amb+2, gamesky.light_diff, gamesky.light_diff+1,gamesky.light_diff+2);
                gamesky.light_amb[3] = 1;
                gamesky.light_diff[3] = 1;
                p_skies.push_back(gamesky);
            }
        }
        gbuff_in.fclose();
    }

    p_skysph.init(this, 10, 0);
    {
        Pict2 pictsmoke;
        gbuff_in.f_open("smokea.png", "rb");
        pictsmoke.loadpng(gbuff_in.fbuffptr(), gbuff_in.fbuffsz());
        gbuff_in.fclose();
        pictsmoke.r2a();
        p_smoketex = load_texture(pictsmoke, false);
    }
    unsigned int fontsize[2] = {16, 6};
    unsigned int charsize[2] = {32, 38};
    p_glfont.init(fontsize, charsize, 34, "font4b.png");

    Pict2 pictfont_rgba;
    gbuff_in.f_open("fontd.png", "rb");
    pictfont_rgba.loadpng(gbuff_in.fbuffptr(), gbuff_in.fbuffsz());
    gbuff_in.fclose();
    Pict2 pictfont_a;
    gbuff_in.f_open("fonta.png", "rb");
    pictfont_a.loadpng(gbuff_in.fbuffptr(), gbuff_in.fbuffsz());
    gbuff_in.fclose();
    pictfont_rgba.r2a(pictfont_a);
    p_fonttex = load_texture(pictfont_rgba);
    p_glfont.set_texture(p_fonttex); // kvůli render_c();

    init_hud();

    float font_color[4] = {1, 1, 1, 1};
    p_results.title.init(15, 1, 2.f, 0, -1, &p_glfont, font_color);
    p_results.title.set_pos(0.f, 12.f);
    p_results.title.puts(0, "Race Results");

    p_results.line0.init(50, 1, 1.5f, 0, -1, &p_glfont, font_color);
    p_results.line0.set_pos(0.f, 9.f);
    p_results.line0.puts(0, "Position          Time          Best Lap");

    p_results.position.init(20, 4, 1.5f, -1, -1, &p_glfont, font_color);
    p_results.position.set_pos(-8.f, 7.f);
    p_results.position.puts(0, "1. Player 1\n"
                            "2. Player 2\n"
                            "DNF Player 3\n"
                            "DNF Player 4");

    p_results.time.init(20, 4, 1.5f, -1, -1, &p_glfont, font_color);
    p_results.time.set_pos(2.7f, 7.f);
    p_results.time.puts(0, time_m_s(2375.316));
    p_results.time.puts(1, time_m_s(0));
    p_results.time.puts(2, time_m_s(0));
    p_results.time.puts(3, time_m_s(0));

    p_results.best.init(20, 4, 1.5f, -1, -1, &p_glfont, font_color);
    p_results.best.set_pos(15.3f, 7.f);
    p_results.best.puts(0, time_m_s(0));
    p_results.best.puts(1, time_m_s(0));
    p_results.best.puts(2, time_m_s(0));
    p_results.best.puts(3, time_m_s(0));

    init_keytest();

    set_proj_mtrx(); // přidáno
    p_gamemenu.p_gamemng = this;
    p_gamemenu.init();

    p_sound_game_static.init();

    p_ghostOld = std::make_unique<Ghost>();
    p_ghostNew.clear(); p_ghostNew.resize(4);

    p_particles.resize(4);
}

void Gamemng::input(unsigned char keys[4*4])
{
    for (unsigned int i = 0; i != p_players; ++i)
    {
        p_playerkeys[i].key_up    = keys[i*4+0];
        p_playerkeys[i].key_down  = keys[i*4+1];
        p_playerkeys[i].key_left  = keys[i*4+2];
        p_playerkeys[i].key_right = keys[i*4+3];
    }
}

const char* time_m_s(float time)
{
    int time_min = int(time/60.f);
    float time_sec = fmod(time, 60.f);
    static char timebuf[15] = {0};
    snprintf(timebuf, 14, "%d:%05.2f", time_min, time_sec);
    return timebuf;
}

void Gamemng::init_keytest()
{
    float font_color[4] = {0.6, 0.6, 0.6, 1};
    float font_color1[4] = {1, 1, 1, 1};
    char buff[64] = {0};
    char controlBuffer[1024] = {0};
    for (int i = 0; i != 4; ++i)
    {
        snprintf(buff, 63, "Player %d", i+1);
        p_keytest[i].player.init(25, 1, 2.f, 0, 0, &p_glfont, font_color1);
        p_keytest[i].player.set_pos(0.f, 6.f);
        p_keytest[i].player.puts(0, buff);

        p_keytest[i].left.init(25, 1, 2.f, -1, 0, &p_glfont, font_color);
        p_keytest[i].left.set_pos(-2.f, 0.f);
        p_settings->getControlName(controlBuffer, 1023, i * 4 + 2, false);
        p_keytest[i].left.puts(0, controlBuffer);

        p_keytest[i].right.init(25, 1, 2.f, 1, 0, &p_glfont, font_color);
        p_keytest[i].right.set_pos(2.f, 0.f);
        p_settings->getControlName(controlBuffer, 1023, i * 4 + 3, false);
        p_keytest[i].right.puts(0, controlBuffer);

        p_keytest[i].down.init(25, 1, 2.f, 0, 0, &p_glfont, font_color);
        p_keytest[i].down.set_pos(0.f, -3.f);
        p_settings->getControlName(controlBuffer, 1023, i * 4 + 1, false);
        p_keytest[i].down.puts(0, controlBuffer);

        p_keytest[i].up.init(25, 1, 2.f, 0, 0, &p_glfont, font_color);
        p_keytest[i].up.set_pos(0.f, 3.f);
        p_settings->getControlName(controlBuffer, 1023, i * 4 + 0, false);
        p_keytest[i].up.puts(0, controlBuffer);
    }
}

void Gamemng::init_hud()
{
    float font_color[4] = {1, 1, 1, 1};
    p_gltext_fps.init(20, 1, 800.f/p_viewport[1]*0.5f /*5*/, 1, -1, &p_glfont, font_color);
    p_gltext_fps.set_pos(p_fpscoord[0], p_fpscoord[1]);

    const float guiShift = p_wide169 ? 1.f : 0.f;

    for (int i = 0; i != 4; ++i)
    {
        p_playerhud[i].speed.init(10, 1, 2.f, -1, 1, &p_glfont, font_color);
        p_playerhud[i].speed.set_pos(20.f, -14.2f + guiShift);
        p_playerhud[i].speed.puts(0, "100");

        p_playerhud[i].speed_km_h.init(5, 1, 1.f, -1, 1, &p_glfont, font_color);
        p_playerhud[i].speed_km_h.set_pos(20.f, -15.f + guiShift);
        p_playerhud[i].speed_km_h.puts(0, "km/h");

        p_playerhud[i].laptime_r.init(20, 1, 1.f, -1, -1, &p_glfont, font_color);
        p_playerhud[i].laptime_r.set_pos(20.f, 13.f - guiShift);
        p_playerhud[i].laptime_r.puts(0, "0:10.14");

        p_playerhud[i].laptime_r_lap.init(20, 1, 1.f, -1, -1, &p_glfont, font_color);
        p_playerhud[i].laptime_r_lap.set_pos(14.6f, 14.f - guiShift);
        p_playerhud[i].laptime_r_lap.puts(0, "           Lap");

        p_playerhud[i].laptime_r1.init(20, 2, 2.f, -1, -1, &p_glfont, font_color);
        p_playerhud[i].laptime_r1.set_pos(20.f, 14.8f - guiShift);
        p_playerhud[i].laptime_r1.puts(0, "1/50");

        p_playerhud[i].laptime_l.init(20, 3, 1.f, 1, -1, &p_glfont, font_color);
        p_playerhud[i].laptime_l.set_pos(-20.f, 15.f - guiShift);
        p_playerhud[i].laptime_l.puts(0, "Prev\nBest\nRace");

        p_playerhud[i].laptime_l_best.init(20, 2, 1.f, 1, -1, &p_glfont, font_color);
        p_playerhud[i].laptime_l_best.set_pos(-17.f, 15.f - guiShift);
        p_playerhud[i].laptime_l_best.puts(0, "0:10.14\n0:10.14");

        p_playerhud[i].laptime_l1.init(20, 1, 1.f, 1, -1, &p_glfont, font_color);
        p_playerhud[i].laptime_l1.set_pos(-17.f, 13.f - guiShift);
        p_playerhud[i].laptime_l1.puts(0, "1:10.14");

        p_playerhud[i].position.init(12, 1, 2.f, 0, 0, &p_glfont, font_color);
        p_playerhud[i].position.set_pos(0.f, 3.f - guiShift);
        p_playerhud[i].position.puts(0, "");

        p_playerhud[i].newrecord.init(30, 1, 1.5f, 0, 0, &p_glfont, font_color);
        p_playerhud[i].newrecord.set_pos(0.f, 12.6f);
        p_playerhud[i].newrecord.puts(0, "New Lap Record!");
    }

    p_gltext_start.init(10, 1, 1.f/*velikost textu*/, 0, 0, &p_glfont, font_color);
    p_gltext_start.set_pos(0.f, 0.f);
}

extern GLuint g_ghost_tex;
extern int g_ghost_w;
extern int g_ghost_h;

void Gamemng::render_frame(const glm::mat4& m)
{
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    // vykreslení oblohy
    glm::mat4 mdl_rot_mtrx = m; // zkopíruje se transformační matice
    mdl_rot_mtrx[3] = glm::vec4(0.f, 0.f, 0.f, 1.f); // translační část se vynuluje (obloha se neposouvá, jen rotuje)

    glm::mat4 sky_mat = glm::rotate(mdl_rot_mtrx, glm::radians(p_skyang), glm::vec3(0, 1, 0));

    p_shadermng.set(ShaderUniMat4::ModelViewMat, sky_mat);
    p_shadermng.set(ShaderUniMat4::ModelViewMat, sky_mat);
    p_skysph.render();

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);

    p_shadermng.set(ShaderUniMat4::ModelViewMat, m);
    p_shadermng.set(ShaderUniVec4::LightPos, m * glm::vec4(p_light_position[0], p_light_position[1], p_light_position[2], p_light_position[3]));

    // vykreslení mapy
    if (ge_bpass1)p_map_rendermng->render_o_pass1(glm::value_ptr(m)); // zjištění viditelnosti (octree)
    p_map_rendermng->render_o_pass2(m); // vykreslení viditelných částí mapy

    glDepthFunc(GL_LESS); checkGL();

    p_shadermng.set(ShaderUniMat4::ModelViewMat, m);

    p_map_rendermng->render_o_pass_s2(); // vykreslení blendů

    // render objektů
    for (unsigned int i = 0; i != p_mapobjs.size(); ++i)
    {
        Mapobj& mapobj = p_mapobjs[i];

        glm::mat4 mdl_mtrx = glm::translate(m, glm::vec3(mapobj.rbo->p_x0[1], 0.f, mapobj.rbo->p_x0[0]));
        mdl_mtrx = glm::rotate(mdl_mtrx, glm::radians(mapobj.rbo->p_ax0*57.29577951308232f), glm::vec3(0, 1, 0));

        p_shadermng.set(ShaderUniMat4::ModelViewMat, mdl_mtrx);

        if (ge_bpass1)mapobj.rendermng->render_o_pass1(glm::value_ptr(mdl_mtrx));
        mapobj.rendermng->render_o_pass_s3(); // vykreslení stínů
        mapobj.rendermng->render_o_pass2(mdl_mtrx);
    }

    glm::mat4 cm_mat =
            p_mtrx_texcm * // prvotní natočení cubemapy podle polohy slunce
            glm::transpose(mdl_rot_mtrx); // inverze aktuální rotační matice, rotace podle aktuální polohy kamery
    p_shadermng.set(ShaderUniMat4::TexMat, cm_mat);

    glActiveTexture(GL_TEXTURE0 + (int)ShaderUniTex::Cube);
    glBindTexture(GL_TEXTURE_CUBE_MAP, p_skycmtex); checkGL();
    glActiveTexture(GL_TEXTURE0);


    glm::mat4 mdl_ms[4];

    // render aut
    for (unsigned int i = 0; i != p_players; ++i)
    {
        mdl_ms[i] = glm::translate(m, glm::vec3(p_car2do[i].p_x0[1], 0.f, p_car2do[i].p_x0[0]));
        mdl_ms[i] = glm::rotate(mdl_ms[i], glm::radians(p_car2do[i].p_ax0*57.29577951308232f), glm::vec3(0, 1, 0));
        p_shadermng.set(ShaderUniMat4::ModelViewMat, mdl_ms[i]);
        if (ge_bpass1)p_carrendermng[i].render_o_pass1(glm::value_ptr(mdl_ms[i])); // zjištění, zda jde auto vidět (přes 1 bounding sphere)
        p_carrendermng[i].render_o_pass_s3(); // vykreslení stínů
        p_carrendermng[i].render_o_pass2(mdl_ms[i]); // vykreslení 1. části modelu
    }
    for (unsigned int i = 0; i != p_players; ++i)
    {
        p_carrendermng[i].render_o_pass_glassTint(mdl_ms[i]);
    }
    for (unsigned int i = 0; i != p_players; ++i)
    {
        p_carrendermng[i].render_o_pass_glassReflection(mdl_ms[i]);
    }

    static const float texCoords[8] = {0, 0, 1, 0, 1, 1, 0, 1};
    // 3.fáze renderu aut - render částic
    static std::vector<float> vertexArray;
    static std::vector<float> texCoordArray;
    static std::vector<float> colorArray;
    static std::vector<GLushort> indexArray;
    vertexArray.clear();
    texCoordArray.clear();
    colorArray.clear();

    float smokeColor[3];
    for (int i = 0; i != 3; ++i)
    {
        smokeColor[i] = p_light_ambient[i] + p_light_diffuse[i];
        if (smokeColor[i] > 1.f)
            smokeColor[i] = 1.f;
        smokeColor[i] *= 0.6; // odstín šedi 0.6 pronásobený světlem
    }

    // smoke generation
    for (unsigned int i = 0; i != p_players; ++i)
    {
        if (p_carrendermng[i].isVisible())
        {
            glm::mat4 mdl_mtrx = glm::translate(m, glm::vec3(p_car2do[i].p_x0[1], 0.f, p_car2do[i].p_x0[0]));

            for (unsigned int j = 0; j != p_particles[i].m_particleContainer.size(); ++j)
            {
                Particle& particle = p_particles[i].m_particleContainer[j];
                glm::vec3 result = mdl_mtrx * glm::vec4(particle.position[0], particle.position[1], particle.position[2], 1.f);

                for (int i = 0; i != 4; ++i)
                {
                    colorArray.push_back(smokeColor[0]);
                    colorArray.push_back(smokeColor[1]);
                    colorArray.push_back(smokeColor[2]);
                    colorArray.push_back(particle.density);
                }
                texCoordArray.push_back(texCoords[(0 + particle.texConfig * 2) % 8]);
                texCoordArray.push_back(texCoords[(1 + particle.texConfig * 2) % 8]);

                vertexArray.push_back(result[0] - particle.radius);
                vertexArray.push_back(result[1] - particle.radius);
                vertexArray.push_back(result[2]);

                texCoordArray.push_back(texCoords[(2 + particle.texConfig * 2) % 8]);
                texCoordArray.push_back(texCoords[(3 + particle.texConfig * 2) % 8]);

                vertexArray.push_back(result[0] + particle.radius);
                vertexArray.push_back(result[1] - particle.radius);
                vertexArray.push_back(result[2]);

                texCoordArray.push_back(texCoords[(4 + particle.texConfig * 2) % 8]);
                texCoordArray.push_back(texCoords[(5 + particle.texConfig * 2) % 8]);

                vertexArray.push_back(result[0] + particle.radius);
                vertexArray.push_back(result[1] + particle.radius);
                vertexArray.push_back(result[2]);

                texCoordArray.push_back(texCoords[(6 + particle.texConfig * 2) % 8]);
                texCoordArray.push_back(texCoords[(7 + particle.texConfig * 2) % 8]);

                vertexArray.push_back(result[0] - particle.radius);
                vertexArray.push_back(result[1] + particle.radius);
                vertexArray.push_back(result[2]);
            }
        }
    }
    for (GLushort i = indexArray.size()/6; i < vertexArray.size()/12; ++i)
    {
        indexArray.push_back(i * 4 + 0);
        indexArray.push_back(i * 4 + 1);
        indexArray.push_back(i * 4 + 2);

        indexArray.push_back(i * 4 + 0);
        indexArray.push_back(i * 4 + 2);
        indexArray.push_back(i * 4 + 3);
    }

    int indexArraySize = vertexArray.size()/12*6;

    if (!vertexArray.empty() && !texCoordArray.empty() && !colorArray.empty())
    {
        glEnable(GL_BLEND); checkGL();
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); checkGL();
        glBindTexture(GL_TEXTURE_2D, p_smoketex); checkGL();
        glDepthMask(GL_FALSE); checkGL();
        p_shadermng.set(ShaderUniMat4::ModelViewMat, glm::mat4(1.f));
        p_shadermng.use(ShaderId::ColorTex);
        p_shadermng.set(ShaderUniInt::AlphaDiscard, (GLint)0);
        glEnableVertexAttribArray((GLuint)ShaderAttrib::Pos); checkGL();
        glEnableVertexAttribArray((GLuint)ShaderAttrib::Tex); checkGL();
        glEnableVertexAttribArray((GLuint)ShaderAttrib::Color); checkGL(); // smoke color array
        glVertexAttribPointer((GLuint)ShaderAttrib::Pos, 3, GL_FLOAT, GL_FALSE, 0, vertexArray.data());
        glVertexAttribPointer((GLuint)ShaderAttrib::Color, 4, GL_FLOAT, GL_FALSE, 0, colorArray.data());
        glVertexAttribPointer((GLuint)ShaderAttrib::Tex, 2, GL_FLOAT, GL_FALSE, 0, texCoordArray.data());
        glDrawElements(GL_TRIANGLES, indexArraySize, GL_UNSIGNED_SHORT, indexArray.data()); // smoke render
        glDisableVertexAttribArray((GLuint)ShaderAttrib::Pos); checkGL();
        glDisableVertexAttribArray((GLuint)ShaderAttrib::Tex); checkGL();
        glDisableVertexAttribArray((GLuint)ShaderAttrib::Color); checkGL();
        glDepthMask(GL_TRUE); checkGL();
        glDisable(GL_BLEND); checkGL();
    }

    // render ghost car
    if (p_isGhost && (p_ghostUpdated || p_ghostAvailable) && p_playerstate[0].lap_i_max > 0 && p_playerstate[0].lap_i_max <= p_laps) // rendering
    {


        bool useSampleCoverage = g_multisampleMode;

        if (useSampleCoverage)
        {
            glSampleCoverage(0.5, GL_FALSE); checkGL();
            glEnable(GL_SAMPLE_COVERAGE); checkGL();
        }
        else
        {
            p_shadermng.set(ShaderUniInt::Halftone, (GLint)1);
        }

        float framef = p_ghost_time*10.0;
        int framei = framef;
        float framej = framef - float(framei);
        float framej1 = 1.0-framej;
        int framei1 = framei+1;
        float ghostX = 0, ghostY = 0, ghostA = 0;
        int visible = 0;
        if (framei1 < p_ghostOld->m_num) {
            visible = 1;
            ghostX = p_ghostOld->m_frames[framei*4+0]*framej1+p_ghostOld->m_frames[framei1*4+0]*framej;
            ghostY = p_ghostOld->m_frames[framei*4+1]*framej1+p_ghostOld->m_frames[framei1*4+1]*framej;
            ghostA = p_ghostOld->m_frames[framei*4+2]*framej1+p_ghostOld->m_frames[framei1*4+2]*framej;

            float angle_vector0 = std::cos(ghostA);
            float angle_vector1 = std::sin(ghostA);

            float ghostdiffx = ghostX-p_ghost_x_prev[0];
            float ghostdiffy = ghostY-p_ghost_x_prev[1];

            p_ghost_wheel_rot += angle_vector0*ghostdiffx+angle_vector1*ghostdiffy;

            p_ghost_x_prev[0] = ghostX;
            p_ghost_x_prev[1] = ghostY;
        }

        if (visible && (p_ghostAvailable || p_ghostUpdated)) {
            glm::mat4 mdl_mtrx = glm::translate(m, glm::vec3(ghostY, 0.f, ghostX));
            mdl_mtrx = glm::rotate(mdl_mtrx, glm::radians(ghostA*57.29577951308232f), glm::vec3(0.f, 1.f, 0.f));

            p_shadermng.set(ShaderUniMat4::ModelViewMat, mdl_mtrx);

            if (ge_bpass1) p_ghostrendermng[p_ghostUpdated].render_o_pass1(glm::value_ptr(mdl_mtrx)); // zjištění, zda jde auto vidět (přes 1 bounding sphere)
            p_ghostrendermng[p_ghostUpdated].render_o_pass_s3(); // vykreslení stínů
            p_ghostrendermng[p_ghostUpdated].render_o_pass2(mdl_mtrx); // vykreslení 1. části modelu
            p_ghostrendermng[p_ghostUpdated].render_o_pass_glassTint(mdl_mtrx);
            p_ghostrendermng[p_ghostUpdated].render_o_pass_glassReflection(mdl_mtrx);
        }
        if (useSampleCoverage)
        {
            glDisable(GL_SAMPLE_COVERAGE); checkGL();
        }
        else
        {
            p_shadermng.set(ShaderUniInt::Halftone, (GLint)0);
        }
    }
}

void Gamemng::restart()
{
    p_timesync.init();
    p_particleTimesync.init();
    p_state = 0;
#ifdef TEST_SCRSHOT
    p_state = 1;
#endif
    p_state0_time = g_state0_time;
    p_state0_5 = true;
    p_state0_5_time = 0.f;

    p_finished = 0;

    for (unsigned int i = 0; i != 4; ++i)
    {
        p_newlaprecordtxttime[i] = -1;
    }

    float startpos[8] = {-0.1f, 2.5f, -0.1f, -2.5f, -10.1f, 2.5f, -10.1f, -2.5f}; // výpočet startovních pozic aut
    if (p_reverse)
    {
        for (unsigned int i = 0; i != 4; ++i)
            startpos[i*2] *= -1.f; // otočí se x (souřadnice pozic)
    }
    float startang = p_reverse ? float(M_PI) : 0.f;
    unsigned int startpos_i[4];
    switch (p_players)
    {
    case 1:
        startpos_i[0] = randn1(2);
        break;
    case 2:
        startpos_i[0] = 0;
        startpos_i[1] = 1;
        randn1mix(startpos_i, sizeof(unsigned int), 2);
        break;
    case 3:
        startpos_i[0] = 0;
        startpos_i[1] = 1;
        startpos_i[2] = 2+randn1(2);
        randn1mix(startpos_i, sizeof(unsigned int), 3);
        break;
    default:
        startpos_i[0] = 0;
        startpos_i[1] = 1;
        startpos_i[2] = 2;
        startpos_i[3] = 3;
        randn1mix(startpos_i, sizeof(unsigned int), 4);
    }

    for (unsigned int i = 0; i != 4; ++i) // p?esun aut na start
    {
        startpos[i*2+0] += p_map_model->p_cen[3+2];
        startpos[i*2+1] += p_map_model->p_cen[3+0];
    }

    for (unsigned int i = 0; i != p_players; ++i)
    {
        p_car2do[i].p_v[0] = p_car2do[i].p_v[1] = 0.f;
        p_car2do[i].p_av = 0.f;

        p_car2do[i].p_ax = startang;
        memcpy(p_car2do[i].p_x, startpos+startpos_i[i]*2, 2*sizeof(float));

        float car_max_x = 0.f;
        for (unsigned int j = 0; j != p_car2do[i].p_bbox_sz; ++j)
        {
            car_max_x = std::max(car_max_x, p_car2do[i].p_bbox[j].x[0]);
        }
        if (p_reverse)
            p_car2do[i].p_x[0] += car_max_x;
        else
            p_car2do[i].p_x[0] -= car_max_x;
        p_car2do[i].p_x0[0] = p_car2do[i].p_x[0];

        p_carcam[i].p_ang = p_carcam[i].p_ang0 = startang;
    }
    for (std::vector<Mapobj>::iterator it = p_mapobjs.begin(); it != p_mapobjs.end(); ++it)
    {
        it->rbo->p_x[0] = it->pos[0];
        it->rbo->p_x[1] = it->pos[1];
        it->rbo->p_ax = it->ang;

        it->rbo->p_v[0] = it->rbo->p_v[1] = 0.f;
        it->rbo->p_av = 0.f;
    }
    for (int i = 0; i != 4; ++i)
    {
        p_playerstate[i].lap_i = 0;
        p_playerstate[i].lap_i_max = 0;
        p_playerstate[i].lap_time = 0.f;
        p_playerstate[i].state_prev = 1;
        p_playerstate[i].race_time = 0.f;
        p_playerstate[i].best_time = 0.f;

        p_playerstate[i].state_finish = 0; // 0 - neskončil, 1 - skončil;
        p_playerstate[i].finish_time = 0.f;
        p_playerstate[i].player = i;

        p_playerhud[i].position.puts(0, "");

        p_playerstate[i].position_time = 0.f;
        p_playerstate[i].state_position = 0;
    }
    p_ghost_time = 0.f;
    for (int i = 0; i != 4; ++i)
    {
        p_particles[i].clear();
    }
    glClear (GL_COLOR_BUFFER_BIT); checkGL();
    SDL_GL_SwapWindow(gameWindow);
    glClear (GL_COLOR_BUFFER_BIT); checkGL();
    SDL_GL_SwapWindow(gameWindow);
    glClear (GL_COLOR_BUFFER_BIT); checkGL();
}

void Gamemng::set_proj_mtrx()
{
    p_proj_mtrx0 = glm::frustum(-p_frust[0]*p_frust[2], p_frust[0]*p_frust[2],
            -p_frust[1]*p_frust[2], p_frust[1]*p_frust[2], p_frust[2], p_frust[3]);
    switch (p_players)
    {
    case 1:
        p_proj_mtrx[0] = glm::frustum(-p_frust[0]*p_frust[2], p_frust[0]*p_frust[2],
                -p_frust[1]*p_frust[2], p_frust[1]*p_frust[2], p_frust[2], p_frust[3]);
        break;
    case 2:
        p_proj_mtrx[0] = glm::frustum(-p_frust[0]*p_frust[2]*2, p_frust[0]*p_frust[2]*2,
                -p_frust[1]*p_frust[2], p_frust[1]*p_frust[2]*3, p_frust[2], p_frust[3]);
        p_proj_mtrx[1] = glm::frustum(-p_frust[0]*p_frust[2]*2, p_frust[0]*p_frust[2]*2,
                -p_frust[1]*p_frust[2]*3, p_frust[1]*p_frust[2], p_frust[2], p_frust[3]);
        break;
    default: // 2 a 3
        p_proj_mtrx[0] = glm::frustum(-p_frust[0]*p_frust[2], p_frust[0]*p_frust[2]*3,
                -p_frust[1]*p_frust[2], p_frust[1]*p_frust[2]*3, p_frust[2], p_frust[3]);
        p_proj_mtrx[1] = glm::frustum(-p_frust[0]*p_frust[2], p_frust[0]*p_frust[2]*3,
                -p_frust[1]*p_frust[2]*3, p_frust[1]*p_frust[2], p_frust[2], p_frust[3]);
        p_proj_mtrx[2] = glm::frustum(-p_frust[0]*p_frust[2]*3, p_frust[0]*p_frust[2],
                -p_frust[1]*p_frust[2], p_frust[1]*p_frust[2]*3, p_frust[2], p_frust[3]);
        p_proj_mtrx[3] = glm::frustum(-p_frust[0]*p_frust[2]*3, p_frust[0]*p_frust[2],
                -p_frust[1]*p_frust[2]*3, p_frust[1]*p_frust[2], p_frust[2], p_frust[3]);
    }
}
