#include "gamemng.h"
#include "mainmenu.h"
#include "glhelpers1.h"
#include "alleg_minisdl.h"

/*
Restart Race
Test Keys -> GMSTATE_T
Options
End Race
Quit to Menu
*/

void Gamemenu::init()
{
    float font_color[4] = {0.6, 0.6, 0.6, 1};
    gltext_menu.init(40, 6, 2.f, 0, 0, &(p_gamemng->p_glfont), font_color);
    gltext_menu.set_pos(0.f, 0.f);
    gltext_menu.puts(0,
        "Restart Race\n"
        "Test Keys\n"
        "Options\n"
        "End Race\n"
        "Quit to Menu\n"
    );

    p_text_opt.init(40, 5, 2.f, 0, 0, &(p_gamemng->p_glfont), font_color);
    p_text_opt.set_pos(0.f, 0.f);
    p_text_opt.puts(0,
        STRING_OPTIONS_TITLE
    );

    p_text_opt2.init(40, 5, 2.f, -1, 0, &(p_gamemng->p_glfont), font_color);
    p_text_opt2.set_pos(0.f, 0.f);
    p_text_opt2.puts(0,
        STRING_OPTIONS_LABELS
    );

    p_text_opt3.init(40, 5, 2.f, 1, 0, &(p_gamemng->p_glfont), font_color);
    p_text_opt3.set_pos(0.f, 0.f);
    p_text_opt3.puts(0,
        STRING_OPTIONS_ARROWS
    );

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
}

void Gamemenu::render()
{
    if (state >= GMSTATE_RESTART && state <= GMSTATE_QUIT)
    {
        gltext_menu.render_c();
    }
    else if (state >= GMSTATE_O_SOUNDVOL && state <= GMSTATE_O_VIEWDIST)
    {
        p_text_opt.render_c();
        p_text_opt2.render_c();
        p_text_opt3.render_c();

        glDisable(GL_TEXTURE_2D); checkGL();
        glDisable(GL_BLEND); checkGL();
        glColor4f(1, 1, 1, 1); checkGL();
        glEnableClientState(GL_VERTEX_ARRAY); checkGL();
        glVertexPointer(3, GL_FLOAT, 0, p_opt_verts); checkGL();
        glColor3fv(p_opt_color0); checkGL();
        glDrawArrays(GL_QUADS, 0, 4); checkGL();
        glColor3fv(p_opt_color1); checkGL();
        glDrawArrays(GL_QUADS, 4, 4); checkGL();
        glDisableClientState(GL_VERTEX_ARRAY); checkGL();
    }
}

void Gamemenu::keydown(unsigned int sym)
{
    if (!bmenu)
    {
        if (sym == SDLK_ESCAPE)
        {
            p_gamemng->gamemenu_sw(false);
        }
        else if ((sym == SDLK_RETURN || sym == SDLK_KP_ENTER) && p_gamemng->p_state == 2)
        {
            p_gamemng->gamemenu_sw(false);
        }
        else return;
    } else {
        switch (state)
        {
        case GMSTATE_RESTART:
            if (sym == SDLK_ESCAPE) p_gamemng->gamemenu_sw(true);
            else if (sym == SDLK_RETURN || sym == SDLK_KP_ENTER)
            {
                p_gamemng->gamemenu_sw(true);
                p_gamemng->restart();
            }
            else if (sym == SDLK_DOWN) state = p_gamemng->p_state != 2 ? GMSTATE_TEST : GMSTATE_OPTIONS;
            else if (sym == SDLK_UP) state = GMSTATE_QUIT;
            break;
        case GMSTATE_TEST:
            if (sym == SDLK_ESCAPE) p_gamemng->gamemenu_sw(true);
            else if (sym == SDLK_RETURN || sym == SDLK_KP_ENTER) state = GMSTATE_T;
            else if (sym == SDLK_DOWN) state = GMSTATE_OPTIONS;
            else if (sym == SDLK_UP) state = GMSTATE_RESTART;
            break;
        case GMSTATE_OPTIONS:
            if (sym == SDLK_ESCAPE) p_gamemng->gamemenu_sw(true);
            else if (sym == SDLK_RETURN || sym == SDLK_KP_ENTER) state = GMSTATE_O_SOUNDVOL;
            else if (sym == SDLK_DOWN) state = p_gamemng->p_state != 2 ? GMSTATE_END : GMSTATE_QUIT;
            else if (sym == SDLK_UP) state = p_gamemng->p_state != 2 ? GMSTATE_TEST : GMSTATE_RESTART;
            break;
        case GMSTATE_END:
            if (sym == SDLK_ESCAPE) p_gamemng->gamemenu_sw(true);
            else if (sym == SDLK_RETURN || sym == SDLK_KP_ENTER)
            {
                p_gamemng->gamemenu_sw(true);
                p_gamemng->end_race();
            }
            else if (sym == SDLK_DOWN) state = GMSTATE_QUIT;
            else if (sym == SDLK_UP) state = GMSTATE_OPTIONS;
            break;
        case GMSTATE_QUIT:
            if (sym == SDLK_ESCAPE) p_gamemng->gamemenu_sw(true);
            else if (sym == SDLK_RETURN || sym == SDLK_KP_ENTER)
            {
                p_settings->set("sound_volume", p_gamemng->get_global_volume());
                p_settings->set("view_distance", p_gamemng->get_far());
                p_gamemng->gamemenu_sw(true);
                p_gamemng->quit_race(); // vypne zvuk
                p_menu->menu();
            }
            else if (sym == SDLK_DOWN) state = GMSTATE_RESTART;
            else if (sym == SDLK_UP) state = p_gamemng->p_state != 2 ? GMSTATE_END : GMSTATE_OPTIONS;
            break;
        case GMSTATE_T:
            if (sym == SDLK_ESCAPE) state = GMSTATE_TEST;
            break;
        case GMSTATE_O_SOUNDVOL:
            if      (sym == SDLK_ESCAPE) state = GMSTATE_OPTIONS;
            else if (sym == SDLK_RETURN || sym == SDLK_KP_ENTER) state = GMSTATE_OPTIONS;
            else if (sym == SDLK_LEFT || sym == SDLK_RIGHT)
            {
                if (sym == SDLK_LEFT)
                {
                    p_gamemng->set_global_volume(p_gamemng->get_global_volume()-5);
                }
                else
                {
                    p_gamemng->set_global_volume(p_gamemng->get_global_volume()+5);
                }
                p_gamemng->p_sound_game_static.playSoundTest(p_gamemng->get_global_volume() * 0.01f);
            }
            else if (sym == SDLK_DOWN) state = GMSTATE_O_VIEWDIST;
            else if (sym == SDLK_UP) state = GMSTATE_O_VIEWDIST;
            break;
        case GMSTATE_O_VIEWDIST:
            if      (sym == SDLK_ESCAPE) state = GMSTATE_OPTIONS;
            else if (sym == SDLK_RETURN || sym == SDLK_KP_ENTER) state = GMSTATE_OPTIONS;
            else if (sym == SDLK_LEFT)  { p_gamemng->set_far(p_gamemng->get_far()-1); }
            else if (sym == SDLK_RIGHT) { p_gamemng->set_far(p_gamemng->get_far()+1); }
            else if (sym == SDLK_DOWN) state = GMSTATE_O_SOUNDVOL;
            else if (sym == SDLK_UP) state = GMSTATE_O_SOUNDVOL;
            break;
        }
    }
    float font_color[4] = {0.6, 0.6, 0.6, 1};
    float font_color_hi[4] = {1, 1, 1, 1};
    if (state >= GMSTATE_RESTART && state <= GMSTATE_QUIT)
    {
        if (p_gamemng->p_state != 2)
        {
            gltext_menu.puts(0,
                "Restart Race\n"
                "Test Keys\n"
                "Options\n"
                "End Race\n"
                "Quit to Menu\n"
            );
            gltext_menu.set_color(font_color);
            gltext_menu.set_color(state - GMSTATE_RESTART, font_color_hi);
        } else {
            gltext_menu.puts(0,
                "\n"
                "Restart Race\n"
                "Options\n"
                "Quit to Menu\n"
                "\n"
            );
            gltext_menu.set_color(font_color);
            if (state == GMSTATE_RESTART) gltext_menu.set_color(1, font_color_hi);
            else if (state == GMSTATE_OPTIONS) gltext_menu.set_color(2, font_color_hi);
            else if (state == GMSTATE_QUIT) gltext_menu.set_color(3, font_color_hi);
        }
    }
    else if (state >= GMSTATE_O_SOUNDVOL && state <= GMSTATE_O_VIEWDIST)
    {
        p_text_opt2.set_color(font_color);
        p_text_opt2.set_color(state-GMSTATE_O_SOUNDVOL+2, font_color_hi);
        p_text_opt3.set_color(font_color);
        p_text_opt3.set_color(state-GMSTATE_O_SOUNDVOL+2, font_color_hi);


        p_opt_color0[0] = p_opt_color0[1] = p_opt_color0[2] = (state == GMSTATE_O_SOUNDVOL) ? 1.f : 0.6f;
        p_opt_color1[0] = p_opt_color1[1] = p_opt_color1[2] = (state == GMSTATE_O_SOUNDVOL) ? 0.6f : 1.f;

        float xcmax = 8.f*0.319f;
        float xposun = 4.3f*0.319f;

        p_opt_verts[3] = xcmax*p_gamemng->get_global_volume()*0.01f+xposun;
        p_opt_verts[6] = xcmax*p_gamemng->get_global_volume()*0.01f+xposun;

        p_opt_verts[15] = xcmax*p_gamemng->get_far()*0.1f+xposun;
        p_opt_verts[18] = xcmax*p_gamemng->get_far()*0.1f+xposun;
    }
}

void Gamemng::gamemenu_sw(bool b_quit)
{
    if (b_quit)
        p_gamemenu.bmenu = false;
    else
    {
        p_gamemenu.bmenu = true;
        p_gamemenu.state = GMSTATE_RESTART;
    }
    if (p_gamemenu.bmenu)
    {
        // zastav zvuky
        for (unsigned int i = 0; i != p_players; ++i)
        {
            p_sound_car[i].stop();
        }
        //SDL_EnableKeyRepeat(500, 30);
    } else {
        // rozjeď zvuky - to udělá automaticky frame díky chytrému stop();
        //SDL_EnableKeyRepeat(0, 0);
    }
}
