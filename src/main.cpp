#include "appdefs.h"

#include <cstdio> // ve windows výstup na konzoli funguje až po zavolání ActivateConsole
#include <algorithm>
#include <vector>
#include <set>

#include <dos.h>
#include <dpmi.h>
#include <go32.h>

#include "minisdl.h"
#include <GL/gl.h>
#include <GL/dmesa.h>
#include "minial.h"
#include "doskeyb.h"

#include "gltext.h"
#include "skysph.h"

#define NO_CAMERA
//#undef NO_CAMERA

//#define TESTING_SLOWDOWN 1

#ifndef NO_CAMERA
#include "cam.h"
#endif

//SDL_Keycode k;

#include "pict2.h"

#include "matmng.h"

#include "datkey.h"

#include "cstring1.h"

#include "gamemng.h"

#include "rand1.h"

#include "mtrxinv.h"

#include "gbuff_in.h"

#include "load_texture.h"

#include "settings_dat.h"

#include "controls.h"

#include "mainmenu.h"

//#include "settingsdialog.h"

#include "fopendir.h"

#include "bits.h"

#ifdef __MACOSX__
#include <CoreFoundation/CFBundle.h>
#endif

std::vector<ALuint> global_al_sources;
std::vector<ALuint> global_al_buffers;

_go32_dpmi_seginfo OldISR, NewISR;

#define TIMER 8

volatile uint8_t timerCounter = 0;

#define LOCK_VARIABLE(x)    _go32_dpmi_lock_data((void *)&x,(long)sizeof(x));
#define LOCK_FUNCTION(x)    _go32_dpmi_lock_code((void*)x,(long)x##_end - (long)x);
#define END_OF_FUNCTION(x)    void x##_end(void) { }

void TickHandler(void)
{
    ++timerCounter;
}
END_OF_FUNCTION(TickHandler)

void timer_set(uint16_t prm)
{
    if (prm > 0 && prm < 150)
        return;

    outportb(0x43,0x36);
    outportb(0x40,prm);
    outportb(0x40,prm >> 8);
}

void timer_init(uint16_t count)
{
    LOCK_FUNCTION(TickHandler);
    LOCK_VARIABLE(timerCounter);

    _go32_dpmi_get_protected_mode_interrupt_vector(TIMER, &OldISR);

    NewISR.pm_offset = (int)TickHandler;
    NewISR.pm_selector = _go32_my_cs();

    _go32_dpmi_chain_protected_mode_interrupt_vector(TIMER, &NewISR);

    timer_set(count);
}

void timer_quit(void)
{
    _go32_dpmi_set_protected_mode_interrupt_vector(TIMER, &OldISR);
}

static uint8_t cntPrev = 0;
static uint8_t cntNow = 0;

float getdeltaT()
{
    cntNow = timerCounter; // dostodo
    uint8_t diff = cntNow - cntPrev;
    cntPrev = cntNow;
    return diff * 0.01f;
}

// odkazy na různá místa projektu:
// tření o zeď
// odrazivost při kolizích
// magický vzorec pro výpočet útlumu při nárazu
// další věci k časům na kolo

int g_mipmaps_available = 0;

//std::vector<JoystickDevice>* g_joystickDevices = 0;

void my_exit(int ret, bool callExit)
{
    /*for (unsigned int i = 0; i < g_joystickDevices->size(); ++i)
    {
        if (SDL_JoystickOpened(i))
            (*g_joystickDevices)[i].close();
    }*/
    //SDL_Quit();
    if (callExit)
    {
        exit(ret);
    }
}

int n_klavesy = 0; // počet stisknutých kláves

extern int ge_bpass1; // globální vypnutí aktualizace pohledu

extern int g_freecam;

extern int g_sound_on;

GLuint g_ghost_tex = 0;
int g_ghost_w = 0;
int g_ghost_h = 0;

//int g_multisampleMode = 0; // 0 - 0ff, 1 - 2x, 2 - 4x

void saveTgaScreenshot()
{
    static int screenshotNumber = 0;
    static std::vector<unsigned char> pixelbuffer;
    GLint okno_rozmery[4];
    glGetIntegerv(GL_VIEWPORT, okno_rozmery); checkGL();
    pixelbuffer.resize(okno_rozmery[2]*okno_rozmery[3]*3);
    //okno_rozmery[2] = (okno_rozmery[2] / 4) * 4; // fix for a 4-byte padding issue // better fix is a 1-byte pixel packing
    if (okno_rozmery[2]*okno_rozmery[3] == 0)
        return;
    glReadPixels(0,  0,  okno_rozmery[2], okno_rozmery[3], GL_BGR, GL_UNSIGNED_BYTE, &(pixelbuffer[0])); checkGL();
    char filename[256] = {0};
    snprintf(filename, 255, "omscr%.3d.tga", screenshotNumber);
    //FILE* fout = fopen(filename, "wb");
    FILE* fout = fopenDir(filename, "wb");
    //fprintf(stderr, "%s_%s\n", __PRETTY_FUNCTION__, filename);
    if (fout != NULL) {
        unsigned char tgaheader[] = {
            0, //0  1 byte  IDLength    velikost obrazového identifikátoru
            0, //1  1 byte  ColorMapType    typ barevné mapy
            2, //2  1 byte  ImageType   typ obrázku
            0,0, //3    2 byte  CMapStart   počátek barevné palety
            0,0, //5    2 byte  CMapLength  délka barevné palety
            0, //7  1 byte  CMapDepth   bitová hloubka položek barevné palety
            0,0, //8    2 byte  XOffset     X-ová souřadnice počátku obrázku
            0,0 //10    2 byte  YOffset     Y-ová souřadnice počátku obrázku
            };
        unsigned short tgaheader2[] = {
            (unsigned short)okno_rozmery[2],//12     2 byte  Width   šířka obrázku uvedená v pixelech
            (unsigned short)okno_rozmery[3]//14  2 byte  Height  výška obrázku uvedená v pixelech
            };
        unsigned char tgaheader3[] = {
            24, //16    1 byte  PixelDepth  počet bitů na jeden pixel (bitová hloubka)
            0 //17  1 byte  ImageDescriptor     popisovač obrázku
            };
        fwrite(tgaheader, 1, sizeof(tgaheader), fout);
        fwrite(tgaheader2, 1, sizeof(tgaheader2), fout);
        fwrite(tgaheader3, 1, sizeof(tgaheader3), fout);
        fwrite(&(pixelbuffer[0]), 1, okno_rozmery[2]*okno_rozmery[3]*3, fout);

        fclose(fout);
    }
    ++screenshotNumber;
}

#define CAMERA_KEY_COUNT 10

static DMesaVisual dv;
static DMesaContext dc;
static DMesaBuffer db;

static bool dmesaInit = false;

static void gfx_dos_init_impl(int w, int h)
{
    int configScreenWidth = w;
    int configScreenHeight = h;

    dmesaInit = true;

    dv = DMesaCreateVisual(configScreenWidth, configScreenHeight, 16, 60, 1, 1, 0, 16, 0, 0);
    if (!dv) {
        printf("DMesaCreateVisual failed: resolution not supported?\n");
        abort();
    }

    dc = DMesaCreateContext(dv, NULL);
    if (!dc) {
        printf("DMesaCreateContext failed\n");
        abort();
    }

    db = DMesaCreateBuffer(dv, 0, 0, configScreenWidth, configScreenHeight);
    if (!db) {
        printf("DMesaCreateBuffer failed\n");
        abort();
    }

    DMesaMakeCurrent(dc, db);
}

static void gfx_dos_shutdown_impl(void)
{
    if (dmesaInit)
    {
        DMesaMakeCurrent(NULL, NULL);
        DMesaDestroyBuffer(db); db = NULL;
        DMesaDestroyContext(dc); dc = NULL;
        DMesaDestroyVisual(dv); dv = NULL;
    }
}

int my_main (int argc, char** argv)
{
    (void)argc; (void)argv;
    /*{
     // bits_crop_npot unit test
        unsigned int u = 257;
        unsigned int uc = bits_crop_npot(u);
        printf("%u %u\n", u, uc);
    }*/

    /*bool skipSettings = false;
    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "--skip-settings") == 0)
        {
            skipSettings = true;
        }
        else if (strcmp(argv[i], "--test-stderr") == 0)
        {
            fprintf(stderr, "stderr test output\n");
            fflush(stderr);
        }
    }*/

    /*static std::vector<JoystickDevice> joystickDevices;
    g_joystickDevices = &joystickDevices;
    std::vector<JoystickIdentifier> joystickNotConnectedDevices;*/

    Control controls[16] = {
        Control(SDLK_UP),
        Control(SDLK_DOWN),
        Control(SDLK_LEFT),
        Control(SDLK_RIGHT),

        Control(),
        Control(),
        Control(),
        Control(),

        Control(),
        Control(),
        Control(),
        Control(),

        Control(),
        Control(),
        Control(),
        Control(),
    };

    Settings settings("settings.dat", /*&joystickDevices, &joystickNotConnectedDevices,*/ controls);
    if (settings.load())
    {
        // make sure the settings.dat file exists after first run
        settings.save();
    }

    keyb_hook_int();

    timer_init(315000000/22/12/100);

    int sound_quality = settings.get("sound_quality");

    int ma_freq = 11025 * sound_quality;
    if (ma_freq == 0)
    {
        ma_freq = 22050;
    }

    // initialize SDL video
    /*if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0)
    {
        fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
        return 1;
    }*/

    /*{
        int numJoysticks = SDL_NumJoysticks();
        if (numJoysticks)
        {
            joystickDevices.resize(numJoysticks);
            for (int i = 0; i < numJoysticks; ++i)
                joystickDevices[i].open(i);
            SDL_JoystickEventState(SDL_ENABLE);
        }
    }
    initializeParametersIndices(joystickDevices);*/

    gfx_dos_init_impl(settings.get("screen_x"), settings.get("screen_y"));

    static const char aldeviceNone[] = "none";
    static const char aldeviceSB[] = "sb";
    static const char aldeviceGUS[] = "gus";

    const char* aldevicestr = 0;
    switch (settings.get("sound_device"))
    {
    case 0:
        aldevicestr = aldeviceNone;
        break;
    case 1:
        aldevicestr = aldeviceSB;
        break;
    case 2:
        aldevicestr = aldeviceGUS;
        break;
    default:
        break;
    }

    glClearColor(0, 0, 0, 0);

    // Initialize Open AL

    ALCdevice* aldevice = alcOpenDevice(aldevicestr); // NULL parameter = open default device
    ALCcontext* alcontext = NULL;
    if (aldevice != NULL) {

        static const ALCint attribs[] = {
            ALC_FREQUENCY, ma_freq,
            0, 0
        };

        alcontext = alcCreateContext(aldevice, attribs); // create context
        if (alcontext != NULL) {
            alcMakeContextCurrent(alcontext); // set active context
        } else {
            fprintf(stderr, "%s", "Error: Can't create OpenAL context\n");
        }
    } else {
        if (aldevicestr == NULL)
            fprintf(stderr, "Error: Can't open default OpenAL device");
        else
            fprintf(stderr, "Error: Can't open OpenAL device \"%s\"\n", aldevicestr);
    }

    ALfloat listenerPos[] = { 0.0, 0.0, 0.0 };
    ALfloat listenerVel[] = { 0.0, 0.0, 0.0 };
    ALfloat listenerOri[] = { 0.0, 0.0, -1.0,  0.0, 1.0, 0.0 };
    alListenerfv(AL_POSITION,    listenerPos);
    alListenerfv(AL_VELOCITY,    listenerVel);
    alListenerfv(AL_ORIENTATION, listenerOri);

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    GLint okno_rozmery[4];
    glGetIntegerv(GL_VIEWPORT, okno_rozmery); checkGL();

    /* Make sure we're chaning the model view and not the projection */
    glMatrixMode( GL_MODELVIEW ); checkGL();

    // nastavení úvodních materiálů a zobrazení
    float material1[4] = {1, 1, 1, 1};
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, material1); checkGL();
    float lightspec0[4] = {0, 0, 0, 1};
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightspec0); checkGL();
    // důležité vypnutí defaultního rozptýleného světla
    float lightmodelamb0[4] = {0, 0, 0, 1};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lightmodelamb0); checkGL();

    glEnable(GL_CULL_FACE); checkGL();
    glEnable(GL_DEPTH_TEST); checkGL();
    glDepthFunc(GL_LESS); checkGL();
    glAlphaFunc(GL_GREATER, 0.5); checkGL();
    glEnable(GL_LIGHT0); checkGL();
    glEnable(GL_LIGHTING); checkGL();

    glDepthRange(0, 1); checkGL();

    {
        uint16_t halftonetex_pix[] = {
            0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,
            1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,
            0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,
            1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,
            0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,
            1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,
            0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,
            1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,
            0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,
            1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,
            0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,
            1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,
            0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,
            1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,
            0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,
            1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,
        };
        glGenTextures(1, &g_ghost_tex); checkGL();
        for (int i = 0; i != 256; ++i) halftonetex_pix[i] = halftonetex_pix[i] ? 0xffff : 0xfff0;
        glBindTexture(GL_TEXTURE_2D, g_ghost_tex); checkGL();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, halftonetex_pix); checkGL();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); checkGL();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); checkGL();
        glBindTexture(GL_TEXTURE_2D, 0); checkGL();
    }

    g_ghost_w = okno_rozmery[2];
    g_ghost_h = okno_rozmery[3];

    srand1(); // defaultní hodnota seed

    char gameDatPathCstr[1024] = {0};

    strncpy(gameDatPathCstr,
            "openmrac.dat"
            , 1023);

    // inicializace načítání z datového souboru
    if (!gbuff_in.init_dat(gameDatPathCstr)) { fprintf(stderr, "Error loading %s\n", gameDatPathCstr); return 1; }

    /*///////////////////////////////////////*/
    Gamemng gamemng;
    gamemng.p_settings = &settings;
    gamemng.init("maps.def", "objs.def", "cars.def", "skies.def");
    gamemng.set_global_volume(0);
    gamemng.p_bfps = settings.get("show_fps");
    gamemng.init_sound();

    g_hq_textures = settings.get("hq_textures");
    g_textureFiltering = settings.get("texture_filter");
    g_textureMipmaps = settings.get("mipmaps");

    MainMenu menu;
    menu.init(&gamemng, &settings);

    gamemng.p_gamemenu.p_settings = &settings;
    gamemng.p_gamemenu.p_menu = &menu;


#ifndef NO_CAMERA
    Cam kamera;
    float pos[3] = {0,0,0};
    kamera.init(pos, 0,0,50/**0.2*/,M_PI/**0.2*/);
#endif

    // program main loop
    bool done = false;
    //bool isactive = true;

    unsigned char player_bkeys[16] = {0};

#ifndef NO_CAMERA
    //SDLKey kamerakeys[8] = {SDLK_LEFT, SDLK_RIGHT, SDLK_DOWN, SDLK_UP, SDLK_w, SDLK_s, SDLK_a, SDLK_d};
    SDLKey kamerakeys[CAMERA_KEY_COUNT] = {SDLK_DELETE, SDLK_PAGEDOWN, SDLK_END, SDLK_HOME, SDLK_w, SDLK_s, SDLK_a, SDLK_d, SDLK_SPACE, SDLK_c};
    unsigned char kamera_bkeys[CAMERA_KEY_COUNT] = {0};
#endif

    bool b_test_map = false;
    if (b_test_map)
    {
        menu.p_players = 1;
        menu.p_cars_sel[0] = 0;
        menu.p_cars_tex_sel[0] = 0;
        menu.p_track_sel = 0;
        menu.p_sky_sel = 0;
        menu.p_direction_sel = 0;
        menu.p_view_dist = 10;
        menu.p_sound_vol = 0;
        menu.p_laps = 20;
        menu.game();
    }
#if TESTING_SLOWDOWN
    bool bslowdown = false;
#endif
    bool f12pressed = false;

#ifndef NO_CAMERA
    bool b_kamera_fast = true;
#endif

    while (!done)
    {
        MA_periodicStream();

        // message processing loop
        SDL_Event event;
        memset(&event, 0, sizeof(SDL_Event));
        while (SDL_PollEvent(&event))
        {
            // check for messages
            switch (event.type)
            {
//            case SDL_ACTIVEEVENT:
//              /* Something's happend with our focus
//               * If we lost focus or we are iconified, we
//               * shouldn't draw the screen
//               */
//              if (event.active.gain == 0)
//                  isactive = false;
//              else
//                  isactive = true;
//              break;

            /*case SDL_QUIT: // exit if the window is closed
                done = true;
                break;*/

            case SDL_KEYDOWN: // check for keypresses
                {
                    //#if defined(__WIN32__)
                    // pro fullscreen to nefunguje ani v linuxu samo
                    /*if (event.key.keysym.sym == SDLK_F4 && (event.key.keysym.mod & (KMOD_LALT | KMOD_RALT)))
                    {
                        done = true;
                        break;
                    }*/
                    //#endif

                    if (event.key.keysym.sym == SDLK_F12) {
                        f12pressed = true;
                    }

                    if (menu.p_bactive)
                    {
                        done = menu.keydown(event.key.keysym.sym);
                        break;
                    }

                    // exit if ESCAPE is pressed
                    if (gamemng.p_gamemenu.bmenu || event.key.keysym.sym == SDLK_ESCAPE /*|| (gamemng.p_state == 2 && event.key.keysym.sym == SDLK_RETURN)*/)
                    {
                        gamemng.p_gamemenu.keydown(event.key.keysym.sym);
                    }

                    #ifndef NO_CAMERA
                    if (event.key.keysym.sym == SDLK_TAB)
                    {
                        g_freecam = !g_freecam;
                    }
                    if (event.key.keysym.sym == SDLK_LSHIFT) {
                        b_kamera_fast = false;
                    }
                    #endif

                    #if TESTING_SLOWDOWN
                    if (event.key.keysym.sym == SDLK_RCTRL) {
                        bslowdown = !bslowdown;
                    }
                    #endif

                    for (int i = 0; i != 16; ++i)
                    {
                        if (controls[i].type == Control::E_KEYBOARD && (unsigned int)(event.key.keysym.sym) == controls[i].i)
                        {
                            player_bkeys[i] = 1;
                        }
                        /*if ((unsigned int)(event.key.keysym.sym) == player_keys[i])
                            player_bkeys[i] = 1;*/
                    }
#ifndef NO_CAMERA
                    // ovládání free kamery - keydown
                    if (g_freecam)
                        for (int i = 0; i != CAMERA_KEY_COUNT; ++i)
                            if (event.key.keysym.sym == kamerakeys[i])
                                kamera_bkeys[i] = 1;
#endif
                    ++n_klavesy;
                    break;
                }
            case SDL_KEYUP: // check for keypresses
                {
                    for (int i = 0; i != 16; ++i)
                    {
                        /*if ((unsigned int)(event.key.keysym.sym) == player_keys[i])
                            player_bkeys[i] = 0;*/
                        if (controls[i].type == Control::E_KEYBOARD && (unsigned int)(event.key.keysym.sym) == controls[i].i)
                        {
                            player_bkeys[i] = 0;
                        }
                    }
#ifndef NO_CAMERA
                    // ovládání free kamery - keyup
                    if (g_freecam)
                        for (int i = 0; i != CAMERA_KEY_COUNT; ++i)
                            if (event.key.keysym.sym == kamerakeys[i])
                                kamera_bkeys[i] = 0;
                    if (event.key.keysym.sym == SDLK_LSHIFT) {
                        b_kamera_fast = true;
                    }
#endif
                    --n_klavesy;
                    break;
                }

            /*case SDL_MOUSEBUTTONDOWN:
                {
                    for (int i = 0; i != 16; ++i)
                    {
                        if (controls[i].type == Control::E_MBUTTON && (unsigned int)(event.button.button) == controls[i].i)
                        {
                            player_bkeys[i] = 1;
                        }
                    }

                }
                break;
            case SDL_MOUSEBUTTONUP:
                {
                    for (int i = 0; i != 16; ++i)
                    {
                        if (controls[i].type == Control::E_MBUTTON && (unsigned int)(event.button.button) == controls[i].i)
                        {
                            player_bkeys[i] = 0;
                        }
                    }
                }
                break;
            case SDL_JOYAXISMOTION:
                {
                    for (int i = 0; i != 16; ++i)
                    {
                        if (controls[i].type == Control::E_JAXIS && event.jaxis.which == controls[i].joystickDeviceIndex && event.jaxis.axis == controls[i].i)
                        {
                            if (controls[i].joystickAxisValue == -2)
                            {
                                if (event.jaxis.value < -1)
                                {
                                    player_bkeys[i] = 1;
                                }
                                else if (event.jaxis.value > 1)
                                {
                                    player_bkeys[i] = 0;
                                }
                            }
                            else if (controls[i].joystickAxisValue == -1)
                            {
                                if (event.jaxis.value < -16384)
                                {
                                    player_bkeys[i] = 1;
                                }
                                else
                                {
                                    player_bkeys[i] = 0;
                                }
                            }
                            else if (controls[i].joystickAxisValue == 1)
                            {
                                if (event.jaxis.value > 16384)
                                {
                                    player_bkeys[i] = 1;
                                }
                                else
                                {
                                    player_bkeys[i] = 0;
                                }
                            }
                            else if (controls[i].joystickAxisValue == 2)
                            {
                                if (event.jaxis.value < -1)
                                {
                                    player_bkeys[i] = 0;
                                }
                                else if (event.jaxis.value > 1)
                                {
                                    player_bkeys[i] = 1;
                                }
                            }
                        }
                    }
                    break;
                }
            case SDL_JOYBUTTONDOWN:
                {
                    for (int i = 0; i != 16; ++i)
                    {
                        if (controls[i].type == Control::E_JBUTTON && event.jbutton.which == controls[i].joystickDeviceIndex && event.jbutton.button == controls[i].i)
                        {
                            player_bkeys[i] = 1;
                        }
                    }
                    break;
                }
            case SDL_JOYBUTTONUP:
                {
                    for (int i = 0; i != 16; ++i)
                    {
                        if (controls[i].type == Control::E_JBUTTON && event.jbutton.which == controls[i].joystickDeviceIndex && event.jbutton.button == controls[i].i)
                        {
                            player_bkeys[i] = 0;
                        }
                    }
                    break;
                }
            case SDL_JOYHATMOTION:
                {
                    for (int i = 0; i != 16; ++i)
                    {
                        if (controls[i].type == Control::E_JHAT && event.jhat.which == controls[i].joystickDeviceIndex && event.jhat.hat == controls[i].i)
                        {
                            if (event.jhat.value & controls[i].joystickHatValue)
                            {
                                player_bkeys[i] = 1;
                            }
                            else
                            {
                                player_bkeys[i] = 0;
                            }
                        }
                    }
                    break;
                }
                break;*/
            } // end switch
            if (menu.p_bactive)
            {
                menu.event(event);
            }
            else
            {
                /*if (event.type == SDL_JOYAXISMOTION)
                {
                    if (static_cast<unsigned>(event.jaxis.which) < joystickDevices.size())
                    {
                        JoystickDevice& joystickDevice = joystickDevices[static_cast<unsigned>(event.jaxis.which)];
                        if (static_cast<unsigned>(event.jaxis.axis) < joystickDevice.axesStates.size())
                        {
                            joystickDevice.axesStates[static_cast<unsigned>(event.jaxis.axis)].updateState(event.jaxis.value, false);
                        }
                    }
                }*/
            }
        } // end of message processing

        float deltaT = getdeltaT(); // čas mezi snímky v sekundách

#ifndef NO_CAMERA
        float cameraDeltaT = deltaT;
#endif

        #if TESTING_SLOWDOWN
            if (bslowdown)
                deltaT *= 0.01;
        #endif

        {
            static float fps_time = 0.f;
            static float fps_frames = 0.f;
            fps_time += deltaT;
            fps_frames += 1.f;
            if (fps_time >= 1.f)
            {
                static char fps_buff[256] = {0};
                float fps_fps = fps_frames/fps_time;
                snprintf(fps_buff, 255, "%.2f FPS", fps_fps);

                gamemng.p_gltext_fps.puts(0, fps_buff);

                fps_time = 0.f;
                fps_frames = 0.f;
            }
        }

        if (menu.p_bactive)
        {
            menu.render();
            DMesaSwapBuffers(db);
            if (f12pressed) {
                f12pressed = false;
                saveTgaScreenshot();
            }
            continue;
        }

        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); checkGL();

#ifndef NO_CAMERA
        float speed = b_kamera_fast ? 1.f : 0.1f;
        float rotSpeed = b_kamera_fast ? 1.f : 0.4f;
        if (kamera_bkeys[0]) kamera.turn_l(cameraDeltaT*rotSpeed);
        if (kamera_bkeys[1]) kamera.turn_r(cameraDeltaT*rotSpeed);
        if (kamera_bkeys[2]) kamera.turn_d(cameraDeltaT*rotSpeed);
        if (kamera_bkeys[3]) kamera.turn_u(cameraDeltaT*rotSpeed);

        if (kamera_bkeys[4]) kamera.move_f(cameraDeltaT*speed);
        if (kamera_bkeys[5]) kamera.move_b(cameraDeltaT*speed);
        if (kamera_bkeys[6]) kamera.move_l(cameraDeltaT*speed);
        if (kamera_bkeys[7]) kamera.move_r(cameraDeltaT*speed);

        if (kamera_bkeys[8]) kamera.move_u(cameraDeltaT*speed);
        if (kamera_bkeys[9]) kamera.move_d(cameraDeltaT*speed);
#endif

        glEnable(GL_DEPTH_TEST); checkGL();
        glDisable(GL_LIGHTING); checkGL();
        glDepthFunc(GL_LESS); checkGL();

        glColor4f(1.f, 1.f, 1.f, 1.f); checkGL();
        glEnable(GL_TEXTURE_2D); checkGL();
        gamemng.input(player_bkeys);
        float deltaTclamped = std::min(deltaT, 1.f);

        // transformace a vykreslení scény
        glPushMatrix(); checkGL();
#ifndef NO_CAMERA
            if (g_freecam) kamera.transform(); // transformace free kamery, pokud se používá, k jiným transformacím nedojde
#endif
            gamemng.frame(deltaTclamped); // ořezání času na maximálně 1 sekundu
        glPopMatrix(); checkGL();

        glEnable(GL_DEPTH_TEST); checkGL();

        //SDL_GL_SwapBuffers();
        DMesaSwapBuffers(db);

        if (f12pressed) {
            f12pressed = false;
            saveTgaScreenshot();
        }

    } // end main loop

    // Zapiseme nastaveni
    menu.save_settings();
    if (!menu.p_bactive)
    {
        settings.set("sound_volume", gamemng.get_global_volume());
        settings.set("view_distance", gamemng.get_far());
    }
    if (!b_test_map)
        settings.save();

    for (unsigned i = 0; i != global_al_sources.size(); ++i) {
        alDeleteSources(1, &(global_al_sources[i]));
    }
    for (unsigned i = 0; i != global_al_buffers.size(); ++i) {
        alDeleteBuffers(1, &(global_al_buffers[i]));
    }

    if (alcontext != NULL)
        alcDestroyContext(alcontext);
    if (aldevice != NULL)
        alcCloseDevice(aldevice);

    // all is well
    return 0;
}

void onExit(void)
{
    // clean up 3dfx
    gfx_dos_shutdown_impl();
    keyb_unhook_int();
    timer_quit();
}

int main (int argc, char** argv)
{
    atexit(onExit);
    int ret = my_main(argc, argv);
    my_exit(ret, false);
    return ret;
}
