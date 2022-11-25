#ifndef HLIDAC_MAINMENU_H
#define HLIDAC_MAINMENU_H

#include "gamemng.h"
#include "gltext.h"
#include "settings_dat.h"

#include <SDL2/SDL.h>

enum MainMenu_states {
    STATE_RACE = 0, STATE_CONTROLS, STATE_OPTIONS, STATE_BEST_LAPS, STATE_QUIT, STATE_Q,

    STATE_R_NUM, STATE_R_CAR1, STATE_R_CAR2, STATE_R_CAR3, STATE_R_CAR4,
        STATE_R_TRACK, STATE_R_LAPS, STATE_R_DIRECTION, STATE_R_DAYTIME, STATE_R_GO,

    STATE_CONTROLS_BEGIN,
    STATE_CONTROLS_TEST_KEYBOARD = STATE_CONTROLS_BEGIN,
    STATE_CONTROLS_P1_UP, STATE_CONTROLS_P1_DOWN, STATE_CONTROLS_P1_LEFT, STATE_CONTROLS_P1_RIGHT,
    STATE_CONTROLS_P2_UP, STATE_CONTROLS_P2_DOWN, STATE_CONTROLS_P2_LEFT, STATE_CONTROLS_P2_RIGHT,
    STATE_CONTROLS_P3_UP, STATE_CONTROLS_P3_DOWN, STATE_CONTROLS_P3_LEFT, STATE_CONTROLS_P3_RIGHT,
    STATE_CONTROLS_P4_UP, STATE_CONTROLS_P4_DOWN, STATE_CONTROLS_P4_LEFT, STATE_CONTROLS_P4_RIGHT,
    STATE_CONTROLS_END,

    STATE_CONTROLS_TEST_KEYBOARD_SCREEN = STATE_CONTROLS_END,

    STATE_OPTIONS_SOUNDVOL, STATE_OPTIONS_VIEWDIST,

    STATE_BEST_LAPS_SCREEN,

    STATE_CAR_SEL,

    STATE_TRACK_SEL,
};

class Gamemng;


#if defined(__amigaos4__)
#define Menu Menu_
#endif

class MainMenu {
public:
    MainMenu() : p_bactive(false), p_state(0), p_car_i(0), p_gamemng(0), p_settings(0), p_controlsChanged(false), p_players(0), p_laps(0),
        p_direction_sel(false), p_sky_sel(0), p_sound_vol(0), p_view_dist(0), p_track_sel(0), p_testKeysCount(0), p_enterMode(false) { }
    ~MainMenu() { }
    void init(Gamemng* gamemng, Settings* settings);
    void menu(); // nahrát textury a změnit stav
    void game(); // uvolnit textury a změnit stav
    void save_settings();
    bool keydown(unsigned int sym);
    void event(const SDL_Event& e);
    void afterEvent();
    void loadAndPrintBestTimes();
    void render(); // vykreslení menu

    void exitEnterMode();

    bool p_bactive; // na začátku false, na true se změní po zavolání menu()
    int p_state; // = 0
    int p_car_sel[4]; //
    int p_car_i; // <0, 4), číslo auta u stavu STATE_CAR_SEL
    int p_car_tex_sel[4]; //

    Gamemng* p_gamemng; // pomocí tohoto se získá ukazatel na glfont
    Settings* p_settings;

    bool p_controlsChanged;

    GLuint p_logo_textura;

    // další struktura na textury aut, možná propojená s gamemng, to samé tratě
    Gltext p_text_main;
    Gltext p_text_main_ver;
    Gltext p_text_main_www;
    Gltext p_text_race_go, p_text_race_n, p_text_race_car, p_text_race_car2,
        p_text_race_track0, p_text_race_track1, p_text_race_track24, p_text_race_track35;//, p_text_race_laps;
    Gltext p_text_race_status;
    Gltext p_text_test_keyboard, p_text_test_keyboard_description, p_text_test_keyboard_status;
    Gltext p_text_controls, p_text_controls0, p_text_controls1, p_text_controls2, p_text_controls_status, p_text_controls_status_enter;
    Gltext p_text_opt, p_text_opt2, p_text_opt3;
    Gltext p_text_carsel;

    Gltext p_text_best_laps;
    Gltext p_text_best_laps_header;
    Gltext p_text_best_laps_tracks;
    Gltext p_text_best_laps_tracks_reversed_flag;
    Gltext p_text_best_laps_cars;
    Gltext p_text_best_laps_times;

    float p_opt_verts[4*3*2];
    float p_opt_color0[3];
    float p_opt_color1[3];

    unsigned int p_players; // 1 - 4
    unsigned int p_laps; // 1 - 50
    bool p_direction_sel;
    int p_daytime_sel;
    int p_sky_sel;
    int p_sound_vol;
    int p_view_dist;

    int p_cars_sel[4];
    int p_cars_tex_sel[4];

    int p_track_sel;

    int p_testKeysCount;
    SDL_Keycode p_testKeysLastKeyDown;
    SDL_Keycode p_testKeysLastKeyUp;

    bool p_enterMode;

    GLuint p_tex_sel_bnd; // pracovní proměnná pro aktivní texturu výběru

    static const float font_color[4];
    static const float font_color_disconnected[4];
    static const float font_color_ver[4];
    static const float font_color_hi[4];
    static const float font_color_enter[4];

};

#endif // HLIDAC_MAINMENU_H

