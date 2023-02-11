#include "gamemng.h"
#include "cstring1.h"
#include "rand1.h"
#include "gbuff_in.h"
#include "pict2.h"
#include <cmath>
#include <algorithm>

#include "load_texture.h"

extern unsigned int map_octopus_min_tris;

extern float g_state0_time;

bool cmp_mapobj_id(const Mapobj& m1, const Mapobj& m2)
{
    return m1.id < m2.id;
}

bool Gamemng::load(int players_sel, const int* cars_sel, const int* cars_tex_sel, int map_sel, int sky_sel, bool breverse)
{
    unload();

    if (p_maps.empty() ||
        p_objs.empty() ||
        p_cars.empty() ||
        p_skies.empty())
        return false;

    memset(p_wheel_rot, 0, sizeof(float)*4); // nastavení úhlu kol na 0
    p_reverse = breverse; // nastavení opačného směru tratě
    // nastavení rozsahu na <1; 4>
    if (players_sel > 4) players_sel = 4;
    if (players_sel < 1) players_sel = 1;
    p_players = players_sel;
    // výběr objektu s načítacími informacemi
    Gamemap& gamemap = p_maps[map_sel];
    Gamesky& gamesky = p_skies[sky_sel];

    p_skyang = gamemap.light_ah; // nastavení horizontálního úhlu světla (+natočení oblohy)
    {
        // načtení textury oblohy z bufferu
        Pict2 pict;
        gbuff_in.f_open(gamesky.sky_tex, "rb");
        pict.loadpng(gbuff_in.fbuffptr(), gbuff_in.fbuffsz());
        gbuff_in.fclose();
        GLuint skytex = load_texture(pict);
        glBindTexture(GL_TEXTURE_2D, skytex); checkGL();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); checkGL(); // vertikálně se textura neopakuje
        glBindTexture(GL_TEXTURE_2D, 0); checkGL();
        p_skysph.set_tex(skytex); //

    // CUBEMAP
        // načtení textury cube mapy
        Pict2 pict_cm;
        gbuff_in.f_open(gamesky.skycm_tex, "rb");
        pict_cm.loadpng(gbuff_in.fbuffptr(), gbuff_in.fbuffsz());
        gbuff_in.fclose();
        p_skycmtex = load_texture_cube_map(pict_cm);
    }

    // loading map...
    p_map_model     = std::make_unique<T3dm>();      // model mapy -> p_map_matmng, p_map_oct, p_map_rendermng, startpos, p_collider, seznam pohyblivých objektů
    p_map_matmng    = std::make_unique<Matmng>();    // správce textur a statického osvětlení -> p_map_rendermng
    p_map_oct       = std::make_unique<Octopus>();   //  -> p_map_rendermng
    p_map_rendermng = std::make_unique<Rendermng>(); //
    p_map_rendermng->p_skycmtex = p_skycmtex;
    static const char* model_o_names[] = {"", "bound", "mapobject", "ibound", 0};
    p_map_model->load(gamemap.filename, model_o_names);
    p_map_model->scale(10.f);

    float light_ambient[4];
    memcpy(light_ambient, gamesky.light_amb, 4*sizeof(float));
    memcpy(p_light_ambient, gamesky.light_amb, 4*sizeof(float));

    p_shadermng.set(ShaderUniVec4::LightAmbient, glm::vec4(light_ambient[0], light_ambient[1], light_ambient[2], light_ambient[3]));

    float light_diffuse[4];
    memcpy(light_diffuse, gamesky.light_diff, 4*sizeof(float));
    memcpy(p_light_diffuse, gamesky.light_diff, 4*sizeof(float));

    p_shadermng.set(ShaderUniVec4::LightDiffuse, glm::vec4(light_diffuse[0], light_diffuse[1], light_diffuse[2], light_diffuse[3]));

    p_light_position[3] = 0;
    p_light_position[0] = sinf(gamemap.light_ah*0.0174532925199433f)*cosf(gamemap.light_av*0.0174532925199433f);
    p_light_position[1] = sinf(gamemap.light_av*0.0174532925199433f);
    p_light_position[2] = cosf(gamemap.light_ah*0.0174532925199433f)*cosf(gamemap.light_av*0.0174532925199433f);

    // načte všechny materiály pro daný 3D model a vytvoří případně statické osvětlení (je jeho majitelem)
    p_map_matmng->load(p_map_model.get(), light_ambient, light_diffuse, p_light_position);

    float frustum[6] = {-p_frust[0], p_frust[0], -p_frust[1], p_frust[1], p_frust[2], p_frust[3]};
    // pokud jsou dva hráči, obraz je extrémně širokoúhlý
    if (p_players == 2)
    {
        frustum[0] *= 2.f;
        frustum[1] *= 2.f;
    }
    p_map_oct->init(frustum, *p_map_model, map_octopus_min_tris, 50);
    p_map_rendermng->init(this, p_map_model.get(), p_map_matmng.get(), p_map_oct.get());
    // end loading map

    // získání souřadnic objektů na mapě
    int i_tst = p_map_model->getgidobj(2);
    if (p_map_model->p_o.size() != 0 && i_tst != -1) // vytvoření seznamu objektů na mapě
    {
        const O3dm& m_o = p_map_model->p_o[i_tst];
        for (unsigned int i = 0; i != m_o.p_i.size(); ++i)
        {
            bool bfound = false;
            // projdou se všechny objekty v objektové mapě, aby se bod nevkládal 2x
            for (std::vector<Mapobj>::const_iterator it = p_mapobjs.begin(); it != p_mapobjs.end(); ++it)
            {
                if (it->vert_i == m_o.p_i[i] ||
                    ( // pokud je to jiný bod ve stejném místě
                    p_map_model->p_v[it->vert_i*8+0] == p_map_model->p_v[m_o.p_i[i]*8+0]&&
                    p_map_model->p_v[it->vert_i*8+1] == p_map_model->p_v[m_o.p_i[i]*8+1]&&
                    p_map_model->p_v[it->vert_i*8+2] == p_map_model->p_v[m_o.p_i[i]*8+2]
                    )) // pokud už pole obsahuje bod s indexem, je nalezen
                {
                    bfound = true;
                    break;
                }
            }
            if (!bfound)
            {
                Mapobj mapobj;
                mapobj.vert_i = m_o.p_i[i];
                mapobj.ang = p_map_model->p_cen[m_o.p_gi*3+1]+p_map_model->p_v[m_o.p_i[i]*8+1]*M_PI;
                mapobj.ang /= 10.f;
                mapobj.pos[0] = p_map_model->p_cen[m_o.p_gi*3+0]+p_map_model->p_v[m_o.p_i[i]*8+2];
                mapobj.pos[1] = p_map_model->p_cen[m_o.p_gi*3+2]+p_map_model->p_v[m_o.p_i[i]*8+0];
                const int texo_w = 8;
                const int texo_h = 8;
                int texo_x = 0;
                int texo_y = 0;
                texo_x = (int)floor(p_map_model->p_v[m_o.p_i[i]*8+6]*float(texo_w));
                texo_y = (int)floor(p_map_model->p_v[m_o.p_i[i]*8+7]*float(texo_h));
                int o_id = texo_x+texo_y*texo_w;
                --o_id;
                if (o_id < 0 || o_id >= int(p_objs.size()))
                    o_id = -1;
                mapobj.id = o_id;
                if (o_id != -1)
                    p_mapobjs.push_back(std::move(mapobj));
            }
        }
    }
    std::sort(p_mapobjs.begin(), p_mapobjs.end(), cmp_mapobj_id);

    for (unsigned int i = 0; i != p_objs.size(); ++i)
    {
        for (std::vector<Mapobj>::const_iterator it = p_mapobjs.begin(); it != p_mapobjs.end(); ++it)
        {   // načtou se data ke všem objektům použitým v mapě
            if (i == it->id) // pokud je v mapě aspoň jeden objekt
            {
                // načtou se data
                p_objs[i].t3dm = std::make_unique<T3dm>();
                const char* o_names[] = {"", "bound", 0};
                p_objs[i].t3dm->load(p_objs[i].filename, o_names);
                // načíst materiály
                p_objs[i].matmng = std::make_unique<Matmng>();
                p_objs[i].matmng->load(p_objs[i].t3dm.get());
                break;
            }
        }
    }

    p_timesync.init();
    p_particleTimesync.init();

    for (std::vector<Mapobj>::iterator it = p_mapobjs.begin(); it != p_mapobjs.end(); ++it)
    {
        it->rbo = std::make_unique<RBSolver>();
        float rbo_v[2] = {0, 0};
        it->rbo->init(it->pos, it->ang, rbo_v, 0, p_objs[it->id].m/* m */, 0.5*p_objs[it->id].m*p_objs[it->id].r*p_objs[it->id].r/* am */, &p_timesync, p_objs[it->id].t3dm.get());
        it->r = p_objs[it->id].r;
        it->f = p_objs[it->id].f;
        // vytvořit rendermng
        it->rendermng = std::make_unique<Rendermng>();
        it->rendermng->p_skycmtex = p_skycmtex;
        it->rendermng->init(this, p_objs[it->id].t3dm.get(), p_objs[it->id].matmng.get(), 0);
        it->rendermng->set_oc(frustum, *(p_objs[it->id].t3dm));
    }

    p_sound_car.resize(p_players);

    for (unsigned int i = 0; i != p_players; ++i)
    {
        p_cars_sel[i] = cars_sel[i];
        p_carmodel[i] = std::make_unique<T3dm>();
        const char* carmodel_o_names[] = {"", "bound", "wheel_fl", "wheel_fr", "wheel_back", 0};
        p_carmodel[i]->load(p_cars[cars_sel[i]].filename, carmodel_o_names);
        p_carmatmng[i] = std::make_unique<Matmng>();
        if (cars_tex_sel[i] != 0)
        {
            int cars_tex_sel_pom = cars_tex_sel[i];
            if (int(p_cars[cars_sel[i]].pict_tex.size()) <= cars_tex_sel_pom)
                cars_tex_sel_pom = p_cars[cars_sel[i]].pict_tex.size()-1;
            for (unsigned int j = 0; j != p_carmodel[i]->p_m.size(); ++j)
            {
                for (unsigned int k = 0; k != p_cars[cars_sel[i]].sz_names; ++k)
                {
                    if (strcmp(p_cars[cars_sel[i]].names[k].c_str(), p_carmodel[i]->p_m[j].c_str()) == 0)
                    {
                        p_carmodel[i]->p_m[j] = p_cars[cars_sel[i]].names[cars_tex_sel_pom*p_cars[cars_sel[i]].sz_names+k];
                    }
                }
            }
        }

        p_carmatmng[i]->load(p_carmodel[i].get());

        p_sound_game_static.load(i, p_cars[cars_sel[i]].p_engine0_sample, p_cars[cars_sel[i]].p_engine1_sample);

        // zvuky
        p_sound_car[i].p_global_volume = &p_global_volume; // je důležité, aby toto bylo první
        p_sound_car[i].init(p_sound_game_static.p_engine0_stream[i], p_sound_game_static.p_engine1_stream[i], p_cars[cars_sel[i]].engine1_pitch, p_sound_game_static.p_skid_stream[i], i, p_players);

        // částice
        p_particles[i].clear();
    }

    p_car2do.clear(); p_car2do.resize(p_players);
    p_car2dp.clear(); p_car2dp.resize(p_players);
    p_cartransf.clear(); p_cartransf.resize(p_players);
    p_carrendermng.clear(); p_carrendermng.resize(p_players);
    for (unsigned int i = 0; i != p_players; ++i)
    {
        p_carrendermng[i].p_skycmtex = p_skycmtex;
    }

    float startpos0[2] = {0, 0};

    for (unsigned int i = 0; i != p_players; ++i)
    {
        p_carrendermng[i].init(this, p_carmodel[i].get(), p_carmatmng[i].get(), 0);
        p_carrendermng[i].set_oc(frustum, *(p_carmodel[i]));
        p_cartransf[i].init(2, 3); // od druhé skupiny se budou transformovat tři objekty
        p_carrendermng[i].set_transf(p_cartransf.data()+i);

        // skutečná pozice se nastaví až pomocí restart()
        p_car2do[i].init(startpos0/* ukazatel na vylosovanou pozici auta */, 0.f, RBf::zerov(), 0, 1255/* m */, /*2000*/3000/* am */, &p_timesync, p_carmodel[i].get());

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

        p_car2dp[i].init(1.24, 0.58, -1.14, 0.58, p_car2do.data()+i);
    }

    p_isGhost = p_players == 1;
    p_ghostAvailable = 0;
    if (/*p_isGhost*/true)
    {
        p_ghost_wheel_rot = 0;
        p_ghost_x_prev[0] = 0;
        p_ghost_x_prev[1] = 0;
        p_ghostUpdated = 0;
        p_ghostAvailable = p_ghostOld->load(map_sel, breverse);
        for (unsigned int i = 0; i != 4; ++i)
        {
            p_newlaprecordtxttime[i] = -1;
            p_ghostNew[i].m_track = map_sel;
            p_ghostNew[i].m_reverse = !(!int(breverse));
            p_ghostNew[i].m_car = cars_sel[i];
            p_ghostNew[i].m_carcolor = cars_tex_sel[i];
        }
        int ghostcari[2] = {p_ghostAvailable ? p_ghostOld->m_car : 0 , cars_sel[0]};
        int ghostcarcolori[2] = {p_ghostAvailable ? p_ghostOld->m_carcolor : 0, cars_tex_sel[0]};
        p_ghostmodel.clear(); p_ghostmodel.resize(2);
        p_ghostmatmng.clear(); p_ghostmatmng.resize(2);
        p_ghosttransf.clear(); p_ghosttransf.resize(2);
        p_ghostrendermng.clear(); p_ghostrendermng.resize(2);
        p_ghostrendermng[0].p_skycmtex = p_skycmtex;
        p_ghostrendermng[1].p_skycmtex = p_skycmtex;
        const char* ghost_o_names[] = {"", "bound", "wheel_fl", "wheel_fr", "wheel_back", 0};
        for (int i = 0; i != 2; ++i) {
            p_ghostmodel[i].load(p_cars[ghostcari[i]].filename, ghost_o_names);
            if (ghostcarcolori[i] != 0) {
                if (int(p_cars[ghostcari[i]].pict_tex.size()) <= ghostcarcolori[i])
                    ghostcarcolori[i] = p_cars[ghostcari[i]].pict_tex.size()-1;
                for (unsigned int j = 0; j != p_ghostmodel[i].p_m.size(); ++j)
                {
                    for (unsigned int k = 0; k != p_cars[ghostcari[i]].sz_names; ++k)
                    {
                        if (p_cars[ghostcari[i]].names[k] == p_ghostmodel[i].p_m[j])
                        {
                            p_ghostmodel[i].p_m[j] = p_cars[ghostcari[i]].names[ghostcarcolori[i]*p_cars[ghostcari[i]].sz_names+k];
                        }
                    }
                }
            }
            p_ghostmatmng[i].load(p_ghostmodel.data()+i);
            p_ghostrendermng[i].init(this, p_ghostmodel.data()+i, p_ghostmatmng.data()+i, 0);
            p_ghostrendermng[i].set_oc(frustum, p_ghostmodel[i]);
            p_ghosttransf[i].init(2, 3); // od druhé skupiny se budou transformovat tři objekty
            p_ghostrendermng[i].set_transf(p_ghosttransf.data()+i);
        }
    }

    p_collider = std::make_unique<Collider>();
    p_rbos.clear(); p_rbos.resize(p_players+p_mapobjs.size()); // pole s objekty určenými pro kolizní testování
    for (unsigned int i = 0; i != p_players; ++i) // první prvky pole jsou hráči
        p_rbos[i] = p_car2do.data()+i;
    for (unsigned int i = 0; i != p_mapobjs.size(); ++i)
        p_rbos[i+p_players] = p_mapobjs[i].rbo.get();
    p_collider->init(8, 3, p_map_model.get(), p_rbos.data(), p_rbos.size());
    p_collider->p_players = p_players;
    p_collider->p_sound_crash = p_sound_crash.get();

    p_mtrx_texcm = glm::scale(glm::mat4(1), glm::vec3(-1, -1, -1));
    p_mtrx_texcm = glm::rotate(p_mtrx_texcm, glm::radians(-p_skyang), glm::vec3(0, 1, 0));

    for (unsigned int i = 0; i != p_players; ++i)
    {
        p_carcam[i].init(/*r*/6.f, /*y*/2.2f, p_car2do[i].p_ax, /*h_ang*/-8, &(p_car2do[i].p_ax), &(p_car2do[i].p_ax0), p_car2do[i].p_x0, &p_timesync, p_collider.get());
        p_startcam[i].init(/*r*/6.f, /*y*/2.2f, p_car2do[i].p_ax, /*h_ang*/-8, &(p_car2do[i].p_ax), &(p_car2do[i].p_ax0), p_car2do[i].p_x0, &p_timesync, p_collider.get());
    }

    set_proj_mtrx();
    restart();

    p_laps = 3;
    return true;
}

void Gamemng::unload()
{
    glDeleteTextures(1, &(p_skysph.tex_sky)); checkGL();
    p_skysph.tex_sky = 0;
    glDeleteTextures(1, &(p_skycmtex)); checkGL();
    p_skycmtex = 0;
    p_skysph.tex_sky = 0;
    // v destruktoru zrušit texturu slunce

    p_map_model = nullptr;
    p_map_matmng = nullptr;
    p_map_oct = nullptr;
    p_map_rendermng = nullptr;
    // smazat p_mapobjs

    for (unsigned int i = 0; i != p_objs.size(); ++i)
    {
        p_objs[i].t3dm = nullptr;
        p_objs[i].matmng = nullptr;
    }

    for (std::vector<Mapobj>::iterator it = p_mapobjs.begin(); it != p_mapobjs.end(); ++it)
    {
        it->rbo = nullptr;
        it->rendermng = nullptr;
    }
    p_mapobjs.clear();

    for (unsigned int i = 0; i != p_players; ++i)
    {
        p_carmodel[i] = nullptr;
        p_carmatmng[i] = nullptr;
        p_sound_car[i].stop();
    }

    p_ghostmodel.clear();
    p_ghostmatmng.clear();
    p_ghostrendermng.clear();
    p_ghosttransf.clear();

    p_car2do.clear();
    p_car2dp.clear();
    p_cartransf.clear();
    p_carrendermng.clear();
    p_sound_car.clear();

    p_collider = nullptr;
    p_rbos.clear();
    p_players = 0;
}
