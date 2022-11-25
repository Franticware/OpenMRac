#include "platform.h"
#include "appdefs.h"

#include <cstdio> // ve windows výstup na konzoli funguje až po zavolání ActivateConsole
#include <algorithm>
#include <vector>
#include <set>

#include <SDL2/SDL.h>
#include <SDL2/SDL_endian.h>
#ifndef __MACOSX__
#include <GL/gl.h>
#else
#include <OpenGL/gl.h>
#endif

#ifdef USE_MINIAL
#include "minial.h"
#else
#ifndef __MACOSX__
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#else
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#endif
#endif

#include "gltext.h"

#include "glext1.h"

#include "octopus.h"

#include "skysph.h"

#define NO_CAMERA
//#undef NO_CAMERA

//#define TESTING_SLOWDOWN 1

#ifndef NO_CAMERA
#include "cam.h"
#endif

#include "gameaux.h"
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

#include "settingsdialog.h"

#include "fopendir.h"

#ifdef __MACOSX__
#include <CoreFoundation/CFBundle.h>
#endif

std::vector<ALuint> global_al_sources;
std::vector<ALuint> global_al_buffers;

// odkazy na různá místa projektu:
// tření o zeď
// odrazivost při kolizích
// magický vzorec pro výpočet útlumu při nárazu
// další věci k časům na kolo

int g_mipmaps_available = 0;

int EnableOpenGL(bool fullscreen, bool vsync, unsigned int width, unsigned int height/*, unsigned int depth = 0*/);

std::vector<JoystickDevice>* g_joystickDevices = 0;

SDL_GLContext maincontext; // Our opengl context handle

void my_exit(int ret, bool callExit)
{
    for (unsigned int i = 0; i < g_joystickDevices->size(); ++i)
    {
        /*if (SDL_JoystickOpened(i))
            (*g_joystickDevices)[i].close();*/
    }
    SDL_GL_DeleteContext(maincontext);
    SDL_Quit();
    
#ifndef __MACOSX__
    if (callExit)
    {
        exit(ret); // exit everywhere except mac
    }
#else
    _exit(ret); // kill on mac
#endif
}

int n_klavesy = 0; // počet stisknutých kláves

extern int ge_bpass1; // globální vypnutí aktualizace pohledu

extern int g_freecam;

extern int g_sound_on;

GLuint g_ghost_tex = 0;
int g_ghost_w = 0;
int g_ghost_h = 0;

int g_multisampleMode = 2; // 0 - 0ff, 1 - 2x, 2 - 4x
int g_textureFiltering = 2; // 0 - bilinear, 1 - trilinear, 2 - aniso

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
    snprintf(filename, 255, "openmrac-scr%.3d.tga", screenshotNumber);
    //FILE* fout = fopen(filename, "wb");
    FILE* fout = fopenDir(filename, "wb", OPENMRAC_ORG, OPENMRAC_APP);
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
            SDL_SwapLE16(static_cast<unsigned short>(okno_rozmery[2])),//12     2 byte  Width   šířka obrázku uvedená v pixelech
            SDL_SwapLE16(static_cast<unsigned short>(okno_rozmery[3]))//14  2 byte  Height  výška obrázku uvedená v pixelech
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

void initScreenModesVector(std::vector<ScreenMode>& screenModesVector, ScreenMode& currentScreenMode, ScreenMode defaultScreenMode)
{
    std::set<ScreenMode> screenModesSet;

    screenModesSet.insert(currentScreenMode);
    screenModesSet.insert(defaultScreenMode);

    screenModesSet.insert(ScreenMode(320, 200, 0));
    screenModesSet.insert(ScreenMode(320, 240, 0));
    screenModesSet.insert(ScreenMode(400, 300, 0));
    screenModesSet.insert(ScreenMode(640, 400, 0));
    screenModesSet.insert(ScreenMode(640, 480, 0));
    screenModesSet.insert(ScreenMode(800, 600, 0));
    screenModesSet.insert(ScreenMode(1024, 768, 0));
    screenModesSet.insert(ScreenMode(1152, 720, 0));
    screenModesSet.insert(ScreenMode(1152, 864, 0));
    screenModesSet.insert(ScreenMode(1280, 720, 0));
    screenModesSet.insert(ScreenMode(1280, 800, 0));
    screenModesSet.insert(ScreenMode(1280, 960, 0));
    screenModesSet.insert(ScreenMode(1280, 1024, 0));
    screenModesSet.insert(ScreenMode(1400, 1050, 0));
    screenModesSet.insert(ScreenMode(1440, 810, 0));
    screenModesSet.insert(ScreenMode(1440, 900, 0));
    screenModesSet.insert(ScreenMode(1600, 900, 0));
    screenModesSet.insert(ScreenMode(1600, 1200, 0));
    screenModesSet.insert(ScreenMode(1920, 1080, 0));
    screenModesSet.insert(ScreenMode(1920, 1200, 0));
    screenModesSet.insert(ScreenMode(2560, 1440, 0));
    screenModesSet.insert(ScreenMode(0, 0, 1));

    std::copy(screenModesSet.begin(), screenModesSet.end(), std::back_inserter(screenModesVector));
}

#define CAMERA_KEY_COUNT 10

int my_main (int argc, char** argv)
{   
    bool skipSettings = false;
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
    }

    static std::vector<JoystickDevice> joystickDevices;
    g_joystickDevices = &joystickDevices;
    std::vector<JoystickIdentifier> joystickNotConnectedDevices;

    // initialize SDL video
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0)
    {
        fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
        return 1;
    }

    {
        int numJoysticks = SDL_NumJoysticks();
        if (numJoysticks)
        {
            joystickDevices.resize(numJoysticks);
            for (int i = 0; i < numJoysticks; ++i)
                joystickDevices[i].open(i);
            SDL_JoystickEventState(SDL_ENABLE);
        }
    }

    initializeParametersIndices(joystickDevices);

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

    // SDL2 key codes differ from SDL1.2 ones, so the settings file cannnot be shared
    Settings settings("settings-sdl2.dat", &joystickDevices, &joystickNotConnectedDevices, controls);
    settings.load();

    if (!skipSettings)
    {
        ScreenMode currentScreenMode(settings.get("screen_x"), settings.get("screen_y"), settings.get("fullscreen"));
        ScreenMode defaultScreenMode(settings.getDefault("screen_x"), settings.getDefault("screen_y"), settings.getDefault("fullscreen"));

        std::vector<ScreenMode> screenModesVector;
        initScreenModesVector(screenModesVector, currentScreenMode, defaultScreenMode);

        std::vector<std::string> alDevicesVector;
        const char* alDevices = NULL;
#ifndef __MACOSX__
        if (alcIsExtensionPresent(NULL, "ALC_ENUMERATE_ALL_EXT")) {
            alDevices = alcGetString(NULL, ALC_ALL_DEVICES_SPECIFIER);
        }
        else
#endif
        if (alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT")) {
            alDevices = alcGetString(NULL, ALC_DEVICE_SPECIFIER);
        }
        while (*alDevices)
        {
            alDevicesVector.push_back(alDevices);
            alDevices += strlen(alDevices)+1;
        }

        SettingsDialog settingsDialog(screenModesVector, currentScreenMode, defaultScreenMode,
            settings.get("vsync"),
            settings.get("antialiasing"),
            settings.get("texture_filter"),
            settings.get("show_fps"),
            alDevicesVector, settings.getOpenalDevice());

        settingsDialog.execute();

        if (!settingsDialog.m_continue)
        {
            return 0;
        }

        ScreenMode selectedScreenMode = screenModesVector[settingsDialog.getSelectedScreenMode()];

        settings.set("screen_x", selectedScreenMode.width);
        settings.set("screen_y", selectedScreenMode.height);
        settings.set("fullscreen", selectedScreenMode.fullscreen);
        settings.set("show_fps", settingsDialog.getShowFpsChecked());
        settings.set("vsync", settingsDialog.getVsyncChecked());
        settings.set("texture_filter", settingsDialog.getTextureFilter());
        settings.set("antialiasing", settingsDialog.getAntialiasingMode());
        settings.setOpenalDevice(settingsDialog.getOpenalDevice());

        settings.save();
    }

    if (gameWindow)
    {
        SDL_SetWindowTitle(gameWindow, "");
    }

    getdeltaT_init();

    unsigned int isfullscreen = settings.get("fullscreen");

    g_multisampleMode = settings.get("antialiasing");
    g_textureFiltering = settings.get("texture_filter");

    if (isfullscreen)
        SDL_ShowCursor(SDL_DISABLE);

    int enableOpenGLResult = EnableOpenGL(isfullscreen, settings.get("vsync"),
        settings.get("screen_x"), settings.get("screen_y"));
    if (enableOpenGLResult != 0)
    {
        return enableOpenGLResult; // error
    }
    
    //GLfloat textureSize;
    //glGetFloatv(GL_MAX_TEXTURE_SIZE, &textureSize); checkGL();
    //fprintf(stderr, "max texture size %f\n", textureSize);
    //fflush(stderr);

#if !USE_VSYNC
    bool limitFramerate = settings.get("vsync");
#endif

    if (gameWindow)
    {
        SDL_SetWindowTitle(gameWindow, "OpenMRac " OPENMRAC_VERSION);
    }
        
    if (strcmp((const char*)glGetString(GL_VERSION), "1.4") >= 0) {
        g_mipmaps_available = 1;
    }   
#ifdef __MACOSX__
    g_mipmaps_available = 1;
#endif
    
    // Initialize Open AL

    const char* aldevicestr = settings.getOpenalDevice();

    ALCdevice* aldevice = alcOpenDevice(aldevicestr); // NULL parameter = open default device
    ALCcontext* alcontext = NULL;
    if (aldevice != NULL) {
        alcontext = alcCreateContext(aldevice,NULL); // create context
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

#if !defined(__MORPHOS__) && !defined(__amigaos4__)
    Glext glext;
    if (!glext.init_ARB_texture_cube_map())
    {
        fprintf(stderr, "Error: Unsupported OpenGL extension GL_ARB_texture_cube_map.\n");
        return 5;
    }

    if (g_multisampleMode)
    {
        if (!glext.init_ARB_multisample())
        {
            fprintf(stderr, "Warning: Unsupported OpenGL extension GL_ARB_multisample.\n");
            g_multisampleMode = 0;
        }
    }
#endif

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

#ifndef __MORPHOS__
    if (g_multisampleMode)
    {
        glEnable(GL_MULTISAMPLE_ARB); checkGL();
    }
#endif

    glEnable(GL_CULL_FACE); checkGL();
    glEnable(GL_DEPTH_TEST); checkGL();
    glDepthFunc(GL_LESS); checkGL();
    glAlphaFunc(GL_GREATER, 0.5); checkGL();
    glEnable(GL_LIGHT0); checkGL();
    glEnable(GL_LIGHTING); checkGL();
    
    glDepthRange(0, 1); checkGL();
    
    {
        char halftonetex_pix[] = {
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
        for (int i = 0; i != 256; ++i) halftonetex_pix[i] *= 255;
        glBindTexture(GL_TEXTURE_2D, g_ghost_tex); checkGL();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 16, 16, 0, GL_ALPHA, GL_UNSIGNED_BYTE, halftonetex_pix); checkGL();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); checkGL();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); checkGL();
        glBindTexture(GL_TEXTURE_2D, 0); checkGL();
    }
    
    g_ghost_w = okno_rozmery[2];
    g_ghost_h = okno_rozmery[3];

    srand1(); // defaultní hodnota seed
    
    char gameDatPathCstr[1024] = {0};
#ifdef __MACOSX__
    {
        // http://stackoverflow.com/questions/8768217/how-can-i-find-the-path-to-a-file-in-an-application-bundle-nsbundle-using-c
        // Get a reference to the main bundle
        CFBundleRef mainBundle = CFBundleGetMainBundle();

        // Get a reference to the file's URL
        CFURLRef gameDatURL = CFBundleCopyResourceURL(mainBundle, CFSTR("openmrac"), CFSTR("dat"), NULL);

        // Convert the URL reference into a string reference
        CFStringRef gameDatPath = CFURLCopyFileSystemPath(gameDatURL, kCFURLPOSIXPathStyle);

        // Get the system encoding method
        CFStringEncoding encodingMethod = CFStringGetSystemEncoding();

        // Convert into a C string
        CFStringGetCString(gameDatPath, gameDatPathCstr, 1023, encodingMethod);
    }
#else

//#define DIR_OPENMRAC_DAT "/home/vojta/"
    strncpy(gameDatPathCstr,
    #ifdef DIR_OPENMRAC_DAT

        #define m2s_(a) m2s2_(a)
        #define m2s2_(a) #a

        m2s_(DIR_OPENMRAC_DAT)
    #endif
            "openmrac.dat"
            , 1023);
#endif

    // inicializace načítání z datového souboru
    if (!gbuff_in.init_dat(gameDatPathCstr)) { fprintf(stderr, "Error loading %s\n", gameDatPathCstr); return 1; }

    /*///////////////////////////////////////*/
    Gamemng gamemng;
    gamemng.p_settings = &settings;
    gamemng.init("maps.def", "objs.def", "cars.def", "skies.def");
    gamemng.set_global_volume(0);
    gamemng.p_bfps = settings.get("show_fps");
    gamemng.init_sound();

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

#if !USE_VSYNC
        Uint32 startFrameTime = 0;
        if (limitFramerate)
        {
            startFrameTime = SDL_GetTicks();
        }
#endif
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

            case SDL_QUIT: // exit if the window is closed
                done = true;
                break;

            case SDL_KEYDOWN: // check for keypresses
                {
                    //#if defined(__WIN32__)
                    // pro fullscreen to nefunguje ani v linuxu samo
                    if (event.key.keysym.sym == SDLK_F4 && (event.key.keysym.mod & (KMOD_LALT | KMOD_RALT)))
                    {
                        done = true;
                        break;
                    }
                    //#endif

                    if (event.key.keysym.sym == 
#ifndef __MACOSX__
                        SDLK_F12
#else
                        SDLK_F12
#endif
                        ) {
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

            case SDL_MOUSEBUTTONDOWN:
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
                break;
            } // end switch
            if (menu.p_bactive)
            {
                menu.event(event);
            }
            else
            {
                if (event.type == SDL_JOYAXISMOTION)
                {
                    if (static_cast<unsigned>(event.jaxis.which) < joystickDevices.size())
                    {
                        JoystickDevice& joystickDevice = joystickDevices[static_cast<unsigned>(event.jaxis.which)];
                        if (static_cast<unsigned>(event.jaxis.axis) < joystickDevice.axesStates.size())
                        {
                            joystickDevice.axesStates[static_cast<unsigned>(event.jaxis.axis)].updateState(event.jaxis.value, false);
                        }
                    }
                }
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
            SDL_GL_SwapWindow(gameWindow);
            if (f12pressed) {
                f12pressed = false;
                saveTgaScreenshot();
            }
            SDL_Delay(10);
            continue;
        }

        // smazání depth bufferu každý snímek a color bufferu každý 100. snímek
        static unsigned int frame_clr_cnt = 0;
        if (frame_clr_cnt >= 100 || g_freecam)
        {
            glClear (GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); checkGL();
            frame_clr_cnt = 0;
        } else {
            glClear (GL_DEPTH_BUFFER_BIT); checkGL();
            ++frame_clr_cnt;
        }

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

        SDL_GL_SwapWindow(gameWindow);
        
        if (f12pressed) {
            f12pressed = false;
            saveTgaScreenshot();
        }


#if !USE_VSYNC
        if (limitFramerate)
        {
            const Uint32 minFrameTime = 8;
            Uint32 endFrameTime = SDL_GetTicks();
            Uint32 frameTime = endFrameTime - startFrameTime; // when the timer overflows, frameTime gets very big, then it is not less then minimum frame time and framerate limiting is skipped; fortunately this happens once in ~49 days ;-)
            if (frameTime < minFrameTime)
            {
                Uint32 frameDelay = minFrameTime - frameTime;
                SDL_Delay(frameDelay);
            }
        }
#endif
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

#if defined(__WIN32__)

#ifndef DPI_ENUMS_DECLARED
typedef enum PROCESS_DPI_AWARENESS
{
    PROCESS_DPI_UNAWARE = 0,
    PROCESS_SYSTEM_DPI_AWARE = 1,
    PROCESS_PER_MONITOR_DPI_AWARE = 2
} PROCESS_DPI_AWARENESS;
#endif

typedef BOOL (WINAPI * SETPROCESSDPIAWARE_T)(void);
typedef HRESULT (WINAPI * SETPROCESSDPIAWARENESS_T)(PROCESS_DPI_AWARENESS);

bool win32_SetProcessDpiAware(void) {
    HMODULE shcore = LoadLibraryA("Shcore.dll");
    SETPROCESSDPIAWARENESS_T SetProcessDpiAwareness = NULL;
    if (shcore) {
        SetProcessDpiAwareness = (SETPROCESSDPIAWARENESS_T) GetProcAddress(shcore, "SetProcessDpiAwareness");
    }
    HMODULE user32 = LoadLibraryA("User32.dll");
    SETPROCESSDPIAWARE_T SetProcessDPIAware = NULL;
    if (user32) {
        SetProcessDPIAware = (SETPROCESSDPIAWARE_T) GetProcAddress(user32, "SetProcessDPIAware");
    }

    bool ret = false;
    if (SetProcessDpiAwareness) {
        ret = SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE) == S_OK;
    } else if (SetProcessDPIAware) {
        ret = SetProcessDPIAware() != 0;
    }

    if (user32) {
        FreeLibrary(user32);
    }
    if (shcore) {
        FreeLibrary(shcore);
    }
    return ret;
}

volatile bool ha = win32_SetProcessDpiAware();

#endif

int main (int argc, char** argv)
{
    int ret = my_main(argc, argv);
    my_exit(ret, false);
    return ret;
}

// 0 - success, 1 - error
int EnableOpenGL(bool fullscreen, bool vsync, unsigned int width, unsigned int height/*, unsigned int depth*/)
{
    // Request OpenGL context
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    if (g_multisampleMode)
    {
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        int samples = 1 << g_multisampleMode;
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, samples);
    }

    Uint32 flags = 0;

    if (fullscreen)
    {
        flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    }

    gameWindow = SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            fullscreen ? 0 : width, fullscreen ? 0 : height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | flags);

    if (!gameWindow)
    {
        SDL_Quit();
        return 1;
    }

    // Create our opengl context and attach it to our window
    maincontext = SDL_GL_CreateContext(gameWindow);
    // This makes our buffer swap syncronized with the monitor's vertical refresh
    SDL_GL_SetSwapInterval(vsync ? 1 : 0);

    int actualWidth, actualHeight;
    SDL_GetWindowSize(gameWindow, &actualWidth, &actualHeight);

    glViewport(0, 0, actualWidth, actualHeight); checkGL();
    return 0;
}

