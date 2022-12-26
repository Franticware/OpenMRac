#ifndef HLIDAC_GAMEMNG_H
#define HLIDAC_GAMEMNG_H

#include "platform.h"

#include "3dm.h"
#include "octopus.h"
#include "matmng.h"
#include "collider.h"
#include "rbsolver.h"
#include "car2d.h"
#include "skysph.h"
#include "gltext.h"
#include "soundmng.h"
#include "settings_dat.h"
//#include "mainmenu.h"
#include "ghost.h"
#include "particles.h"

#include <vector>
#include <memory>
#ifndef __MACOSX__
#include <GL/gl.h>
#else
#include <OpenGL/gl.h>
#endif

#include "glm1.h"

#define STRING_OPTIONS_TITLE   "Options\n\n\n\n"
#define STRING_OPTIONS_LABELS  "\n\nSound Volume:\nView Distance:"
//#define STRING_OPTIONS_ARROW "<            >"
#define STRING_OPTIONS_ARROW "<                      >"
#define STRING_OPTIONS_ARROWS  "\n\n  " STRING_OPTIONS_ARROW "\n  " STRING_OPTIONS_ARROW "\n"

enum Gamemenu_states {
    GMSTATE_RESTART = 0, GMSTATE_TEST, GMSTATE_OPTIONS, GMSTATE_END, GMSTATE_QUIT,
    GMSTATE_T,
    GMSTATE_O_SOUNDVOL, GMSTATE_O_VIEWDIST,
};

extern SDL_Window* gameWindow;

struct Gamemap {
    Gamemap() : light_ah(0), light_av(0), pict_tex(0) { filename[0] = 0; filename_tex[0] = 0; name[0] = 0; }
    //Gamemap(const Gamemap& gamemap) { memcpy(this, &gamemap, sizeof(Gamemap)); }
    char filename[256];
    float light_ah;
    float light_av;

    // textura
    char filename_tex[256];
    char name[256];
    GLuint pict_tex;
};

struct Gameobj {
    Gameobj() : m(0), r(0), f(0) { filename[0] = 0; }
    char filename[256];
    float m; // hmotnost
    float r; // poloměr pro výpočet momentu setrvačnosti a třecí síly
    float f; // tření
    // model a materiály
    std::unique_ptr<T3dm> t3dm;
    std::unique_ptr<Matmng> matmng;
};

struct Car_th {
    Car_th() : tex(0) { fname[0] = 0; }
    GLuint tex; // textura
    float color[4];
    char fname[256]; // jméno souboru s texturou
};

struct Gamecar {
    Gamecar() : engine1_pitch(0) { fname_sample_engine0[0] = 0; fname_sample_engine1[0] = 0; name[0] = 0; }
    char filename[256];
    char filename_cmo[256];
    // model a materiály
    std::vector<std::string> names; // jména pro výměnu původních textur, počet je sz_names*sz_mods
    std::vector<Car_th> pict_tex; // obrázek s texturou do menu, počet je v sz_mods
    unsigned int sz_names;

    char fname_sample_engine0[256];
    char fname_sample_engine1[256];
    float engine1_pitch;

    ALbuffer p_engine0_sample;
    ALbuffer p_engine1_sample;

    float exhaust_position[3];
    float exhaust_direction[3];

    char name[256];
};

struct Gamesky {
    Gamesky() { sky_tex[0] = 0; skycm_tex[0] = 0; }
    char sky_tex[256];
    char skycm_tex[256];
    float light_amb[4];
    float light_diff[4];
    // model a materiály
};

struct Mapobj {
    Mapobj() : vert_i(0), ang(0), id(0), r(0), f(0) { pos[0] = 0; pos[1] = 0; }
    unsigned int vert_i;
    float ang;
    float pos[2];
    unsigned int id;
    float r;
    float f;
    std::unique_ptr<RBSolver> rbo;
    std::unique_ptr<Rendermng> rendermng;
};

struct Playerkeys {
    unsigned char key_left, key_right, key_down, key_up;
};

struct Playerhud {
    Gltext speed;
    Gltext speed_km_h;
    Gltext laptime_l;
    Gltext laptime_r_lap;
    Gltext laptime_l1;
    Gltext laptime_l_best;
    Gltext laptime_r;
    Gltext laptime_r1;
    Gltext newrecord;

    Gltext position;
};

struct Results {
    Gltext title;
    Gltext line0; // Position, time, best lap
    Gltext position;
    Gltext time;
    Gltext best;
};

struct Keytest {
    Gltext player, left, right, down, up;
};

struct Playerstate {
    int lap_i;
    int lap_i_max; // největší zatím dosažená hodnota čísla kola a je min. 1
    float lap_time; // aktualni cas v kole v sekundach
    float best_time; // aktualni cas v kole v sekundach
    float race_time; // aktualni cas v zavode v sekundach

    int state; // 0 - nic, 1 - pred carou, 2 - za carou/na care
    int state_prev;

    int state_finish; // 0 - neskončil, 1 - skončil;
    float finish_time; // až bude finish time u všech > neco.f, Gamemng::p_state = 2 (zobrazení výsledků)
    int player; // <0, 4)

    float position_time;
    int state_position;

};

class Gamemng;
class MainMenu;

class Gamemenu {
public:
    Gamemenu() : bmenu(false), state(0), p_gamemng(0), p_settings(0), p_menu(0)
    {
        for (int i = 0; i != 4*3*2; ++i)
        {
            p_opt_verts[i] = 0;
        }
        for (int i = 0; i != 3; ++i)
        {
            p_opt_color0[i] = 0;
            p_opt_color1[i] = 0;
        }
    }
    void sw();
    void keydown(unsigned int sym);
    void render();
    void init();

    bool bmenu;
    int state;
    Gamemng* p_gamemng;
    Settings* p_settings;
    MainMenu* p_menu;

    Gltext gltext_menu;
    Gltext p_text_opt, p_text_opt2, p_text_opt3;

    float p_opt_verts[4*3*2];
    float p_opt_color0[3];
    float p_opt_color1[3];
};

//typedef float Gamemtrx[16];

class Carcam {
public:
    void init(float r, float y, float ang, float h_ang, const float* , const float* ang0, const float* pos0, const TimeSync* timesync, const Collider* collider); // úhly jsou v radiánech, h_ang ve stupních
    void update(bool bstep);
    glm::mat4 transf();

    const float* p_ang_base;
    const float* p_ang0_base;
    const float* p_pos0_base;
    const TimeSync* p_timesync;
    float p_r;
    float p_r0;
    float p_mindist; // nejmenší vzdálenost od zdi
    float p_y;
    float p_h_ang;

    float p_ang;
    float p_ang0;

    const Collider* p_collider;
};

const char* time_m_s(float time);

class Gamemng {
public:
    Gamemng() :
        p_rbos(0), p_reverse(false), p_players(0),
        p_wide169(false), p_far(0), p_car2do(0), p_car2dp(0), p_cartransf(0), p_carrendermng(0),
        p_ghostmodel(0), p_ghostmatmng(0), p_ghostrendermng(0), p_ghosttransf(0),
        p_isGhost(0), p_ghostUpdated(0), p_ghostAvailable(0),
        p_ghost_time(0), p_finished(0), p_laps(0), p_settings(0)
    {
        for (int i = 0; i != 4; ++i)
        {
            p_carmodel[i] = 0;
            p_carmatmng[i] = 0;
        }
    }
    ~Gamemng()
    {
        unload();
        /*destroy all other: textura slunce*/
        glDeleteTextures(1, &(p_suntex)); checkGL();
        glDeleteTextures(1, &(p_smoketex)); checkGL();
    }
    void unload();
    void init(const char* maps_def, const char* objs_def, const char* cars_def, const char* skies_def); // vytvořit skysph
    void init_sound();
    bool load(int players_sel, const int* cars_sel/*[4]*/, const int* cars_tex_sel, int map_sel, int sky_sel, bool breverse);
    void restart();
    void input(unsigned char keys[4*4]); // předání pole pravdivostních hodnot stisku kláves
    void frame(float deltaT, const glm::mat4& freecam_mtrx);
    void render_frame(const glm::mat4& m);

    void render_black();
    void render_bricks();
    void render_black_background();

    void set_scissor(int player);
    void unset_scissor();
    void set_laps(int laps) { p_laps = laps; }
    void set_proj_mtrx(); // vstupem je p_frust
    void set_far(int far); // 0 - 10
    void gamemenu_sw(bool b_quit = false);
    void end_race();
    void init_keytest();

    void quit_race();
//////////////////////////////////////////////////////
    Playerkeys p_playerkeys[4];

    Skysph p_skysph;
    GLuint p_suntex; // přenosná textura - z init, zrušit v destruktoru
    GLuint p_smoketex; // přenosná textura - z init, zrušit v destruktoru

    GLuint p_skycmtex; // cube map or sphere map

    // skytex spravuje p_skysph
    float p_skyang;

    glm::mat4 p_mtrx_texcm;

    std::vector<Gamemap> p_maps;
    std::vector<Gameobj> p_objs;
    std::vector<Gamecar> p_cars;
    std::vector<Gamesky> p_skies;

    std::vector<Mapobj> p_mapobjs; // objektová mapa

    // model a materiály mapy

    std::unique_ptr<T3dm> p_carmodel[4];
    std::unique_ptr<Matmng> p_carmatmng[4];

    std::unique_ptr<T3dm> p_map_model;
    std::unique_ptr<Matmng> p_map_matmng;
    std::unique_ptr<Octopus> p_map_oct;
    std::unique_ptr<Rendermng> p_map_rendermng;

    std::unique_ptr<Collider> p_collider;
    std::vector<RBSolver*> p_rbos;

    float p_finish[2];
    bool p_reverse;
    unsigned int p_players;

    TimeSync p_timesync;
    TimeSync p_particleTimesync;

    float p_light_position[4];
    float p_light_ambient[4];
    float p_light_diffuse[4];
    //float p_light_ambdiff[4];

    GLint p_viewport[2];
    GLint p_scissor[2];
    float p_aspect;
    float p_frust[4]; // hodnoty abs(left) a abs(top) ve vzdálenosti (-)1, abs(near), abs(far)

    bool p_wide169; // aspect ratio is 16:9

    int p_far;

    int get_far() { return p_far; }

    int p_cars_sel[4];
    std::vector<RBSolver> p_car2do; // pole objektů aut
    std::vector<Car2D> p_car2dp;
    std::vector<Transf> p_cartransf;
    std::vector<Rendermng> p_carrendermng;
    std::vector<Particles> p_particles;

    std::vector<T3dm> p_ghostmodel; // pole
    std::vector<Matmng> p_ghostmatmng; // pole
    std::vector<Rendermng> p_ghostrendermng; // pole
    std::vector<Transf> p_ghosttransf; // pole

    std::unique_ptr<Ghost> p_ghostOld;
    std::vector<Ghost> p_ghostNew; // pole 4 prvků
    int p_isGhost; // times 1 (now only used for rendering)
    int p_ghostUpdated; // times 1
    int p_ghostAvailable; // times 1
    int p_ghost_step[4]; // times 4
    float p_ghost_wheel_rot; // times 1 (only used for rendering)
    float p_ghost_x_prev[2]; // times 1 (used for computing wheel rotation)
    float p_newlaprecordtxttime[4]; // times 4
    float p_ghost_time; // times 1 (?)

    float p_wheel_rot[4];

    Carcam p_carcam[4];

    glm::mat4 p_proj_mtrx[4];
    glm::mat4 p_proj_mtrx0;
    glm::mat4 p_proj_mtrx_active;

    Glfont p_glfont;
    float p_fpscoord[2];
    Gltext p_gltext_fps;

    Playerhud p_playerhud[4];
    void init_hud();

    Playerstate p_playerstate[4];

    int p_finished;

    int p_laps;

    int p_state; // 0 - uvodni kamera, 1 - hra, 2 - konec zavodu
    float p_state0_time;
    float p_state0_5_time; // GO!
    bool p_state0_5;
    Gltext p_gltext_start;
    Carcam p_startcam[4];

    GLuint p_fonttex;

    std::unique_ptr<Sound_crash> p_sound_crash;
    std::vector<Sound_car> p_sound_car;

    float p_global_volume;

    int p_global_volume_i;

    int get_global_volume() { return p_global_volume_i; }
    void set_global_volume(int volume) { if (volume < 0) volume = 0; else if (volume > 100) volume = 100;
        p_global_volume_i = volume; p_global_volume = p_global_volume_i*0.01f; }


    Gamemenu p_gamemenu;
    bool p_bfps;

    Results p_results;

    //unsigned int* p_keysym;
    Keytest p_keytest[4];

    Sound_game_static p_sound_game_static;

    Settings* p_settings;

};

#endif
