#include "mainmenu.h"
#include "load_texture.h"
#include "gbuff_in.h"
#include "rand1.h"
#include "appdefs.h"

#include <SDL2/SDL.h>

void my_exit(int ret, bool callExit);

/*static*/ const float MainMenu::font_color[4] = {0.6, 0.6, 0.6, 1};
/*static*/ const float MainMenu::font_color_disconnected[4] = {0.4, 0.4, 0.4, 1};
/*static*/ const float MainMenu::font_color_ver[4] = {0.8, 0.8, 0.8, 1};
/*static*/ const float MainMenu::font_color_hi[4] = {1, 1, 1, 1};
/*static*/ const float MainMenu::font_color_enter[4] = {1, 0.2f, 0.2f, 1};

void MainMenu::init(Gamemng* gamemng, Settings* settings)
{
    p_gamemng = gamemng;
    p_settings = settings;

    p_text_main.init(12, 6, 2.5f, 0, 0, &(p_gamemng->p_glfont), font_color);
    p_text_main.set_pos(0.f, -4.5f - 0.6f);
    p_text_main.puts(0, "Race\nControls\nOptions\nBest Laps\nQuit");

    p_text_main.set_color(0, font_color_hi);

    p_text_main_www.init(70, 1, /*0.8f*/ 1.2f, 0, 0, &(p_gamemng->p_glfont), font_color_hi);
    //p_text_main_www.set_pos(0.f, -14.f + 0.5f);
    p_text_main_www.set_pos(0.f, -14.f + 1.0f);
    p_text_main_www.puts(0, "github.com/Franticware/OpenMRac");
    p_text_main_ver.init(15, 1, 0.8f, -1, 0, &(p_gamemng->p_glfont), font_color);
    p_text_main_ver.set_pos(9.f, 4.f - 0.7f);
    p_text_main_ver.puts(0, OPENMRAC_VERSION);

    p_text_main_ver.set_color(0, font_color_ver);

    p_text_race_n.init(40, 1, 2.f, 0, 0, &(p_gamemng->p_glfont), font_color);
    p_text_race_n.set_pos(0.f, 10.f+1);
    p_text_race_n.puts(0, "Num. of players: <  3  >");


    p_text_race_car.init(40, 4, 2.f, -1, 0, &(p_gamemng->p_glfont), font_color);
    p_text_race_car.set_pos(-1.5f, 5.f+1);
    p_text_race_car.puts(0,
        "Car 1:\n"
        "Car 2:\n"
        "Car 3:\n"
        "Car 4:"
    );

    p_text_race_car2.init(40, 4, 2.f, 1, 0, &(p_gamemng->p_glfont), font_color);
    p_text_race_car2.set_pos(0.f, 5.f+1);
    p_text_race_car2.puts(0,
        "Trb\n"
        "Skd\n"
        "Kfr\n"
        "Trb"
    );


    p_text_race_track0.init(40, 1, 2.f, -1, 0, &(p_gamemng->p_glfont), font_color);
    p_text_race_track0.set_pos(-2.5f, -2.f+1);
    p_text_race_track0.puts(0, "Track:");

    p_text_race_track1.init(40, 1, 2.f, 1, 0, &(p_gamemng->p_glfont), font_color);
    p_text_race_track1.set_pos(-1.f, -2.f+1);
    p_text_race_track1.puts(0, "Speedway");

    p_text_race_track24.init(40, 3, 2.f, -1, 0, &(p_gamemng->p_glfont), font_color);
    p_text_race_track24.set_pos(-2.5f, -5.f);
    p_text_race_track24.puts(0, "Laps:\nDirection:\nDaytime:");

    p_text_race_track35.init(40, 3, 2.f, 1, 0, &(p_gamemng->p_glfont), font_color);
    p_text_race_track35.set_pos(-1.f, -5.f);
    p_text_race_track35.puts(0, "<  10  >\n<  Regular  >\n<  Day  >");


    p_text_race_go.init(4, 1, 2.5f, 0, 0, &(p_gamemng->p_glfont), font_color);
    p_text_race_go.set_pos(0.f, -10.f);
    p_text_race_go.puts(0, "GO!");

    p_text_race_status.init(256, 1, 0.8f, 0, 0, &(p_gamemng->p_glfont), font_color);
    p_text_race_status.set_pos(0.f, -13.f);
    p_text_race_status.puts(0, "initial text");
    p_text_race_status.set_color(font_color_hi);

    p_text_test_keyboard.init(40, 1, 2.f, 0, 0, &(p_gamemng->p_glfont), font_color);
    p_text_test_keyboard.set_pos(0.f, 10.f);
    p_text_test_keyboard.puts(0, "Test Keyboard");

    p_text_test_keyboard_description.init(120, 5, 0.8f, 1, -1, &(p_gamemng->p_glfont), font_color);
    p_text_test_keyboard_description.set_pos(-18.f, 8.3f);
    p_text_test_keyboard_description.puts(0,
        "This tool can be used for finding a valid key combination for your particular\n"
        "keyboard. Most keyboards can detect and handle correctly only a limited amount\n"
        "of keys pressed at the same time. Some keys, such as NumLock or PrtScr, are\n"
        "not detected correctly by the game and should not be used at all (screenshots\n"
        "can be taken by pressing F12 (or "
#ifdef __MACOSX__
        "Cmd/"
#endif
        "Ctrl-F12) and are saved as TGA images).");

    p_text_test_keyboard_status.init(120, 6, 1.1f, 1, -1, &(p_gamemng->p_glfont), font_color);
    p_text_test_keyboard_status.set_pos(-7.5f, 3.f);
    p_text_test_keyboard_status.puts(0,
        "Detected n pressed keys.\n\n"
        "Last pressed key:\n"
        "b\n"
        "Last released key:\n"
        "a");
    p_text_test_keyboard_status.set_color(0, font_color_hi);

    p_text_controls.init(40, 1, 2.f, 0, 0, &(p_gamemng->p_glfont), font_color);
    p_text_controls.set_pos(0.f, 13.f);
    p_text_controls.puts(0, "Controls");

    p_text_best_laps.init(40, 1, 2.f, 0, -1, &(p_gamemng->p_glfont), font_color);
    p_text_best_laps.set_pos(0.f, 12.f);
    p_text_best_laps.puts(0, "Best Laps");

    p_text_best_laps_header.init(60, 1, 2.f, -1, -1, &(p_gamemng->p_glfont), font_color_hi);
    p_text_best_laps_header.set_pos(19.5f, 9.5f);
    p_text_best_laps_header.puts(0, "Track                     Car           Lap Time");

    p_text_best_laps_tracks.init(40, 8, 2.f, -1, -1, &(p_gamemng->p_glfont), font_color_hi);
    p_text_best_laps_tracks.set_pos(-8.f, 7.5f);

    p_text_best_laps_tracks_reversed_flag.init(40, 8, 2.f, 1, -1, &(p_gamemng->p_glfont), font_color_hi);
    p_text_best_laps_tracks_reversed_flag.set_pos(-7.0f, 7.5f);

    for (unsigned i = 0; i != p_gamemng->p_maps.size(); ++i)
    {
        for (unsigned j = 0; j != 2; ++j)
        {
            p_text_best_laps_tracks.puts(i * 2 + j, p_gamemng->p_maps[i].name);
            p_text_best_laps_tracks_reversed_flag.puts(i * 2 + j, j == 0 ? "" : "rev.");
        }
    }

    p_text_best_laps_cars.init(40, 8, 2.f, 0, -1, &(p_gamemng->p_glfont), font_color_hi);
    p_text_best_laps_cars.set_pos(4.0f, 7.5f);
    p_text_best_laps_cars.puts(0, "");

    p_text_best_laps_times.init(40, 8, 2.f, -1, -1, &(p_gamemng->p_glfont), font_color_hi);
    p_text_best_laps_times.set_pos(19.5f, 7.5f);
    p_text_best_laps_times.puts(0, "");

    p_text_controls0.init(40, 21, 1.1f, 0, 0, &(p_gamemng->p_glfont), font_color);
    p_text_controls0.set_pos(0.f, 0.f);
    p_text_controls0.puts(0, "Test Keyboard");

    p_text_controls1.init(40, 21, 1.1f, -1, 0, &(p_gamemng->p_glfont), font_color);
    p_text_controls1.set_pos(-12.f, 0.f);
    p_text_controls1.puts(0,
        "\n"
        "Player 1:    \n"
        "Accelerate\n"
        "Brake\n"
        "Left\n"
        "Right\n"
        "Player 2:    \n"
        "Accelerate\n"
        "Brake\n"
        "Left\n"
        "Right\n"
        "Player 3:    \n"
        "Accelerate\n"
        "Brake\n"
        "Left\n"
        "Right\n"
        "Player 4:    \n"
        "Accelerate\n"
        "Brake\n"
        "Left\n"
        "Right\n"
        );

    p_text_controls2.init(100, 21, 1.1f, 1, 0, &(p_gamemng->p_glfont), font_color);
    p_text_controls2.set_pos(-11.f, 0.f);
    p_text_controls2.puts(0,
        "\n"
        "\n"
        "Accelerate\n"
        "\?\?\?\n"
        "Left\n"
        "Right\n"
        "\n"
        "Accelerate\n"
        "Brake\n"
        "Left\n"
        "Right\n"
        "\n"
        "Accelerate\n"
        "Brake\n"
        "Left\n"
        "Right\n"
        "\n"
        "Accelerate\n"
        "Brake\n"
        "Left\n"
        "Right\n"
        );

    p_text_controls_status.init(256, 3, 0.8f, 0, 0, &(p_gamemng->p_glfont), font_color);
    p_text_controls_status.set_pos(0.f, -13.f);

    p_text_controls_status.puts(0,
        "Select an action to config, then press Return/Enter (before assigning joystick\naxis/hat, it might help to move it around a bit)."
        " Press Delete to remove control.\n"
        "Exit the game before connecting/disconnecting any joysticks.");
    p_text_controls_status.set_color(font_color_hi);

    p_text_controls_status_enter.init(256, 2, 0.8f, 0, 0, &(p_gamemng->p_glfont), font_color);
    p_text_controls_status_enter.set_pos(0.f, -13.f);
    p_text_controls_status_enter.puts(0, "Press a button or move an analog control slowly to the boundary and back.\nPress Escape to cancel.");
    p_text_controls_status_enter.set_color(font_color_enter);

    p_text_opt.init(40, 5, 2.f, 0, 0, &(p_gamemng->p_glfont), font_color);
    p_text_opt.set_pos(0.f, 0.f);
    p_text_opt.puts(0,
        STRING_OPTIONS_TITLE
    );

    p_text_opt2.init(40, 5, 2.f, -1, 0, &(p_gamemng->p_glfont), font_color);
    p_text_opt2.set_pos(0.f, 0.f);
    p_text_opt2.puts(0, STRING_OPTIONS_LABELS);

    p_text_opt3.init(40, 5, 2.f, 1, 0, &(p_gamemng->p_glfont), font_color);
    p_text_opt3.set_pos(0.f, 0.f);
    p_text_opt3.puts(0, STRING_OPTIONS_ARROWS);

#define leftRightSel     "<                             >\n"

    p_text_carsel.init(40, 10, 2.f, 0, 0, &(p_gamemng->p_glfont), font_color);
    p_text_carsel.set_pos(0.f, 0.f);
    p_text_carsel.puts(0,
        "Select player 1 car\n"
        "\n"
        "\n"
        "\n"
        leftRightSel
        "\n"
        "\n"
        "\n"
        "Skd"
        "\n"
    );
    p_text_carsel.set_color(4, font_color_hi);

    // 24 prvků
    float ycmin = -0.7f*0.319f, ycmax = 0.7f*0.319f;
    float xcmin = 0.f*0.319f, xcmax = 8.f*0.319f;
    float xposun = 4.3f*0.319f;
    p_opt_verts[0] = xcmin+xposun;
    p_opt_verts[1] = ycmin;
    p_opt_verts[2] = -10;

    p_opt_verts[3] = xcmax+xposun;
    p_opt_verts[4] = ycmin;
    p_opt_verts[5] = -10;

    p_opt_verts[6] = xcmax+xposun;
    p_opt_verts[7] = ycmax;
    p_opt_verts[8] = -10;

    p_opt_verts[9] = xcmin+xposun;
    p_opt_verts[10] = ycmax;
    p_opt_verts[11] = -10;

    ///////////////////////////
    float yposun = -2.f*0.319f;

    p_opt_verts[12] = p_opt_verts[0];
    p_opt_verts[13] = p_opt_verts[1]+yposun;
    p_opt_verts[14] = p_opt_verts[2];

    p_opt_verts[15] = p_opt_verts[3];
    p_opt_verts[16] = p_opt_verts[4]+yposun;
    p_opt_verts[17] = p_opt_verts[5];

    p_opt_verts[18] = p_opt_verts[6];
    p_opt_verts[19] = p_opt_verts[7]+yposun;
    p_opt_verts[20] = p_opt_verts[8];

    p_opt_verts[21] = p_opt_verts[9];
    p_opt_verts[22] = p_opt_verts[10]+yposun;
    p_opt_verts[23] = p_opt_verts[11];

    menu();
}

void MainMenu::save_settings()
{
    p_settings->set("last_players", p_players);
    if (p_laps < 1) p_laps = 1;
    p_settings->set("last_laps", p_laps);
    if (p_sound_vol > 100) p_sound_vol = 100;
    p_settings->set("sound_volume", p_sound_vol);
    if (p_view_dist > 10) p_view_dist = 10;
    p_settings->set("view_distance", p_view_dist);
    p_settings->set("last_daytime", p_daytime_sel);
    p_settings->set("last_direction", p_direction_sel ? 1 : 0);
    // last_cars
    p_settings->set("last_carsel1", p_cars_sel[0]);
    p_settings->set("last_carsel2", p_cars_sel[1]);
    p_settings->set("last_carsel3", p_cars_sel[2]);
    p_settings->set("last_carsel4", p_cars_sel[3]);
    p_settings->set("last_cartex1", p_cars_tex_sel[0]);
    p_settings->set("last_cartex2", p_cars_tex_sel[1]);
    p_settings->set("last_cartex3", p_cars_tex_sel[2]);
    p_settings->set("last_cartex4", p_cars_tex_sel[3]);
    p_settings->set("last_track", p_track_sel);
}

void MainMenu::menu()
{
    if (p_bactive) // už je aktivní
        return;
    //SDL_EnableKeyRepeat(500, 30);
    p_bactive = true;

    p_state = STATE_RACE;

    p_players = p_settings->get("last_players");
    p_laps = p_settings->get("last_laps");
    if (p_laps < 1) p_laps = 1;
    p_sound_vol = p_settings->get("sound_volume");
    p_view_dist = p_settings->get("view_distance");

    p_direction_sel = p_settings->get("last_direction");
    p_daytime_sel = p_settings->get("last_daytime");

    p_sky_sel = randn1(2);
    if (p_daytime_sel == 1)
        p_sky_sel += 2;

    p_cars_sel[0] = p_settings->get("last_carsel1");
    p_cars_sel[1] = p_settings->get("last_carsel2");
    p_cars_sel[2] = p_settings->get("last_carsel3");
    p_cars_sel[3] = p_settings->get("last_carsel4");
    p_cars_tex_sel[0] = p_settings->get("last_cartex1");
    p_cars_tex_sel[1] = p_settings->get("last_cartex2");
    p_cars_tex_sel[2] = p_settings->get("last_cartex3");
    p_cars_tex_sel[3] = p_settings->get("last_cartex4");
    for (int i = 0; i != 4; ++i)
    {
        if (p_cars_sel[i] >= int(p_gamemng->p_cars.size())) p_cars_sel[i] = int(p_gamemng->p_cars.size())-1;
        if (p_cars_sel[i] < 0) p_cars_sel[i] = 0;
        if (p_cars_tex_sel[i] >= int(p_gamemng->p_cars[p_cars_sel[i]].pict_tex.size())) p_cars_tex_sel[i] = int(p_gamemng->p_cars[p_cars_sel[i]].pict_tex.size())-1;
        if (p_cars_tex_sel[i] < 0) p_cars_tex_sel[i] = 0;
    }
    p_track_sel = p_settings->get("last_track");
    if (p_track_sel >= int(p_gamemng->p_maps.size())) p_track_sel = int(p_gamemng->p_maps.size())-1;
    if (p_track_sel < 0) p_track_sel = 0;

    // tady načíst všechny textury

    Pict2 pictlogo_rgba;
    gbuff_in.f_open("logo-silver-d.png", "rb");
    pictlogo_rgba.loadpng(gbuff_in.fbuffptr(), gbuff_in.fbuffsz());
    gbuff_in.fclose();

    Pict2 pictlogo_a;
    gbuff_in.f_open("logo-silver-a.png", "rb");
    pictlogo_a.loadpng(gbuff_in.fbuffptr(), gbuff_in.fbuffsz());
    gbuff_in.fclose();
    pictlogo_rgba.r2a(pictlogo_a);
    p_logo_textura = load_texture(pictlogo_rgba);

    //
    for (unsigned int i = 0; i != p_gamemng->p_cars.size(); ++i)
    {
        for (unsigned int j = 0; j != p_gamemng->p_cars[i].pict_tex.size(); ++j)
        {
            Pict2 pict_th;
            gbuff_in.f_open(p_gamemng->p_cars[i].pict_tex[j].fname, "rb");
            pict_th.loadjpeg(gbuff_in.fbuffptr(), gbuff_in.fbuffsz());
            gbuff_in.fclose();
            p_gamemng->p_cars[i].pict_tex[j].tex = load_texture(pict_th);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); checkGL(); // textura se neopakuje
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); checkGL(); // textura se neopakuje
        }
    }
    for (unsigned int i = 0; i != p_gamemng->p_maps.size(); ++i)
    {
        Pict2 pict_th;
        gbuff_in.f_open(p_gamemng->p_maps[i].filename_tex, "rb");
        pict_th.loadjpeg(gbuff_in.fbuffptr(), gbuff_in.fbuffsz());
        gbuff_in.fclose();
        p_gamemng->p_maps[i].pict_tex = load_texture(pict_th);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); checkGL(); // textura se neopakuje
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); checkGL(); // textura se neopakuje
    }
}

void MainMenu::game()
{
    if (!p_bactive) // už je aktivní
        return;
    //SDL_EnableKeyRepeat(0, 0);
    p_bactive = false;

    // všechny textury smazat
    p_logo_textura = 0;

    for (unsigned int i = 0; i != p_gamemng->p_cars.size(); ++i)
        for (unsigned int j = 0; j != p_gamemng->p_cars[i].pict_tex.size(); ++j)
        {
            p_gamemng->p_cars[i].pict_tex[j].tex = 0;
        }
    for (unsigned int i = 0; i != p_gamemng->p_maps.size(); ++i)
    {
        p_gamemng->p_maps[i].pict_tex = 0;
    }

    if (!p_gamemng->load(p_players, p_cars_sel, p_cars_tex_sel, p_track_sel, p_sky_sel, p_direction_sel))
    {
        fprintf(stderr, "Error loading game data.\n");
        my_exit(1, true);
    }

    p_gamemng->p_laps = p_laps;
    p_gamemng->set_far(p_view_dist);
    p_gamemng->set_global_volume(p_sound_vol);

    save_settings();
}

void MainMenu::render() // vykreslení menu
{
    p_gamemng->p_shadermng.set(ShaderUniMat4::ModelViewMat, glm::mat4(1));
    p_gamemng->unset_scissor();
    glDisable(GL_BLEND); checkGL();
    glDisable(GL_DEPTH_TEST); checkGL();
    glVertexAttrib4f((GLuint)ShaderAttrib::Color, 1,1,1,1); checkGL();

    if (!(p_state >= STATE_CONTROLS_BEGIN && p_state < STATE_CONTROLS_END) &&
        p_state != STATE_CONTROLS_TEST_KEYBOARD_SCREEN)
    {
        p_gamemng->render_bricks();
    }
    else
    {
        p_gamemng->render_black_background();
    }

    glEnable(GL_BLEND); checkGL();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); checkGL();

    // tady kreslit hlavní menu
    if (p_state >= STATE_RACE && p_state <= STATE_Q)
    {
        p_gamemng->p_shadermng.use(ShaderId::Tex);

        glBindTexture(GL_TEXTURE_2D, p_logo_textura); checkGL();
        static const float seda = 1.f;
        // vykreslení obrázku loga
        glVertexAttrib4f((GLuint)ShaderAttrib::Color, seda, seda, seda,1); checkGL();
        glEnableVertexAttribArray((GLuint)ShaderAttrib::Pos); checkGL();
        glEnableVertexAttribArray((GLuint)ShaderAttrib::Tex); checkGL();
        float y_coord = 2.8 - 0.2;
        static const float vert_array[12] = {-4, -2+y_coord, -10,
                                             4, -2+y_coord, -10,
                                             -4,  2+y_coord, -10,
                                             4,  2+y_coord, -10,
                                            };
        float texc_size = 1;
        static const float texc_array[8] = {0, 0,
                                            texc_size, 0,
                                            0,  texc_size,
                                            texc_size,  texc_size,
                                           };
        glVertexAttribPointer((GLuint)ShaderAttrib::Pos, 3, GL_FLOAT, GL_FALSE, 0, vert_array); checkGL();
        glVertexAttribPointer((GLuint)ShaderAttrib::Tex, 2, GL_FLOAT, GL_FALSE, 0, texc_array); checkGL();
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); checkGL();
        glEnableVertexAttribArray((GLuint)ShaderAttrib::Pos); checkGL();
        glEnableVertexAttribArray((GLuint)ShaderAttrib::Tex); checkGL();

        glVertexAttrib4f((GLuint)ShaderAttrib::Color, 1,1,1,1); checkGL();
        p_text_main.render_c(&p_gamemng->p_shadermng);

        p_text_main_ver.render_c(&p_gamemng->p_shadermng);
        p_text_main_www.render_c(&p_gamemng->p_shadermng);
    }
    else if (p_state >= STATE_R_NUM && p_state <= STATE_R_GO)
    {
        p_text_race_go.render_c(&p_gamemng->p_shadermng);
        p_text_race_n.render_c(&p_gamemng->p_shadermng);
        p_text_race_car.render_c(&p_gamemng->p_shadermng);
        p_text_race_car2.render_c(&p_gamemng->p_shadermng);
        p_text_race_track0.render_c(&p_gamemng->p_shadermng);
        p_text_race_track1.render_c(&p_gamemng->p_shadermng);
        p_text_race_track24.render_c(&p_gamemng->p_shadermng);
        p_text_race_track35.render_c(&p_gamemng->p_shadermng);

        p_text_race_status.render_c(&p_gamemng->p_shadermng);
    }
    else if (p_state >= STATE_CONTROLS_BEGIN && p_state < STATE_CONTROLS_END)
    {
        p_text_controls.render_c(&p_gamemng->p_shadermng);
        p_text_controls0.render_c(&p_gamemng->p_shadermng);
        p_text_controls1.render_c(&p_gamemng->p_shadermng);
        p_text_controls2.render_c(&p_gamemng->p_shadermng);

        if (p_state >= STATE_CONTROLS_P1_UP && p_state <= STATE_CONTROLS_P4_RIGHT && p_enterMode)
        {
            p_text_controls_status_enter.render_c(&p_gamemng->p_shadermng);
        }
        else
        {
            p_text_controls_status.render_c(&p_gamemng->p_shadermng);
        }
    }
    else if (p_state == STATE_CONTROLS_TEST_KEYBOARD_SCREEN)
    {
        p_text_test_keyboard.render_c(&p_gamemng->p_shadermng);
        p_text_test_keyboard_description.render_c(&p_gamemng->p_shadermng);
        p_text_test_keyboard_status.render_c(&p_gamemng->p_shadermng);
    }
    else if (p_state == STATE_BEST_LAPS_SCREEN)
    {
        p_text_best_laps.render_c(&p_gamemng->p_shadermng);
        p_text_best_laps_header.render_c(&p_gamemng->p_shadermng);
        p_text_best_laps_tracks.render_c(&p_gamemng->p_shadermng);
        p_text_best_laps_tracks_reversed_flag.render_c(&p_gamemng->p_shadermng);
        p_text_best_laps_cars.render_c(&p_gamemng->p_shadermng);
        p_text_best_laps_times.render_c(&p_gamemng->p_shadermng);
    }
    else if (p_state >= STATE_OPTIONS_SOUNDVOL && p_state <= STATE_OPTIONS_VIEWDIST)
    {
        static const GLushort quad0[6] = { 0, 1, 2, 0, 2, 3 };
        static const GLushort quad1[6] = { 4, 5, 6, 4, 6, 7 };

        p_text_opt.render_c(&p_gamemng->p_shadermng);
        p_text_opt2.render_c(&p_gamemng->p_shadermng);
        p_text_opt3.render_c(&p_gamemng->p_shadermng);

        p_gamemng->p_shadermng.set(ShaderUniMat4::ModelViewMat, glm::mat4(1));
        p_gamemng->p_shadermng.use(ShaderId::Color);

        glDisable(GL_BLEND); checkGL();
        glEnableVertexAttribArray((GLuint)ShaderAttrib::Pos); checkGL();
        glVertexAttribPointer((GLuint)ShaderAttrib::Pos, 3, GL_FLOAT, GL_FALSE, 0, p_opt_verts); checkGL();
        glVertexAttrib3fv((GLuint)ShaderAttrib::Color, p_opt_color0); checkGL();
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, quad0); checkGL();
        glVertexAttrib3fv((GLuint)ShaderAttrib::Color, p_opt_color1); checkGL();
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, quad1); checkGL();
        glDisableVertexAttribArray((GLuint)ShaderAttrib::Pos); checkGL();
    }
    else if (p_state == STATE_CAR_SEL)
    {
        p_gamemng->p_shadermng.set(ShaderUniMat4::ModelViewMat, glm::mat4(1));
        p_gamemng->p_shadermng.use(ShaderId::ColorTex);
        p_gamemng->p_shadermng.set(ShaderUniInt::AlphaDiscard, (GLint)0);

        glDisable(GL_BLEND); checkGL();
        glBindTexture(GL_TEXTURE_2D, p_tex_sel_bnd); checkGL();
        // vykreslení obrázku loga
        glVertexAttrib4f((GLuint)ShaderAttrib::Color, 1,1,1,1); checkGL();
        glEnableVertexAttribArray((GLuint)ShaderAttrib::Pos); checkGL();
        glEnableVertexAttribArray((GLuint)ShaderAttrib::Tex); checkGL();
        float y_coord = 1.f*0.319f;
        float size = 0.5f*0.319f;
        static const float vert_array[12] = {-16*size, -10*size+y_coord, -10,
                                             16*size, -10*size+y_coord, -10,

                                             -16*size,  10*size+y_coord, -10,
                                             16*size,  10*size+y_coord, -10,
                                            };
        static const float texc_array[8] = {0, 0,
                                            1, 0,

                                            0,  1,
                                            1,  1,
                                           };

        glVertexAttribPointer((GLuint)ShaderAttrib::Pos, 3, GL_FLOAT, GL_FALSE, 0, vert_array); checkGL();
        glVertexAttribPointer((GLuint)ShaderAttrib::Tex, 2, GL_FLOAT, GL_FALSE, 0, texc_array); checkGL();

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); checkGL();
        glDisableVertexAttribArray((GLuint)ShaderAttrib::Pos); checkGL();
        glDisableVertexAttribArray((GLuint)ShaderAttrib::Tex); checkGL();
        glEnable(GL_BLEND); checkGL();

        p_text_carsel.render_c(&p_gamemng->p_shadermng);
        glVertexAttrib3f((GLuint)ShaderAttrib::Color, 1,1,1); checkGL();

        glEnable(GL_BLEND); checkGL();

        {
            glEnableVertexAttribArray((GLuint)ShaderAttrib::Pos); checkGL();
            glEnableVertexAttribArray((GLuint)ShaderAttrib::Tex); checkGL();

            float size = 1;
            float yposun0 = (1+6);
            float yposun1 = (1-6);

            static const float vert_array[24] = {-1*size, -0.506316f*size+yposun0, -10, 1*size, -0.546316f*size+yposun0, -10, 1*size, 0.546316f*size+yposun0, -10, -1*size, 0.506316f*size+yposun0, -10,
                -1*size, -0.506316f*size+yposun1, -10, 1*size, -0.546316f*size+yposun1, -10, 1*size, 0.546316f*size+yposun1, -10, -1*size, 0.506316f*size+yposun1, -10
                };
            static const float texc_array[16] = {
                0.875, 0.28125,
                0.875, 0.148438,
                0.914063, 0.148438,
                0.914063, 0.28125,

                0.914063, 0.28125,
                0.914063, 0.148438,
                0.875, 0.148438,
                0.875, 0.28125,

                };

            static const GLushort ind_array[12] = {
                0,1,2, 0,2,3,
                4,5,6, 4,6,7
            };

            glVertexAttribPointer((GLuint)ShaderAttrib::Pos, 3, GL_FLOAT, GL_FALSE, 0, vert_array); checkGL();
            glVertexAttribPointer((GLuint)ShaderAttrib::Tex, 2, GL_FLOAT, GL_FALSE, 0, texc_array); checkGL();
            glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_SHORT, ind_array); checkGL();

            glDisableVertexAttribArray((GLuint)ShaderAttrib::Pos); checkGL();
            glDisableVertexAttribArray((GLuint)ShaderAttrib::Tex); checkGL();
        }

    }
    else if (p_state == STATE_TRACK_SEL)
    {
        p_gamemng->p_shadermng.set(ShaderUniMat4::ModelViewMat, glm::mat4(1));
        p_gamemng->p_shadermng.use(ShaderId::ColorTex);
        p_gamemng->p_shadermng.set(ShaderUniInt::AlphaDiscard, (GLint)0);

        glDisable(GL_BLEND); checkGL();
        glBindTexture(GL_TEXTURE_2D, p_tex_sel_bnd); checkGL();
        glVertexAttrib4f((GLuint)ShaderAttrib::Color, 1,1,1,1); checkGL();
        glEnableVertexAttribArray((GLuint)ShaderAttrib::Pos); checkGL();
        glEnableVertexAttribArray((GLuint)ShaderAttrib::Tex); checkGL();
        float y_coord = 1.f*0.319f;
        float size = 0.5f*0.319f;
        static const float vert_array[12] = {-16*size, -10*size+y_coord, -10,
                                             16*size, -10*size+y_coord, -10,
                                             -16*size,  10*size+y_coord, -10,
                                             16*size,  10*size+y_coord, -10,
                                            };
        static const float texc_array[8] = {0, 0,
                                            1, 0,
                                            0,  1,
                                            1,  1,
                                           };
        glVertexAttribPointer((GLuint)ShaderAttrib::Pos, 3, GL_FLOAT, GL_FALSE, 0, vert_array);
        glVertexAttribPointer((GLuint)ShaderAttrib::Tex, 2, GL_FLOAT, GL_FALSE, 0, texc_array);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); checkGL();
        glDisableVertexAttribArray((GLuint)ShaderAttrib::Pos); checkGL();
        glDisableVertexAttribArray((GLuint)ShaderAttrib::Tex); checkGL();

        glEnable(GL_BLEND); checkGL();

        p_text_carsel.render_c(&p_gamemng->p_shadermng);
        glVertexAttrib4f((GLuint)ShaderAttrib::Color, 1,1,1,1); checkGL();
    }
    glDisable(GL_BLEND); checkGL();
}

inline bool ignoreKey(SDL_Keycode k)
{
    return k == SDLK_PRINTSCREEN || k == SDLK_NUMLOCKCLEAR;
}

void MainMenu::exitEnterMode()
{
    p_enterMode = false;
    for (unsigned i = 0; i != p_settings->joystickDevices->size(); ++i)
    {
        for (unsigned j = 0; j != (*p_settings->joystickDevices)[i].axesStates.size(); ++j)
        {
            (*p_settings->joystickDevices)[i].axesStates[j].resetEnterMode();
        }
    }
}

void MainMenu::countKeys()
{
    p_testKeysCount = 0;
    for (const auto& p : p_testKeysMap)
    {
        p_testKeysCount += (uint32_t)p.second;
    }
}

void MainMenu::event(const SDL_Event& e)
{
    // check for messages
    switch (e.type)
    {
        case SDL_KEYDOWN: // check for keypresses
        {
            SDL_Keycode sym = e.key.keysym.sym;
            if (p_state == STATE_CONTROLS_TEST_KEYBOARD_SCREEN)
            {
                if (!ignoreKey(e.key.keysym.sym))
                {
                    p_testKeysLastKeyDown = e.key.keysym.sym;
                    p_testKeysMap[e.key.keysym.sym] = 1;
                    countKeys();
                }
                if (sym == SDLK_ESCAPE)
                {
                    p_state = STATE_CONTROLS_TEST_KEYBOARD;
                    //SDL_EnableKeyRepeat(500, 30);
                }
            }
            else if (p_state == STATE_CONTROLS_TEST_KEYBOARD)
            {
                if (sym == SDLK_UP)
                {
                    if (p_state == STATE_CONTROLS_BEGIN)
                    {
                        p_state = STATE_CONTROLS_END - 1;
                    }
                    else
                    {
                        --p_state;
                    }
                }
                else if (sym == SDLK_DOWN)
                {
                    ++p_state;
                    if (p_state == STATE_CONTROLS_END)
                    {
                        p_state = STATE_CONTROLS_BEGIN;
                    }
                }
                else if (sym == SDLK_ESCAPE)
                {
                    p_state = STATE_CONTROLS;
                    if (p_controlsChanged)
                    {
                        p_controlsChanged = false;
                        //cout << "save settings" << endl;
                        //p_settings->save();
                    }
                }
            }
            else if (p_state >= STATE_CONTROLS_P1_UP && p_state <= STATE_CONTROLS_P4_RIGHT)
            {
                if (p_enterMode)
                {
                    int i = p_state - STATE_CONTROLS_P1_UP;
                    if (sym == SDLK_ESCAPE)
                    {
                        exitEnterMode();
                    }
                    else
                    {
                        if (!ignoreKey(sym))
                        {
                            p_settings->controls[i].type = Control::E_KEYBOARD;
                            p_settings->controls[i].i = sym;
                            p_controlsChanged = true;
                            exitEnterMode();
                        }
                    }
                }
                else
                {
                    if (sym == SDLK_UP)
                    {
                        if (p_state == STATE_CONTROLS_BEGIN)
                        {
                            p_state = STATE_CONTROLS_END - 1;
                        }
                        else
                        {
                            --p_state;
                        }
                    }
                    else if (sym == SDLK_DOWN)
                    {
                        ++p_state;
                        if (p_state == STATE_CONTROLS_END)
                        {
                            p_state = STATE_CONTROLS_BEGIN;
                        }
                    }
                    else if (sym == SDLK_ESCAPE)
                    {
                        p_state = STATE_CONTROLS;
                        if (p_controlsChanged)
                        {
                            p_controlsChanged = false;
                            //cout << "save settings" << endl;
                            //p_settings->save();
                        }
                    }
                    else if (sym == SDLK_RETURN || sym == SDLK_KP_ENTER)
                    {
                        p_enterMode = true;
                    }
                    else if (e.key.keysym.sym == SDLK_DELETE)
                    {
                        int i = p_state - STATE_CONTROLS_P1_UP;
                        p_settings->controls[i].type = Control::E_NONE;
                    }
                }
            }
            break;
        }
        case SDL_KEYUP: // check for keypresses
        {
            if (p_state == STATE_CONTROLS_TEST_KEYBOARD_SCREEN)
            {
                if (!ignoreKey(e.key.keysym.sym))
                {
                    p_testKeysLastKeyUp = e.key.keysym.sym;
                    p_testKeysMap[e.key.keysym.sym] = 0;
                    countKeys();
                }
            }
            if (p_state == STATE_CONTROLS && (e.key.keysym.sym == SDLK_RETURN || e.key.keysym.sym == SDLK_KP_ENTER))
            {
                p_state = STATE_CONTROLS_TEST_KEYBOARD;
                //p_controlsChanged = true;
                //exitEnterMode();
                p_enterMode = false; // počáteční inicializace
            }
            else if (p_state == STATE_CONTROLS_TEST_KEYBOARD && (e.key.keysym.sym == SDLK_RETURN || e.key.keysym.sym == SDLK_KP_ENTER))
            {
                p_state = STATE_CONTROLS_TEST_KEYBOARD_SCREEN;
                //SDL_EnableKeyRepeat(0, 0);
                p_testKeysCount = 0;
                p_testKeysLastKeyDown = -1; //SDLK_LAST;
                p_testKeysLastKeyUp = -1; //SDLK_LAST;
                p_testKeysMap.clear();
            }
            break;
        }
        case SDL_JOYAXISMOTION:
        {


            if (static_cast<unsigned>(e.jaxis.which) < p_settings->joystickDevices->size())
            {
                JoystickDevice& joystickDevice = (*p_settings->joystickDevices)[static_cast<unsigned>(e.jaxis.which)];
                if (static_cast<unsigned>(e.jaxis.axis) < joystickDevice.axesStates.size())
                {
                    int updateStateResult = joystickDevice.axesStates[static_cast<unsigned>(e.jaxis.axis)].updateState(e.jaxis.value, p_enterMode);
                    if (updateStateResult != 0 && p_state >= STATE_CONTROLS_P1_UP && p_state <= STATE_CONTROLS_P4_RIGHT)
                    {
                        int i = p_state - STATE_CONTROLS_P1_UP;
                        p_settings->controls[i].type = Control::E_JAXIS;
                        p_settings->controls[i].i = e.jaxis.axis;
                        p_settings->controls[i].joystickDeviceIndex = e.jaxis.which;
                        p_settings->controls[i].joystickAxisValue = updateStateResult;
                        p_controlsChanged = true;
                        exitEnterMode();
                    }
                }
            }

            break;
        }
        case SDL_JOYBUTTONDOWN:
        {
            if (p_state >= STATE_CONTROLS_P1_UP && p_state <= STATE_CONTROLS_P4_RIGHT && p_enterMode)
            {
                int i = p_state - STATE_CONTROLS_P1_UP;
                p_settings->controls[i].type = Control::E_JBUTTON;
                p_settings->controls[i].i = e.jbutton.button;
                p_settings->controls[i].joystickDeviceIndex = e.jbutton.which;
                p_controlsChanged = true;
                exitEnterMode();
            }
            break;
        }
        case SDL_JOYHATMOTION:
        {
            if (p_state >= STATE_CONTROLS_P1_UP && p_state <= STATE_CONTROLS_P4_RIGHT && p_enterMode)
            {
                int i = p_state - STATE_CONTROLS_P1_UP;
                static const Uint8 hatDirections[4] = {SDL_HAT_LEFT, SDL_HAT_RIGHT, SDL_HAT_UP, SDL_HAT_DOWN};
                for (int j = 0; j != 4; ++j)
                {
                    if (e.jhat.value & hatDirections[j])
                    {
                        p_settings->controls[i].type = Control::E_JHAT;
                        p_settings->controls[i].i = e.jhat.hat;
                        p_settings->controls[i].joystickDeviceIndex = e.jhat.which;
                        p_settings->controls[i].joystickHatValue = hatDirections[j];
                        p_controlsChanged = true;
                        exitEnterMode();
                        break;
                    }
                }
            }
            break;
        }
        case SDL_MOUSEBUTTONDOWN:
        {
            if (p_state >= STATE_CONTROLS_P1_UP && p_state <= STATE_CONTROLS_P4_RIGHT)
            {
                if (p_enterMode)
                {
                    int i = p_state - STATE_CONTROLS_P1_UP;


                    //if (e.button.button != SDL_BUTTON_WHEELDOWN && e.button.button != SDL_BUTTON_WHEELUP)
                    {
                        p_settings->controls[i].type = Control::E_MBUTTON;
                        p_settings->controls[i].i = e.button.button;
                        p_controlsChanged = true;
                        exitEnterMode();
                    }
                }
            }
            break;
        }
    } // end switch

    afterEvent();
}

bool MainMenu::keydown(unsigned int sym)
{
    switch (p_state)
    {
    case STATE_RACE:
        if (sym == SDLK_DOWN) p_state = STATE_CONTROLS;
        else if (sym == SDLK_UP) p_state = STATE_QUIT;
        else if (sym == SDLK_RETURN || sym == SDLK_KP_ENTER) p_state = STATE_R_NUM;
        else if (sym == SDLK_ESCAPE) p_state = STATE_QUIT;
        break;
    case STATE_CONTROLS:
        if (sym == SDLK_DOWN) p_state = STATE_OPTIONS;
        else if (sym == SDLK_UP) p_state = STATE_RACE;
        else if (sym == SDLK_ESCAPE) p_state = STATE_QUIT;
        break;
    case STATE_OPTIONS:
        if (sym == SDLK_DOWN) p_state = STATE_BEST_LAPS;
        else if (sym == SDLK_UP) p_state = STATE_CONTROLS;
        else if (sym == SDLK_RETURN || sym == SDLK_KP_ENTER) p_state = STATE_OPTIONS_SOUNDVOL;
        else if (sym == SDLK_ESCAPE) p_state = STATE_QUIT;
        break;
    case STATE_BEST_LAPS:
        if (sym == SDLK_DOWN) p_state = STATE_QUIT;
        else if (sym == SDLK_UP) p_state = STATE_OPTIONS;
        else if (sym == SDLK_RETURN || sym == SDLK_KP_ENTER)
        {
            p_state = STATE_BEST_LAPS_SCREEN;
            loadAndPrintBestTimes();
        }
        else if (sym == SDLK_ESCAPE) p_state = STATE_QUIT;
        break;

    case STATE_QUIT:
        if (sym == SDLK_DOWN) p_state = STATE_RACE;
        else if (sym == SDLK_UP) p_state = STATE_BEST_LAPS;
        else if (sym == SDLK_RETURN || sym == SDLK_KP_ENTER) p_state = STATE_Q;
        else if (sym == SDLK_ESCAPE) p_state = STATE_QUIT;
        break;

    case STATE_BEST_LAPS_SCREEN:
        if (sym == SDLK_RETURN || sym == SDLK_ESCAPE || sym == SDLK_KP_ENTER) p_state = STATE_BEST_LAPS;
        break;

    case STATE_R_NUM:
        if (sym == SDLK_DOWN) p_state = STATE_R_CAR1;
        else if (sym == SDLK_UP) p_state = STATE_R_GO;
        else if (sym == SDLK_LEFT) {--p_players; if (p_players < 1) p_players = 1;}
        else if (sym == SDLK_RIGHT) {++p_players; if (p_players > 4) p_players = 4;}
        else if (sym == SDLK_ESCAPE) p_state = STATE_RACE;
        break;
    case STATE_R_CAR1:
        p_car_i = 0;
        if (sym == SDLK_DOWN) {if (p_players > 1) p_state = STATE_R_CAR2; else p_state = STATE_R_TRACK;}
        else if (sym == SDLK_UP) p_state = STATE_R_NUM;
        else if (sym == SDLK_ESCAPE) p_state = STATE_RACE;
        else if (sym == SDLK_RETURN || sym == SDLK_KP_ENTER) { p_state = STATE_CAR_SEL; p_car_i = 0; }
        break;
    case STATE_R_CAR2:
        p_car_i = 1;
        if (sym == SDLK_DOWN) {if (p_players > 2) p_state = STATE_R_CAR3; else p_state = STATE_R_TRACK;}
        else if (sym == SDLK_UP) p_state = STATE_R_CAR1;
        else if (sym == SDLK_ESCAPE) p_state = STATE_RACE;
        else if (sym == SDLK_RETURN || sym == SDLK_KP_ENTER) { p_state = STATE_CAR_SEL; p_car_i = 1; }
        break;
    case STATE_R_CAR3:
        p_car_i = 2;
        if (sym == SDLK_DOWN) {if (p_players > 3) p_state = STATE_R_CAR4; else p_state = STATE_R_TRACK;}
        else if (sym == SDLK_UP) p_state = STATE_R_CAR2;
        else if (sym == SDLK_ESCAPE) p_state = STATE_RACE;
        else if (sym == SDLK_RETURN || sym == SDLK_KP_ENTER) { p_state = STATE_CAR_SEL; p_car_i = 2; }
        break;
    case STATE_R_CAR4:
        p_car_i = 3;
        if (sym == SDLK_DOWN) {p_state = STATE_R_TRACK;}
        else if (sym == SDLK_UP) p_state = STATE_R_CAR3;
        else if (sym == SDLK_ESCAPE) p_state = STATE_RACE;
        else if (sym == SDLK_RETURN || sym == SDLK_KP_ENTER) { p_state = STATE_CAR_SEL; p_car_i = 3; }
        break;
    case STATE_R_TRACK:
        if (sym == SDLK_DOWN) p_state = STATE_R_LAPS;
        else if (sym == SDLK_UP) p_state = STATE_R_NUM+p_players;
        else if (sym == SDLK_ESCAPE) p_state = STATE_RACE;
        else if (sym == SDLK_RETURN || sym == SDLK_KP_ENTER) { p_state = STATE_TRACK_SEL; p_direction_sel = false; }
        break;
    case STATE_R_LAPS:
        if (sym == SDLK_DOWN) p_state = STATE_R_DIRECTION;
        else if (sym == SDLK_UP) p_state = STATE_R_TRACK;
        else if (sym == SDLK_LEFT) { --p_laps; if (p_laps < 1) p_laps = 1; }
        else if (sym == SDLK_RIGHT) { ++p_laps; if (p_laps > 50) p_laps = 50; }
        else if (sym == SDLK_ESCAPE) p_state = STATE_RACE;
        break;
    case STATE_R_DIRECTION:
        if (sym == SDLK_DOWN) p_state = STATE_R_DAYTIME;
        else if (sym == SDLK_UP) p_state = STATE_R_LAPS;
        else if (sym == SDLK_LEFT) p_direction_sel = false;
        else if (sym == SDLK_RIGHT) p_direction_sel = true;
        else if (sym == SDLK_ESCAPE) p_state = STATE_RACE;
        break;
    case STATE_R_DAYTIME:
        if (sym == SDLK_DOWN) p_state = STATE_R_GO;
        else if (sym == SDLK_UP) p_state = STATE_R_DIRECTION;
        else if (sym == SDLK_LEFT)
        {
            p_daytime_sel = 0;
            p_sky_sel = randn1(2);
            if (p_daytime_sel == 1)
                p_sky_sel += 2;
        }
        else if (sym == SDLK_RIGHT)
        {
            p_daytime_sel = 1;
            p_sky_sel = randn1(2);
            if (p_daytime_sel == 1)
                p_sky_sel += 2;
        }
        else if (sym == SDLK_ESCAPE) p_state = STATE_RACE;
        break;
    case STATE_R_GO:
        if (sym == SDLK_DOWN) p_state = STATE_R_NUM;
        else if (sym == SDLK_UP) p_state = STATE_R_DAYTIME;
        else if (sym == SDLK_ESCAPE) p_state = STATE_RACE;
        else if (sym == SDLK_RETURN || sym == SDLK_KP_ENTER) game();
        break;
    case STATE_OPTIONS_SOUNDVOL:
        if (sym == SDLK_DOWN) p_state = STATE_OPTIONS_VIEWDIST;
        else if (sym == SDLK_UP) p_state = STATE_OPTIONS_VIEWDIST;
        else if (sym == SDLK_LEFT || sym == SDLK_RIGHT)
        {
            if (sym == SDLK_LEFT)
            {
                p_sound_vol -= 5; if (p_sound_vol < 0) p_sound_vol = 0;
            }
            else
            {
                p_sound_vol += 5; if (p_sound_vol > 100) p_sound_vol = 100;
            }
            p_gamemng->p_sound_game_static.playSoundTest(p_sound_vol * 0.01f);
        }
        else if (sym == SDLK_ESCAPE) p_state = STATE_OPTIONS;
        else if (sym == SDLK_RETURN || sym == SDLK_KP_ENTER) p_state = STATE_OPTIONS;
        break;
    case STATE_OPTIONS_VIEWDIST:
        if (sym == SDLK_DOWN) p_state = STATE_OPTIONS_SOUNDVOL;
        else if (sym == SDLK_UP) p_state = STATE_OPTIONS_SOUNDVOL;
        else if (sym == SDLK_LEFT) { --p_view_dist; if (p_view_dist < 0) p_view_dist = 0; }
        else if (sym == SDLK_RIGHT) { ++p_view_dist; if (p_view_dist > 10) p_view_dist = 10; }
        else if (sym == SDLK_ESCAPE) p_state = STATE_OPTIONS;
        else if (sym == SDLK_RETURN || sym == SDLK_KP_ENTER) p_state = STATE_OPTIONS;
        break;


    case STATE_CAR_SEL:
        if (sym == SDLK_DOWN)
        {
            ++p_cars_sel[p_car_i];
            if (p_cars_sel[p_car_i] >= int(p_gamemng->p_cars.size()))
                p_cars_sel[p_car_i] = 0;
            p_cars_tex_sel[p_car_i] = 0;
        }
        else if (sym == SDLK_UP)
        {
            --p_cars_sel[p_car_i];
            if (p_cars_sel[p_car_i] < 0)
                p_cars_sel[p_car_i] = int(p_gamemng->p_cars.size())-1;
            p_cars_tex_sel[p_car_i] = 0;
        }
        else if (sym == SDLK_LEFT)
        { //sz_mods
            //
            --p_cars_tex_sel[p_car_i];
            if (p_cars_tex_sel[p_car_i] < 0)
            {
                p_cars_tex_sel[p_car_i] = int(p_gamemng->p_cars[p_cars_sel[p_car_i]].pict_tex.size())-1;
            }
        }
        else if (sym == SDLK_RIGHT)
        {
            //
            ++p_cars_tex_sel[p_car_i];
            if (p_cars_tex_sel[p_car_i] >= int(p_gamemng->p_cars[p_cars_sel[p_car_i]].pict_tex.size()))
            {
                p_cars_tex_sel[p_car_i] = 0;
            }
        }
        else if (sym == SDLK_ESCAPE) p_state = STATE_R_CAR1+p_car_i;
        else if (sym == SDLK_RETURN || sym == SDLK_KP_ENTER) p_state = STATE_R_CAR1+p_car_i;
        break;
    case STATE_TRACK_SEL:
        if (sym == SDLK_LEFT) { --p_track_sel; if (p_track_sel < 0) p_track_sel = int(p_gamemng->p_maps.size())-1; }
        else if (sym == SDLK_RIGHT) { ++p_track_sel; if (p_track_sel >= int(p_gamemng->p_maps.size())) p_track_sel = 0; }
        else if (sym == SDLK_ESCAPE) p_state = STATE_R_TRACK;
        else if (sym == SDLK_RETURN || sym == SDLK_KP_ENTER) p_state = STATE_R_TRACK;
        break;
    }

    afterEvent();

    if (p_state == STATE_Q)
    {
        return true; // done
    }
    return false; // not done
}

void MainMenu::loadAndPrintBestTimes()
{
    for (unsigned i = 0; i != p_gamemng->p_maps.size(); ++i)
    {
        for (unsigned j = 0; j != 2; ++j)
        {
            Ghost ghost(false);
            if (ghost.load(i, j))
            {
                p_text_best_laps_cars.puts     (i * 2 + j, p_gamemng->p_cars[ghost.m_car].name);
                p_text_best_laps_cars.set_color(i * 2 + j, p_gamemng->p_cars[ghost.m_car].pict_tex[ghost.m_carcolor].color);
                p_text_best_laps_times.puts    (i * 2 + j, time_m_s(ghost.m_seconds));
            }
            else
            {
                p_text_best_laps_cars.puts     (i * 2 + j, "");
                p_text_best_laps_times.puts    (i * 2 + j, "");
            }
        }
    }
}

void MainMenu::afterEvent()
{
    if (p_state >= STATE_CONTROLS_BEGIN && p_state < STATE_CONTROLS_END)
    {
        p_text_controls0.set_color(0, p_state == STATE_CONTROLS_TEST_KEYBOARD ? font_color_hi : font_color);

        p_text_controls1.set_color( 2, p_state == STATE_CONTROLS_P1_UP ? (p_enterMode ? font_color_enter : font_color_hi) : font_color);
        p_text_controls1.set_color( 3, p_state == STATE_CONTROLS_P1_DOWN ? (p_enterMode ? font_color_enter : font_color_hi) : font_color);
        p_text_controls1.set_color( 4, p_state == STATE_CONTROLS_P1_LEFT ? (p_enterMode ? font_color_enter : font_color_hi) : font_color);
        p_text_controls1.set_color( 5, p_state == STATE_CONTROLS_P1_RIGHT ? (p_enterMode ? font_color_enter : font_color_hi) : font_color);

        p_text_controls1.set_color( 7, p_state == STATE_CONTROLS_P2_UP ? (p_enterMode ? font_color_enter : font_color_hi) : font_color);
        p_text_controls1.set_color( 8, p_state == STATE_CONTROLS_P2_DOWN ? (p_enterMode ? font_color_enter : font_color_hi) : font_color);
        p_text_controls1.set_color( 9, p_state == STATE_CONTROLS_P2_LEFT ? (p_enterMode ? font_color_enter : font_color_hi) : font_color);
        p_text_controls1.set_color(10, p_state == STATE_CONTROLS_P2_RIGHT ? (p_enterMode ? font_color_enter : font_color_hi) : font_color);

        p_text_controls1.set_color(12, p_state == STATE_CONTROLS_P3_UP ? (p_enterMode ? font_color_enter : font_color_hi) : font_color);
        p_text_controls1.set_color(13, p_state == STATE_CONTROLS_P3_DOWN ? (p_enterMode ? font_color_enter : font_color_hi) : font_color);
        p_text_controls1.set_color(14, p_state == STATE_CONTROLS_P3_LEFT ? (p_enterMode ? font_color_enter : font_color_hi) : font_color);
        p_text_controls1.set_color(15, p_state == STATE_CONTROLS_P3_RIGHT ? (p_enterMode ? font_color_enter : font_color_hi) : font_color);

        p_text_controls1.set_color(17, p_state == STATE_CONTROLS_P4_UP ? (p_enterMode ? font_color_enter : font_color_hi) : font_color);
        p_text_controls1.set_color(18, p_state == STATE_CONTROLS_P4_DOWN ? (p_enterMode ? font_color_enter : font_color_hi) : font_color);
        p_text_controls1.set_color(19, p_state == STATE_CONTROLS_P4_LEFT ? (p_enterMode ? font_color_enter : font_color_hi) : font_color);
        p_text_controls1.set_color(20, p_state == STATE_CONTROLS_P4_RIGHT ? (p_enterMode ? font_color_enter : font_color_hi) : font_color);

        unsigned controlLineIndices[16] = {2,3,4,5,7,8,9,10,12,13,14,15,17,18,19,20};

        char controlBuff[1024] = {0};
        for (int i = 0; i != 16; ++i)
        {
            p_settings->getControlName(controlBuff, 1023, i, true);
            p_text_controls2.puts(controlLineIndices[i], controlBuff);
            bool disconnectedColor =
                (p_settings->controls[i].type == Control::E_JAXIS ||
                p_settings->controls[i].type == Control::E_JHAT ||
                p_settings->controls[i].type == Control::E_JBUTTON) &&
                p_settings->controls[i].joystickDeviceIndex < 0;
            p_text_controls2.set_color(controlLineIndices[i], disconnectedColor ? font_color_disconnected : font_color);
        }
        if (p_enterMode)
        {
            if (p_state >= STATE_CONTROLS_P1_UP && p_state <= STATE_CONTROLS_P4_RIGHT)
            {
                int i = p_state - STATE_CONTROLS_P1_UP;
                p_text_controls2.puts(controlLineIndices[i], "\?\?\?");
                p_text_controls2.set_color(controlLineIndices[i], font_color_enter);
            }
        }
    }
    else if (p_state == STATE_CONTROLS_TEST_KEYBOARD_SCREEN)
    {
        char buff[256] = {0};
        snprintf(buff, 255, "Detected %d pressed keys.", p_testKeysCount);
        p_text_test_keyboard_status.puts(0, buff);
        if (p_testKeysLastKeyDown != /*SDLK_LAST*/-1)
        {
            snprintf(buff, 255, "  %s", SDL_GetKeyName(p_testKeysLastKeyDown));
            p_text_test_keyboard_status.puts(3, buff);
        }
        else
        {
            p_text_test_keyboard_status.puts(3, "");
        }
        if (p_testKeysLastKeyUp != /*SDLK_LAST*/-1)
        {
            snprintf(buff, 255, "  %s", SDL_GetKeyName(p_testKeysLastKeyUp));
            p_text_test_keyboard_status.puts(5, buff);
        }
        else
        {
            p_text_test_keyboard_status.puts(5, "");
        }
    }
    else if (p_state >= STATE_RACE && p_state <= STATE_Q)
    {
        p_text_main.set_color(0, font_color);
        p_text_main.set_color(1, font_color);
        p_text_main.set_color(2, font_color);
        p_text_main.set_color(3, font_color);
        p_text_main.set_color(4, font_color);
        p_text_main.set_color(5, font_color);
        if (p_state == STATE_Q)
            p_text_main.set_color(5, font_color_hi);
        else
            p_text_main.set_color(p_state-STATE_RACE, font_color_hi);
    }
    else if (p_state >= STATE_R_NUM && p_state <= STATE_R_GO)
    {
        char buff[256] = {0};
        snprintf(buff, 255, "Num. of players: <  %u  >", p_players);
        p_text_race_n.puts(0, buff);

        snprintf(buff, 255, "<  %u  >", p_laps);
        p_text_race_track35.puts(0, buff);
        p_text_race_track35.puts(1, p_direction_sel ? "<  Reversed  >" : "<  Normal  >");
        p_text_race_track35.puts(2, p_daytime_sel == 0 ? "<  Day  >" : "<  Evening  >");

        p_text_race_go.set_color(font_color);
        p_text_race_n.set_color(font_color);
        p_text_race_car.set_color(font_color);
        p_text_race_track0.set_color(font_color);
        p_text_race_track1.set_color(font_color);
        p_text_race_track24.set_color(font_color);
        p_text_race_track35.set_color(font_color);
        p_text_race_car.puts(0,
            "Car 1:\n"
            "Car 2:\n"
            "Car 3:\n"
            "Car 4:"
        );
        for (unsigned int i = 0; i != p_players; ++i)
        {
            p_text_race_car2.puts(i, p_gamemng->p_cars[p_cars_sel[i]].name);
            p_text_race_car2.set_color(i, p_gamemng->p_cars[p_cars_sel[i]].pict_tex[p_cars_tex_sel[i]].color);
        }
        for (unsigned int i = p_players; i != 4; ++i)
        {
            p_text_race_car.puts(i, "");
            p_text_race_car2.puts(i, "");
        }

        p_text_race_track1.puts(0, p_gamemng->p_maps[p_track_sel].name);

        switch (p_state)
        {
        case STATE_R_NUM:
            p_text_race_n.set_color(0, font_color_hi);
            p_text_race_status.puts(0, "Press Left/Right to change.");
            break;
        case STATE_R_CAR1:
        case STATE_R_CAR2:
        case STATE_R_CAR3:
        case STATE_R_CAR4:
            p_text_race_car.set_color(p_state-STATE_R_CAR1, font_color_hi);
            p_text_race_status.puts(0, "Press Return/Enter to select car.");
            break;
        case STATE_R_TRACK:
            p_text_race_track0.set_color(0, font_color_hi);
            p_text_race_track1.set_color(0, font_color_hi);
            p_text_race_status.puts(0, "Press Return/Enter to select track.");
            break;
        case STATE_R_LAPS:
            p_text_race_track24.set_color(0, font_color_hi);
            p_text_race_track35.set_color(0, font_color_hi);
            p_text_race_status.puts(0, "Press Left/Right to change.");
            break;
        case STATE_R_DIRECTION:
            p_text_race_track24.set_color(1, font_color_hi);
            p_text_race_track35.set_color(1, font_color_hi);
            p_text_race_status.puts(0, "Press Left/Right to change.");
            break;
        case STATE_R_DAYTIME:
            p_text_race_track24.set_color(2, font_color_hi);
            p_text_race_track35.set_color(2, font_color_hi);
            p_text_race_status.puts(0, "Press Left/Right to change.");
            break;
        case STATE_R_GO:
            p_text_race_go.set_color(0, font_color_hi);
            p_text_race_status.puts(0, "");
            break;
        default:
            p_text_race_status.puts(0, "");
            break;
        }

        /*STATE_R_NUM, STATE_R_CAR1, STATE_R_CAR2, STATE_R_CAR3, STATE_R_CAR4,
        STATE_R_TRACK, STATE_R_LAPS, STATE_R_DIRECTION, STATE_R_DAYTIME, STATE_R_GO,*/
    }
    else if (p_state >= STATE_OPTIONS_SOUNDVOL && p_state <= STATE_OPTIONS_VIEWDIST)
    {
        p_text_opt2.set_color(font_color);
        p_text_opt2.set_color(p_state-STATE_OPTIONS_SOUNDVOL+2, font_color_hi);
        p_text_opt3.set_color(font_color);
        p_text_opt3.set_color(p_state-STATE_OPTIONS_SOUNDVOL+2, font_color_hi);
        p_opt_color0[0] = p_opt_color0[1] = p_opt_color0[2] = (p_state == STATE_OPTIONS_SOUNDVOL) ? 1.f : 0.6f;
        p_opt_color1[0] = p_opt_color1[1] = p_opt_color1[2] = (p_state == STATE_OPTIONS_SOUNDVOL) ? 0.6f : 1.f;

        float xcmax = 8.f*0.319f;
        float xposun = 4.3f*0.319f;

        p_opt_verts[3] = xcmax*p_sound_vol*0.01f+xposun;
        p_opt_verts[6] = xcmax*p_sound_vol*0.01f+xposun;

        p_opt_verts[15] = xcmax*p_view_dist*0.1f+xposun;
        p_opt_verts[18] = xcmax*p_view_dist*0.1f+xposun;
    }
    else if (p_state == STATE_CAR_SEL)
    {
        char buff[256] = {0};
        snprintf(buff, 255, "Select player %d car", p_car_i+1);
        p_text_carsel.puts(0, buff);
        p_text_carsel.puts(8, p_gamemng->p_cars[p_cars_sel[p_car_i]].name);
        p_text_carsel.set_color(8, p_gamemng->p_cars[p_cars_sel[p_car_i]].pict_tex[p_cars_tex_sel[p_car_i]].color);
        p_tex_sel_bnd = p_gamemng->p_cars[p_cars_sel[p_car_i]].pict_tex[p_cars_tex_sel[p_car_i]].tex;
    }
    else if (p_state == STATE_TRACK_SEL)
    {
        p_text_carsel.puts(0, "Select track");
        p_text_carsel.puts(8, p_gamemng->p_maps[p_track_sel].name);
        p_text_carsel.set_color(8, font_color_hi);
        p_tex_sel_bnd = p_gamemng->p_maps[p_track_sel].pict_tex;
    }
}
