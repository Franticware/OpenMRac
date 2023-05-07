#include "gamemng.h"
#include "cstring1.h"
#include "rand1.h"
#include "mtrxinv.h"
#include "gbuff_in.h"
#include "pict2.h"
#include "load_texture.h"
#include <cmath>
#include <algorithm>
#include "glhelpers1.h"

#include "soundmng.h"

extern int ge_bpass1; // globální vypnutí aktualizace pohledu

extern int g_freecam;

extern int g_sound_on;

// výpočet jednoho snímku + vykreslení přes render_frame
void Gamemng::frame(float deltaT)
{
    if (p_gamemenu.bmenu)
        deltaT = 0.f;

    if (p_state0_5 && p_state == 1)
    {
        p_state0_5_time += deltaT;
        if (p_state0_5_time > 1.f)
            p_state0_5 = false;
    }

    for (unsigned int i = 0; i != 4; ++i)
    {
        if (p_playerstate[i].state_position)
        {
            p_playerstate[i].position_time += deltaT;

            if (p_playerstate[i].position_time > 1.5f)
            {
                p_playerstate[i].state_position = 0;
            }
        }
    }

    if (p_state == 1) { p_timesync.add_deltaT(deltaT);
        p_ghost_time += deltaT;
        for (unsigned int i = 0; i != p_players; ++i)
        {
            if (p_newlaprecordtxttime[i] > 0.f) {
                p_newlaprecordtxttime[i] -= deltaT;
            }
        }
    }
    p_particleTimesync.add_deltaT(deltaT);

    const float guiShift = p_wide169 ? 1.f : 0;

    while (p_timesync.step() && p_state == 1) // po částech času (0.01 s) se přepočítá herní stav
    {
        MA_periodicStream();

        // přepočet postupně všech hráčů (tzn. aut)
        for (unsigned int i = 0; i != p_players; ++i)
        {
            // vytvoření referencí na objekty konkrétního hráče, aby se nemuselo pořád indexovat
            Playerkeys& keys = p_playerkeys[i];
            Car2D& car2dp = p_car2dp[i];
            RBSolver& car2do = p_car2do[i];

            bool bvolnobeh = true;

            if (p_playerstate[i].state_finish) // pro hráče hra končí
            {
                if (p_playerstate[i].finish_time < 0.5f)
                {
                    bvolnobeh = false;
                    car2dp.go_forward();
                }
                else if (p_playerstate[i].finish_time < 3.5f)
                {
                    bvolnobeh = false;
                    car2dp.go_brake();
                }
                else {
                    // volnoběh
                    car2dp.go_zero();
                }
            } else { // hra je v běhu
                if (keys.key_up&&!keys.key_down) // cihla na plynu
                {
                    bvolnobeh = false;
                    car2dp.go_forward(); // přímá úprava veličin pevného tělesa - jeď dopředu, vnitřně se ošetřuje brzda
                }
                else if (!keys.key_up&&keys.key_down) // zpátečka
                {
                    bvolnobeh = false;
                    car2dp.go_backward(); // jeď dozadu, vnitřně se ošetřuje brzda
                }
                else if (keys.key_up&&keys.key_down) // brzda
                {
                    bvolnobeh = false;
                    car2dp.go_brake(); // brzdi
                }
                else if (!keys.key_up&&!keys.key_down) // není stisknuto nic, ani plyn ani brzda
                {
                    car2dp.go_zero(); // jeď na volnoběh
                }

                // zatáčení
                if (keys.key_left&&!keys.key_right)
                {
                    car2dp.steer_left();
                }
                else if (!keys.key_left&&keys.key_right)
                {
                    car2dp.steer_right();
                }
                else
                {
                    car2dp.steer_center();
                }
            }

            p_sound_car[i].p_brake_volume = (car2dp.add_sl_F()-40000.f)/40000.f; // na napojené pevné těleso působí síly tření kol
            if (p_sound_car[i].p_brake_volume < 0.f) p_sound_car[i].p_brake_volume = 0.f;
            if (p_sound_car[i].p_brake_volume > 1.f) p_sound_car[i].p_brake_volume = 1.f;

            car2dp.damp_engine_pitch(1.00015); // zpomalení tónu vrčení motoru - mělo by se projevit jen při volnoběhu
            if (bvolnobeh) // zpomalení při volnoběhu
            {
                car2do.damp_v(0.03*4); // opravit zrychlení není nutné, protože při jízdě se zpomalení neprojevuje
            }
            car2do.update(true); // přepočet rychlostí a poloh objektu auta změněných za časový krok
            car2dp.brake_end(); // pomocná funkce pro brzdu - při zpomalení až "do mínusu" se rychlosti nastaví na nulu

            // zatáčení předních kol (snad)
            if (RBf::len2(car2do.p_v) > 0.01)
            {
                float car2do_tm[2];
                car2do.get_tm(car2do_tm);
                float s = RBf::dprod(car2do.p_v, car2do_tm)*p_timesync.p_T;
                static const float s_mul = 360.f/(2.f*M_PI*p_carmodel[i]->p_cen[4*3+1]);
                p_wheel_rot[i] += s*s_mul;
                if (p_wheel_rot[i] > 360.f)
                    p_wheel_rot[i] -= 360.f;
                else if (p_wheel_rot[i] < -360.f)
                    p_wheel_rot[i] += 360.f;
            }
            // přepočet veličin kamery za časový krok
            p_carcam[i].update(true);

            if (p_playerstate[i].state_finish) // pro hráče hra končí
                p_playerstate[i].finish_time += p_timesync.p_T;
        }

        // jiné objekty než auta mohou jenom zpomalovat (nemají žádný pohon)
        for (unsigned int i = 0; i != p_mapobjs.size(); ++i)
        {
            Mapobj& mapobj = p_mapobjs[i];
            mapobj.rbo->damp_v(mapobj.f);
            mapobj.rbo->damp_av(mapobj.f, mapobj.r);
            mapobj.rbo->update(true);
        }

        // nakonec se testují kolize mezi objekty
        p_collider->test();

        // počítání kol
        for (unsigned int i = 0; i != p_players; ++i)
        {
            if (p_playerstate[i].state_finish)
                continue;
            // vytvoření referencí na objekty konkrétního hráče, aby se nemuselo pořád indexovat
            RBSolver& car2do = p_car2do[i];

            // výpočet současného stavu
            p_playerstate[i].state = 0;
            // pokud se nacházíme v cílové rovince
            if (car2do.p_x[0] < p_map_model->p_cen[3+2]+15.f
                && car2do.p_x[0] > p_map_model->p_cen[3+2]-15.f
                && car2do.p_x[1] < p_map_model->p_cen[3+0]+5.f
                && car2do.p_x[1] > p_map_model->p_cen[3+0]-5.f)
            {
                // zjisti pozici auta, jestli je před nebo za čárou

                {
                    float tm[2];
                    RBf::rot(tm, car2do.p_ax);
                    for (unsigned int i = 0; i != car2do.p_bbox_sz; ++i) // i zastiňuje vnější i
                    {
                        RBf::rot(car2do.p_bbox_rot[i].v, tm, car2do.p_bbox[i].v);
                        RBf::rot(car2do.p_bbox_rot[i].v1, tm, car2do.p_bbox[i].v1);
                        RBf::rot(car2do.p_bbox_rot[i].n1, tm, car2do.p_bbox[i].n1);
                        RBf::rot(car2do.p_bbox_rot[i].x, tm, car2do.p_bbox[i].x);
                        RBf::ladd(car2do.p_bbox_rot[i].x, car2do.p_x);
                    }
                }

                if (p_reverse)
                {
                    float pos_x = 100.f;
                    for (unsigned int j = 0; j != car2do.p_bbox_sz; ++j)
                    {
                        pos_x = std::min(pos_x, car2do.p_bbox_rot[j].x[0]);
                    }
                    if (pos_x > p_map_model->p_cen[3+2])
                    {
                        p_playerstate[i].state = 1;
                    } else {
                        p_playerstate[i].state = 2;
                    }
                } else {
                    float pos_x = -100.f;
                    for (unsigned int j = 0; j != car2do.p_bbox_sz; ++j)
                    {
                        pos_x = std::max(pos_x, car2do.p_bbox_rot[j].x[0]);
                    }
                    if (pos_x < p_map_model->p_cen[3+2]) // toto změnit
                    {
                        p_playerstate[i].state = 1;
                    } else {
                        p_playerstate[i].state = 2;
                    }
                }

                // porovnání stavu s předchozím (reakce na změny stavů před a za čárou)
                if (p_playerstate[i].state_prev == 1 && p_playerstate[i].state == 2)
                {
                    ++p_playerstate[i].lap_i;

                    int lap_i_max_prev = p_playerstate[i].lap_i_max;
                    p_playerstate[i].lap_i_max = std::max(p_playerstate[i].lap_i_max, p_playerstate[i].lap_i);
                    if (lap_i_max_prev != p_playerstate[i].lap_i_max)
                    {
                        if (/*p_isGhost*/true) { // condemned
                            if (p_playerstate[i].lap_i_max > 1) {
                                p_ghostNew[i].m_seconds = p_playerstate[i].lap_time;
                                if (!p_ghostAvailable || p_ghostNew[i].m_seconds < p_ghostOld->m_seconds) {
                                    p_ghostOld->copyFrom(p_ghostNew[i]);
                                    p_ghostNew[i].save();
                                    {
                                        const float newLapRecordTextViewTime = 10.08; //5;
                                        p_newlaprecordtxttime[i] = newLapRecordTextViewTime;
                                    }
                                    p_ghostUpdated = 1;
                                    p_ghostAvailable = 1;
                                }
                            }
                            p_ghostNew[i].m_num = 0;
                            p_ghost_step[i] = 0; // předchod na nové kolo -> reset ghost stepů
                        }

                        p_playerhud[i].laptime_l_best.puts(0, time_m_s(p_playerstate[i].lap_time));
                        if (p_playerstate[i].lap_i_max < 2)
                        {
                            p_playerstate[i].best_time = 0.f;
                        }
                        else if (p_playerstate[i].lap_i_max == 2)
                        {
                            p_playerstate[i].best_time = p_playerstate[i].lap_time;
                        } else {
                            p_playerstate[i].best_time = std::min(p_playerstate[i].best_time, p_playerstate[i].lap_time);
                        }
                        p_playerhud[i].laptime_l_best.puts(1, time_m_s(p_playerstate[i].best_time));

                        p_playerstate[i].lap_time = 0.f;
                        p_ghost_time = 0.f;
                        // další věci k časům na kolo
                        static char buff[256] = {0};
                        snprintf(buff, 255, "%d/%d", std::min(p_playerstate[i].lap_i_max, p_laps), p_laps);
                        p_playerhud[i].laptime_r1.puts(0, buff);

                        unsigned int delka_buff = strlen(buff);
                        if (delka_buff <= 3)
                        {
                            p_playerhud[i].laptime_r_lap.set_pos(15.9f, 14.f - guiShift);
                        }
                        else if (delka_buff == 4)
                        {
                            p_playerhud[i].laptime_r_lap.set_pos(14.6f, 14.f - guiShift);
                        }
                        else
                        {
                            p_playerhud[i].laptime_r_lap.set_pos(13.3f, 14.f - guiShift);
                        }

                        if (p_laps < p_playerstate[i].lap_i_max)
                        {
                            p_playerstate[i].state_finish = 1;
                            ++p_finished;
                        }

                        {
                            unsigned int position = 0;
                            for (unsigned int j = 0; j != p_players; ++j)
                            {
                                if (p_playerstate[i].lap_i_max <= p_playerstate[j].lap_i_max)
                                    ++position;
                            }
                            --position;
                            if (p_playerstate[i].lap_i_max > 1)
                            {
                                switch (position)
                                {
                                case 0:
                                    if (p_players == 1)
                                        p_playerhud[i].position.puts(0, "Finished");
                                    else
                                        p_playerhud[i].position.puts(0, "1st");
                                    break;
                                case 1:
                                    p_playerhud[i].position.puts(0, "2nd");
                                    break;
                                case 2:
                                    p_playerhud[i].position.puts(0, "3rd");
                                    break;
                                case 3:
                                    p_playerhud[i].position.puts(0, "4th");
                                    break;
                                default:
                                    p_playerhud[i].position.puts(0, "");
                                }

                                p_playerstate[i].position_time = 0.f;
                                p_playerstate[i].state_position = 1;
                            } else {
                                p_playerhud[i].position.puts(0, "");
                            }
                        }
                    }
                }
                else if (p_playerstate[i].state_prev == 2 && p_playerstate[i].state == 1)
                {
                    --p_playerstate[i].lap_i;
                }
            }

            // zapamatování si aktuálního stavu pro další krok
            p_playerstate[i].state_prev = p_playerstate[i].state;

            p_playerstate[i].lap_time += p_timesync.p_T;
            p_playerstate[i].race_time += p_timesync.p_T;
        }
        if (p_finished == int(p_players)) // pokud se rovná počet skončených hráčů celkovému počtu hráčů
        {
            bool bkonec = true;
            for (unsigned int i = 0; i != p_players; ++i)
            {
                if (p_playerstate[i].finish_time < 5.f)
                    bkonec = false;
            }
            if (bkonec)
            {
                end_race();
                // tady vytvořit zobrazení výsledků
            }
        }

        for (unsigned int i = 0; i != p_players; ++i)
        {
            // TODO: test, zda se používá ghost
            if (true /*p_isGhost*/ /*condemned*/ && p_ghost_step[i] % 10 == 0 && p_playerstate[i].lap_i_max > 0) {

                if (p_ghostNew[i].m_num < p_ghostNew[i].m_maxnum) {
                    p_ghostNew[i].m_frames[p_ghostNew[i].m_num*4+0] = p_car2do[i].p_x[0];
                    p_ghostNew[i].m_frames[p_ghostNew[i].m_num*4+1] = p_car2do[i].p_x[1];
                    p_ghostNew[i].m_frames[p_ghostNew[i].m_num*4+2] = p_car2do[i].p_ax;
                    p_ghostNew[i].m_frames[p_ghostNew[i].m_num*4+3] = p_car2dp[i].p_whl_a_ren;
                    ++p_ghostNew[i].m_num;
                }
            }
            ++(p_ghost_step[i]);
        }

    }

    if (p_particles)
    {
        while (p_particleTimesync.step()) // po částech času (0.01 s) se přepočítají částice
        {
            MA_periodicStream();
            // přepočet částic postupně u všech hráčů (tzn. aut)
            for (unsigned int i = 0; i != p_players; ++i)
            {
                p_particles[i].step(p_timesync.p_T);
                int randValue = rand();
                bool generateNewParticle = false;
                bool lessParticles = !p_car2dp[i].p_b_acc;
                if (lessParticles)
                {
                    generateNewParticle =
                        //p_particles[i].m_stepCounter % 10 == 0;
                        randValue < RAND_MAX * 0.1f;
                }
                else
                {
                    generateNewParticle =
                        p_particles[i].m_stepCounter % 5 == 0;
                    //randValue < RAND_MAX * 0.2f;
                }
                if (generateNewParticle)
                {
                    glPushMatrix(); checkGL();
                    glLoadIdentity(); checkGL();
                    glRotatef(p_car2do[i].p_ax*57.29577951308232, 0, 1, 0); checkGL();
                    float mdl_mtrx[16];
                    glGetFloatv(GL_MODELVIEW_MATRIX, mdl_mtrx); checkGL();
                    glPopMatrix(); checkGL();
                    float exhaustPosition[3];
                    multMatPos(exhaustPosition, mdl_mtrx, p_cars[p_cars_sel[i]].exhaust_position);
                    float exhaustDirection[3];
                    multMatDir(exhaustDirection, mdl_mtrx, p_cars[p_cars_sel[i]].exhaust_direction);
                    float exhaustExitSpeed = lessParticles ? 0.9f : 1.1f;
                    float densityMultiplier = lessParticles ? 0.5f : 1.f;
                    p_particles[i].newParticle(Particle(exhaustPosition[0], exhaustPosition[1], exhaustPosition[2],
                                                        exhaustDirection[0]*exhaustExitSpeed - 0.1f + 0.2f*static_cast<float>(rand())/RAND_MAX,
                                                        exhaustDirection[1]*exhaustExitSpeed - 0.1f + 0.2f*static_cast<float>(rand())/RAND_MAX,
                                                        exhaustDirection[2]*exhaustExitSpeed - 0.1f + 0.2f*static_cast<float>(rand())/RAND_MAX,
                                                        rand() % 4, 0.04, 0.3,
                                                        (0.55 + 0.1f*static_cast<float>(rand())/RAND_MAX) * densityMultiplier, 0.6 * densityMultiplier));
                }
            }
        }
    }

    // interpolace polohy auta pomocí hodnoty času uvnitř časového kroku
    // (aby nebyl pohyb trhaný po časových krocích)
    for (unsigned int i = 0; i != p_players; ++i)
    {
        p_car2do[i].update(false); // poloha auta
        p_carcam[i].update(false); // poloha kamery
    }

    // interpolace polohy objektů pomocí hodnoty času uvnitř časového kroku
    for (unsigned int i = 0; i != p_mapobjs.size(); ++i)
    {
        p_mapobjs[i].rbo->update(false);
    }

    if (p_state == 0)
    {
        p_state0_time += deltaT;
        for (unsigned int i = 0; i != p_players; ++i)
        {
            if (p_state0_time < 1.75)
            {
                p_startcam[i].p_r0 = -p_state0_time*15/2+21;
            }
            else if (p_state0_time < 2.25)
            {
                p_startcam[i].p_r0 = -0.5968310365946076f*cosf(2.f*M_PI*p_state0_time)-3.75f*p_state0_time+14.4375f;
            }
            else
            {
                p_startcam[i].p_r0 = 6.f;
            }

            if (p_state0_time < 1.5)
            {
                p_startcam[i].p_ang0 = M_PI;
            }
            else if (p_state0_time < 2.f)
            {
                p_startcam[i].p_ang0 = -0.25f*sinf(2.f*M_PI*p_state0_time)-M_PI*0.5f*p_state0_time+1.75f*M_PI;
            }
            else if (p_state0_time < 2.5f)
            {
                p_startcam[i].p_ang0 = -(p_state0_time+0.25f)*M_PI+M_PI*3.f;
            }
            else if (p_state0_time < 3.f)
            {
                p_startcam[i].p_ang0 = 0.25f*sinf(2.f*M_PI*p_state0_time)-M_PI*0.5f*p_state0_time+1.5f*M_PI;
            }
            else
            {
                p_startcam[i].p_ang0 = 0.f;
            }
            p_startcam[i].p_ang0 *= -1.f;
            if (p_reverse)
                p_startcam[i].p_ang0 += M_PI;

        }
    }

    MA_periodicStream();

    if (p_isGhost) // rendering
    {
        int i = p_ghostUpdated;
        {
            float wheel_angle = 0;
            float wheel_rot = p_ghost_wheel_rot*200;

            float framef = p_playerstate[0].lap_time*10.0;
            int framei = framef;
            float framej = framef - framei;
            float framej1 = 1-framej;
            int framei1 = framei+1;
            if (framei1 < p_ghostOld->m_num) {
                wheel_angle = p_ghostOld->m_frames[framei*4+3]*framej1+p_ghostOld->m_frames[framei1*4+3]*framej;
            }

            float transf_mwmx[16];
            unsigned int i_cen;
            glPushMatrix(); checkGL();
            glLoadIdentity(); checkGL();
            i_cen = 2;
            glTranslatef(p_ghostmodel[i].p_cen[i_cen*3+0], p_ghostmodel[i].p_cen[i_cen*3+1], p_ghostmodel[i].p_cen[i_cen*3+2]); checkGL();
            glRotatef(wheel_angle*57.29577951308232f, 0, 1, 0); checkGL();
            glRotatef(wheel_rot, 1, 0, 0); checkGL();
            glGetFloatv(GL_MODELVIEW_MATRIX, transf_mwmx); checkGL();
            p_ghosttransf[i].set_mwmx(transf_mwmx, 2);

            glLoadIdentity(); checkGL();
            i_cen = 3;
            glTranslatef(p_ghostmodel[i].p_cen[i_cen*3+0], p_ghostmodel[i].p_cen[i_cen*3+1], p_ghostmodel[i].p_cen[i_cen*3+2]); checkGL();
            glRotatef(wheel_angle*57.29577951308232f, 0, 1, 0); checkGL();
            glRotatef(wheel_rot, 1, 0, 0); checkGL();
            glGetFloatv(GL_MODELVIEW_MATRIX, transf_mwmx); checkGL();
            p_ghosttransf[i].set_mwmx(transf_mwmx, 3);

            glLoadIdentity(); checkGL();
            i_cen = 4;
            glTranslatef(p_ghostmodel[i].p_cen[i_cen*3+0], p_ghostmodel[i].p_cen[i_cen*3+1], p_ghostmodel[i].p_cen[i_cen*3+2]); checkGL();
            glRotatef(wheel_rot, 1, 0, 0); checkGL();
            glGetFloatv(GL_MODELVIEW_MATRIX, transf_mwmx); checkGL();
            p_ghosttransf[i].set_mwmx(transf_mwmx, 4);
            glPopMatrix(); checkGL();
        }
    }

    for (unsigned int i = 0; i != p_players; ++i) // vytvoření transformačních matic kol
    {
        float transf_mwmx[16];
        unsigned int i_cen;
        glPushMatrix(); checkGL();
        glLoadIdentity(); checkGL();
        i_cen = 2;
        glTranslatef(p_carmodel[i]->p_cen[i_cen*3+0], p_carmodel[i]->p_cen[i_cen*3+1], p_carmodel[i]->p_cen[i_cen*3+2]); checkGL();
        glRotatef(p_car2dp[i].p_whl_a_ren*57.29577951308232f, 0, 1, 0); checkGL();
        glRotatef(p_wheel_rot[i], 1, 0, 0); checkGL();
        glGetFloatv(GL_MODELVIEW_MATRIX, transf_mwmx); checkGL();
        p_cartransf[i].set_mwmx(transf_mwmx, 2);

        glLoadIdentity(); checkGL();
        i_cen = 3;
        glTranslatef(p_carmodel[i]->p_cen[i_cen*3+0], p_carmodel[i]->p_cen[i_cen*3+1], p_carmodel[i]->p_cen[i_cen*3+2]); checkGL();
        glRotatef(p_car2dp[i].p_whl_a_ren*57.29577951308232f, 0, 1, 0); checkGL();
        glRotatef(p_wheel_rot[i], 1, 0, 0); checkGL();
        glGetFloatv(GL_MODELVIEW_MATRIX, transf_mwmx); checkGL();
        p_cartransf[i].set_mwmx(transf_mwmx, 3);

        glLoadIdentity(); checkGL();
        i_cen = 4;
        glTranslatef(p_carmodel[i]->p_cen[i_cen*3+0], p_carmodel[i]->p_cen[i_cen*3+1], p_carmodel[i]->p_cen[i_cen*3+2]); checkGL();
        glRotatef(p_wheel_rot[i], 1, 0, 0); checkGL();
        glGetFloatv(GL_MODELVIEW_MATRIX, transf_mwmx); checkGL();
        p_cartransf[i].set_mwmx(transf_mwmx, 4);
        glPopMatrix(); checkGL();
    }

    // vykreslení jednotlivých okének rozdělené obrazovky
    if (p_state != 2)
    {
    for (unsigned int i = 0; i != p_players; ++i)
    {
        MA_periodicStream();
        set_scissor(i); // nastaví se podokénko (nemusí zabírat celou plochu hlavního SDL okna
        glPushMatrix(); checkGL();
        // transformace podle kamery a vykreslení
        if (!g_freecam)
        {
            if (p_state == 1)
            {
                p_carcam[i].transf();
            }
            else
            {
                p_startcam[i].transf();
            }
        }
        render_frame();
        glPopMatrix(); checkGL();

        glEnable(GL_TEXTURE_2D); checkGL();
        glDisable(GL_LIGHTING); checkGL();
        glEnable(GL_BLEND); checkGL();
        glDisable(GL_DEPTH_TEST); checkGL();
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); checkGL();
        glColor4f(1, 1, 1, 1); checkGL();

        if (!g_freecam)
        {
            char speedbuff[256] = {0};
            snprintf(speedbuff, 255, "%.0f", (RBf::len(p_car2do[i].p_v)*3.6f));
            p_playerhud[i].speed.puts(0, speedbuff);

            p_playerhud[i].speed.render(p_fonttex);

            p_playerhud[i].speed_km_h.render(p_fonttex);

            p_playerhud[i].laptime_l.render(p_fonttex); // Prev Best Race
            p_playerhud[i].laptime_l1.puts(0, time_m_s(p_playerstate[i].race_time));
            p_playerhud[i].laptime_l1.render(p_fonttex);

            if (p_playerstate[i].lap_i_max > 0)
            {
                if (!p_playerstate[i].state_finish)
                {
                    p_playerhud[i].laptime_r.puts(0, time_m_s(p_playerstate[i].lap_time));
                    p_playerhud[i].laptime_r.render(p_fonttex); // aktualni casomira kola
                    p_playerhud[i].laptime_r_lap.render(p_fonttex); // Lap
                    p_playerhud[i].laptime_r1.render(p_fonttex); // 1/5
                }
            }

            if (p_playerstate[i].state_finish)
            {
                glColor4f(1, 1, 1, 1); checkGL();
                p_playerhud[i].position.render(p_fonttex);
            }

            glColor4f(1, 1, 1, 1); checkGL();

            if (p_playerstate[i].lap_i_max >= 2)
                p_playerhud[i].laptime_l_best.render(p_fonttex);

            if (/*p_isGhost*/true) { // condemned
                if (p_newlaprecordtxttime[i] > 0.f)
                    p_playerhud[i].newrecord.render(p_fonttex);
            }
        }

        float scalepom = 7.f;
        if (p_state == 0)
        {
            if (p_state0_time < 3.f)
            {
            }
            else if (p_state0_time < 4.f)
            {
                p_gltext_start.puts(0, "3");
                float pom = p_state0_time - 3.f;
                glColor4f(1, 1, 1, 1.f-pom); checkGL();

                p_gltext_start.renderscale((0.1f+pom)*scalepom, p_fonttex);
            }
            else if (p_state0_time < 5.f)
            {
                p_gltext_start.puts(0, "2");
                float pom = p_state0_time - 4.f;
                glColor4f(1, 1, 1, 1.f-pom); checkGL();

                p_gltext_start.renderscale((0.1f+pom)*scalepom, p_fonttex);
            }
            else if (p_state0_time < 6.f)
            {
                p_gltext_start.puts(0, "1");
                float pom = p_state0_time - 5.f;
                glColor4f(1, 1, 1, 1.f-pom); checkGL();

                p_gltext_start.renderscale((0.1f+pom)*scalepom, p_fonttex);
            }
        }

        if (p_state0_5 && p_state == 1)
        {
            p_gltext_start.puts(0, "GO!");
            glColor4f(1, 1, 1, 1.f-p_state0_5_time); checkGL();

            p_gltext_start.renderscale(3.5f, p_fonttex);
        }

        glEnable(GL_LIGHTING); checkGL();
        glDisable(GL_BLEND); checkGL();
        glEnable(GL_DEPTH_TEST); checkGL();
        glColor4f(1, 1, 1, 1); checkGL();

        // tady vypsat testovací hodnoty při testu kláves
        if (p_gamemenu.bmenu && p_gamemenu.state == GMSTATE_T)
        {
            glDisable(GL_DEPTH_TEST); checkGL();
            glDisable(GL_LIGHTING); checkGL();
            glDisable(GL_TEXTURE_2D); checkGL();
            glEnable(GL_BLEND); checkGL();

            render_black();

            glEnable(GL_TEXTURE_2D); checkGL();
            glEnable(GL_BLEND); checkGL();

            float font_color0[4] = {0.6, 0.6, 0.6, 1};
            float font_color1[4] = {1, 1, 1, 1};
            p_keytest[i].player.render_c();

            if (p_playerkeys[i].key_left)
                p_keytest[i].left.set_color(0, font_color1);
            else
                p_keytest[i].left.set_color(0, font_color0);
            p_keytest[i].left.render_c();

            if (p_playerkeys[i].key_right)
                p_keytest[i].right.set_color(0, font_color1);
            else
                p_keytest[i].right.set_color(0, font_color0);
            p_keytest[i].right.render_c();

            if (p_playerkeys[i].key_down)
                p_keytest[i].down.set_color(0, font_color1);
            else
                p_keytest[i].down.set_color(0, font_color0);
            p_keytest[i].down.render_c();

            if (p_playerkeys[i].key_up)
                p_keytest[i].up.set_color(0, font_color1);
            else
                p_keytest[i].up.set_color(0, font_color0);
            p_keytest[i].up.render_c();

            glEnable(GL_LIGHTING); checkGL();
            glDisable(GL_BLEND); checkGL();
            glEnable(GL_DEPTH_TEST); checkGL();
        }


    }
    /*if (p_players == 3)
    {
        set_scissor(3); // 4. okno
        glClear(GL_COLOR_BUFFER_BIT); checkGL();
    }*/
    }

    // nastavení výchozího okna (přes celé) a vypnutí scissor testu
    unset_scissor();

    glDisable(GL_DEPTH_TEST); checkGL();
    glDisable(GL_LIGHTING); checkGL();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); checkGL();

    if (p_state == 2)
    {
        glEnable(GL_TEXTURE_2D); checkGL();
        glDisable(GL_BLEND); checkGL();
        render_bricks();

        ///////////////

        glEnable(GL_BLEND); checkGL();
        // tady kreslit text výsledků
        p_results.title.render_c();
        p_results.line0.render_c();
        p_results.position.render_c();
        p_results.time.render_c();
        p_results.best.render_c();
    }

    if (p_gamemenu.bmenu && p_gamemenu.state != GMSTATE_T)
    {
        glDisable(GL_DEPTH_TEST); checkGL();
        glDisable(GL_LIGHTING); checkGL();
        glDisable(GL_TEXTURE_2D); checkGL();
        glEnable(GL_BLEND); checkGL();
        render_black();

        glEnable(GL_TEXTURE_2D); checkGL();
        glDisable(GL_LIGHTING); checkGL();


        // tady kreslit menu

        p_gamemenu.render();


    }

    glEnable(GL_BLEND); checkGL();
    glEnable(GL_TEXTURE_2D); checkGL();
    glColor4f(1, 1, 1, 1); checkGL();
    if (p_bfps && p_state != 2)
        p_gltext_fps.render(p_fonttex);
    glEnable(GL_LIGHTING); checkGL();
    glDisable(GL_BLEND); checkGL();
    glEnable(GL_DEPTH_TEST); checkGL();

    if (p_state0_time >= 6.f && p_state == 0)
        p_state = 1;
    // a nakonec zvuky
    for (unsigned int i = 0; i != p_players; ++i)
    {
        int engine_state = 0; // zvuk je na začátku idle
        if (RBf::len(p_car2do[i].p_v)*3.6f < 2.f) // pokud je rychlost menší než 2 km/h, tak je motor idle
        {
            engine_state = 0;
        } else { // jinak motor jede
            if (p_car2dp[i].p_b_acc) engine_state = 1; else engine_state = 2; // 1 - plyn, 2 - volnoběh
        }
        // zpracování zvuků
        if (p_state != 2)
            p_sound_car[i].frame(deltaT, engine_state, p_car2dp[i].get_engine_pitch(), p_car2do[i].p_v);
    }
}

void Gamemng::render_black()
{
    glPushMatrix(); checkGL();

    glLoadIdentity(); checkGL();
    glColor4f(0, 0, 0, 0.8); checkGL(); // ztmavení obrazovky černým poloprůhledným čtvercem
    glEnableClientState(GL_VERTEX_ARRAY); checkGL();
    static const float vert_array[12] = {-20, -10, -10,  20, -10, -10,  20,  10, -10, -20,  10, -10};
    glVertexPointer(3, GL_FLOAT, 0, vert_array); checkGL();
    glDrawArrays(GL_QUADS, 0, 4); checkGL(); afterDrawcall();
    glDisableClientState(GL_VERTEX_ARRAY); checkGL();

    glPopMatrix(); checkGL();
}

void Gamemng::render_bricks()
{
    glPushMatrix(); checkGL();


    glBindTexture(GL_TEXTURE_2D, 0
                  ); checkGL();
    glLoadIdentity(); checkGL();
    static const float seda = 1.f;
    glColor4f(seda, seda, seda, 1); checkGL(); // vykreslení pozadí s texturou cihel
    glEnableClientState(GL_VERTEX_ARRAY); checkGL();
    glEnableClientState(GL_COLOR_ARRAY); checkGL();
    static const float vert_array[12] = {-10, -10, -10,  10, -10, -10,  10,  10, -10, -10,  10, -10};

    static const float color_array[12] = {
        0, 1, 0.2,
        0, 1, 0.2,
        1, 0, 0.2,
        1, 0, 0.2,
    };
    glVertexPointer(3, GL_FLOAT, 0, vert_array); checkGL();
    glColorPointer(3, GL_FLOAT, 0, color_array);
    glDrawArrays(GL_QUADS, 0, 4); checkGL(); afterDrawcall();
    glDisableClientState(GL_VERTEX_ARRAY); checkGL();
    glDisableClientState(GL_COLOR_ARRAY); checkGL();

    glPopMatrix(); checkGL();
}

void Gamemng::render_black_background()
{
    glPushMatrix(); checkGL();

    glBindTexture(GL_TEXTURE_2D, 0); checkGL();
    glLoadIdentity(); checkGL();
    static const float seda = 0.f;
    glColor4f(seda, seda, seda, 1); checkGL(); // vykreslení pozadí s texturou cihel
    glEnableClientState(GL_VERTEX_ARRAY); checkGL();
    static const float vert_array[12] = {-10, -10, -10,  10, -10, -10,  10,  10, -10, -10,  10, -10};
    glVertexPointer(3, GL_FLOAT, 0, vert_array); checkGL();
    glDrawArrays(GL_QUADS, 0, 4); checkGL(); afterDrawcall();
    glDisableClientState(GL_VERTEX_ARRAY); checkGL();
    glPopMatrix(); checkGL();
}
