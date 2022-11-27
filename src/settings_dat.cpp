#include "settings_dat.h"
#include <climits>
#include <cstring>
#include "cstring1.h"
#include <cstdio>
#include <algorithm>
#include "glext1.h"
#include "fopendir.h"
#include "appdefs.h"

    // name,     default, min, max, comment
const Sett_entry_base entry_base[] = {
    {"fullscreen", 0, 0, 1, "0 - windowed, 1 - fullscreen"},
    {"screen_x", 1280, 0, UINT_MAX, "screen resolution"},
    {"screen_y", 720, 0, UINT_MAX, ""},
    {"vsync", 1, 0, 1, "0 - vsync off, 1 - vsync on"},
    {"antialiasing", 0, 0, 2, "0 - off, 1 - 2x, 2 - 4x"},
    {"texture_filter", 1, 0,
#ifdef DISABLE_ANISOTROPY
    1
#else
ASSERT_ANISOTROPY
    2
#endif
    , "0 - bilinear, 1 - trilinear, 2 - anisotropic"},
    {"view_distance", 10, 0, 10, "10 - far (best), 0 - near"},
    {"show_fps", 0, 0, 1, "0 - fps counter off, 1 - fps counter on"},
    {"sound_volume", 100, 0, 100, "0 - 100"},
    {"last_laps", 3, 1, 50, "last session"},
    {"last_daytime", 0, 0, 1, "0 - day, 1 - evening"},
    {"last_track", 0, 0, UINT_MAX, ""},
    {"last_direction", 0, 0, 1, "0 - regular, 1 - reversed"},
    {"last_players", 2, 1, 4, ""},
    {"last_carsel1", 0, 0, UINT_MAX, ""},
    {"last_cartex1", 0, 0, UINT_MAX, ""},
    {"last_carsel2", 0, 0, UINT_MAX, ""},
    {"last_cartex2", 1, 0, UINT_MAX, ""},
    {"last_carsel3", 0, 0, UINT_MAX, ""},
    {"last_cartex3", 2, 0, UINT_MAX, ""},
    {"last_carsel4", 0, 0, UINT_MAX, ""},
    {"last_cartex4", 3, 0, UINT_MAX, ""},
    };

const char* Settings::controlNames[16] = {
    "player1_accel",
    "player1_brake",
    "player1_left ",
    "player1_right",

    "player2_accel",
    "player2_brake",
    "player2_left ",
    "player2_right",

    "player3_accel",
    "player3_brake",
    "player3_left ",
    "player3_right",

    "player4_accel",
    "player4_brake",
    "player4_left ",
    "player4_right",
    };

Settings::Settings(const char* filename, std::vector<JoystickDevice>* joystickDevices, std::vector<JoystickIdentifier>* joystickNotConnectedDevices, Control* controls)
{
    entry_size = sizeof(entry_base)/sizeof(*entry_base);
    entry = new Sett_entry[entry_size];
    for (unsigned int i = 0; i != entry_size; ++i)
    {
        entry[i].defaultVal = entry[i].val = entry_base[i].val;
    }
    this->joystickDevices = joystickDevices;
    this->joystickNotConnectedDevices = joystickNotConnectedDevices;
    this->controls = controls;
    this->filename = filename;
    this->openalDeviceDefault = true;
}

int Settings::load()
{
    // záznamy se předem nastaví na defaultní hodnoty
    //FILE* fin = fopen(filename, "r");
    FILE* fin = fopenDir(filename, "r", OPENMRAC_ORG, OPENMRAC_APP);
    //fprintf(stderr, "%s_%s\n", __PRETTY_FUNCTION__, filename);
    if (!fin) return 1;
    char buff[1024]; // buffer pro načtení řádků
    char controlTypeBuff[1024]; // buffer pro načtení typu control
    char format[1024] = { 0 };
    while (fgets(buff, 1024, fin))
    {
        uncomment(buff);
        bool found = false;
        for (unsigned int i = 0; i != entry_size; ++i)
        {
            snprintf(format, 1023, "%s %s", entry_base[i].key, "%u");
            unsigned int val;
            if (sscanf(buff, format, &val) == 1)
            {
                entry[i].val = std::max(std::min(val, entry_base[i].maxval), entry_base[i].minval);
                found = true;
                break;
            }
        }
        // tady zkusit najít a načíst openal device
        if (!found)
        {
            int readCharactersCount = 0;
            char space;
            if (sscanf(buff, "openal_device%c%n", &space, &readCharactersCount) == 1 && isSpace(space))
            {
                found = true;
                char* restOfBuff = buff + readCharactersCount;

                char* openalDeviceNameBuff = trim(restOfBuff);

                openalDeviceDefault = true;
                if (strcmp(openalDeviceNameBuff, "default") != 0 && strlen(openalDeviceNameBuff) > 2)
                {
                    int openalDeviceNameBuffLen = strlen(openalDeviceNameBuff);
                    if (openalDeviceNameBuff[0] == '"' && openalDeviceNameBuff[openalDeviceNameBuffLen - 1] == '"')
                    {
                        openalDeviceNameBuff[openalDeviceNameBuffLen - 1] = 0;
                        openalDeviceDefault = false;
                        openalDeviceName = openalDeviceNameBuff + 1;
                    }
                }
            }
        }
        if (!found)
        {
            for (unsigned i = 0; i != 16; ++i)
            {
                snprintf(format, 1023, "%s %s", controlNames[i], "%s%n");
                controlTypeBuff[0] = 0;
                int readCharactersCount = 0;
                if (sscanf(buff, format, controlTypeBuff, &readCharactersCount) == 1)
                {
                    char* restOfBuff = buff + readCharactersCount;

                    unsigned controlI = 0;

                    controls[i].type = Control::E_NONE;
                    if (strcmp(controlTypeBuff, "n") == 0)
                    {
                    }
                    else if (strcmp(controlTypeBuff, "k") == 0)
                    {
                        if (sscanf(restOfBuff, "%u", &controlI) == 1)
                        {
                            controls[i].type = Control::E_KEYBOARD;
                            controls[i].i = controlI;
                        }
                    }
                    else if (strcmp(controlTypeBuff, "m") == 0)
                    {
                        if (sscanf(restOfBuff, "%u", &controlI) == 1)
                        {
                            controls[i].type = Control::E_MBUTTON;
                            controls[i].i = controlI;
                        }
                    }
                    else if (strcmp(controlTypeBuff, "a") == 0 || strcmp(controlTypeBuff, "h") == 0 || strcmp(controlTypeBuff, "b") == 0)
                    {
                        unsigned j = 0;
                        for (j = 0; j != strlen(restOfBuff); ++j)
                        {
                            if (restOfBuff[j] == '"')
                            {
                                restOfBuff[j] = 0;
                                break;
                            }
                        }
                        ++j;
                        char* joystickSpecBuff = restOfBuff + j;
                        for (j = strlen(joystickSpecBuff); j != 0; --j)
                        {
                            if (joystickSpecBuff[j-1] == '"')
                            {
                                joystickSpecBuff[j-1] = 0;
                                break;
                            }
                        }
                        char* joystickSpecBuff2 = joystickSpecBuff + j;
                        JoystickIdentifier identifier;
                        identifier.parameters.name = joystickSpecBuff;
                        if (sscanf(joystickSpecBuff2, "%d %d %d %d %d", &(identifier.parameters.buttons), &(identifier.parameters.axes), &(identifier.parameters.hats), &(identifier.parameters.balls), &(identifier.parametersIndex)) == 5)
                        {
                            //
                            int joystickDeviceIndex = -1; // -1 = not connected
                            int joystickNotConnectedIndex = -1; // index do seznamu nepřipojených joysticků

                            for (int k = 0; k != static_cast<int>(joystickDevices->size()); ++k)
                            {
                                if ((*joystickDevices)[k].identifier == identifier)
                                {
                                    joystickDeviceIndex = k;
                                }
                            }
                            if (joystickDeviceIndex == -1)
                            {
                                for (int k = 0; k != static_cast<int>(joystickNotConnectedDevices->size()); ++k)
                                {
                                    if ((*joystickNotConnectedDevices)[k] == identifier)
                                    {
                                        joystickNotConnectedIndex = k;
                                    }
                                }
                                if (joystickNotConnectedIndex == -1)
                                {
                                    joystickNotConnectedIndex = joystickNotConnectedDevices->size();
                                    joystickNotConnectedDevices->push_back(identifier);
                                }
                            }


                            if (strcmp(controlTypeBuff, "a") == 0)
                            {
                                int axisValue = 0;
                                if (sscanf(restOfBuff, "%u %d", &controlI, &axisValue) == 2)
                                {
                                    controls[i].type = Control::E_JAXIS;
                                    controls[i].i = controlI;
                                    controls[i].joystickDeviceIndex = joystickDeviceIndex;
                                    controls[i].joystickNotConnectedIndex = joystickNotConnectedIndex;
                                    controls[i].joystickAxisValue = axisValue;
                                }
                            }
                            else if (strcmp(controlTypeBuff, "h") == 0)
                            {
                                unsigned hatValue = 0;
                                if (sscanf(restOfBuff, "%u %u", &controlI, &hatValue) == 2)
                                {
                                    controls[i].type = Control::E_JHAT;
                                    controls[i].i = controlI;
                                    controls[i].joystickDeviceIndex = joystickDeviceIndex;
                                    controls[i].joystickNotConnectedIndex = joystickNotConnectedIndex;
                                    controls[i].joystickHatValue = hatValue;
                                }
                            }
                            else if (strcmp(controlTypeBuff, "b") == 0)
                            {
                                if (sscanf(restOfBuff, "%u", &controlI) == 1)
                                {
                                    controls[i].type = Control::E_JBUTTON;
                                    controls[i].i = controlI;
                                    controls[i].joystickDeviceIndex = joystickDeviceIndex;
                                    controls[i].joystickNotConnectedIndex = joystickNotConnectedIndex;
                                }
                            }



                        }
                    }
                    break;
                }
            }
        }
    }
    fclose(fin);
    return 0;
}

/*
player1_accel k 273
player1_brake k 274
player1_left  k 276
player1_right k 275
player2_accel h 0 1 "Microsoft X-Box 360 pad" 11 6 1 0 0
player2_brake a 4 -1 "Microsoft X-Box 360 pad" 11 6 1 0 0
player2_left  b 0 "Microsoft X-Box 360 pad" 11 6 1 0 0
player2_right m 1
player3_accel b 2 "Generic   USB  Joystick  " 12 4 1 0 0
player3_brake n
player3_left  n
player3_right n
player4_accel n
player4_brake n
*/

int Settings::save()
{
    //FILE* fout = fopen(filename, "w");
    FILE* fout = fopenDir(filename, "w", OPENMRAC_ORG, OPENMRAC_APP);
    //fprintf(stderr, "%s_%s\n", __PRETTY_FUNCTION__, filename);
    if (!fout) return 1;
    for (unsigned int i = 0; i != entry_size; ++i)
    {
        fprintf(fout, "%s %d%s%s \n", entry_base[i].key,
            std::max(std::min(entry[i].val, entry_base[i].maxval), entry_base[i].minval),
            strlen(entry_base[i].comment) ? "        // " : "" , entry_base[i].comment);
    }

    if (openalDeviceDefault)
    {
        fprintf(fout, "openal_device default\n");
    }
    else
    {
        fprintf(fout, "openal_device \"%s\"\n", openalDeviceName.c_str());
    }

    for (unsigned i = 0; i != 16; ++i)
    {
        // {E_NONE, E_KEYBOARD, E_MBUTTON, E_JAXIS, E_JHAT, E_JBUTTON};
        //   n        k             m          a      h       b
        if (controls[i].type == Control::E_NONE)
        {
            fprintf(fout, "%s n\n", controlNames[i]);
        }
        else if (controls[i].type == Control::E_KEYBOARD)
        {
            fprintf(fout, "%s k %u\n", controlNames[i], controls[i].i);
        }
        else if (controls[i].type == Control::E_MBUTTON)
        {
            fprintf(fout, "%s m %u\n", controlNames[i], controls[i].i);
        }
        else if (controls[i].type == Control::E_JAXIS || controls[i].type == Control::E_JHAT || controls[i].type == Control::E_JBUTTON)
        {
            JoystickIdentifier* identifier = 0;
            if (controls[i].joystickDeviceIndex >= 0)
            {
                identifier = &((*joystickDevices)[controls[i].joystickDeviceIndex]).identifier;
            }
            else
            {
                identifier = &((*joystickNotConnectedDevices)[controls[i].joystickNotConnectedIndex]);
            }

            if (controls[i].type == Control::E_JAXIS)
            {
                fprintf(fout, "%s a %u %d", controlNames[i], controls[i].i, controls[i].joystickAxisValue);
            }
            else if (controls[i].type == Control::E_JHAT)
            {
                fprintf(fout, "%s h %u %u", controlNames[i], controls[i].i, controls[i].joystickHatValue);
            }
            else if (controls[i].type == Control::E_JBUTTON)
            {
                fprintf(fout, "%s b %u", controlNames[i], controls[i].i);
            }
            fprintf(fout, " \"%s\" %d %d %d %d %d\n", identifier->parameters.name.c_str(), identifier->parameters.buttons, identifier->parameters.axes, identifier->parameters.hats, identifier->parameters.balls, identifier->parametersIndex);
        }
    }
    fclose(fout);
    return 0;
}

int Settings::set(const char* key, unsigned int val) // 0 - OK, jinak chyba
{
    for (unsigned int i = 0; i != entry_size; ++i)
    {
        if (strcmp(entry_base[i].key, key) == 0)
        {
            entry[i].val = val;
            return 0;
        }
    }
    //assert(0);
    return 1;
}

const char* Settings::getOpenalDevice() const
{
    if (openalDeviceDefault)
    {
        return 0;
    }
    else
    {
        return openalDeviceName.c_str();
    }
}

unsigned int Settings::get(const char* key)
{
    for (unsigned int i = 0; i != entry_size; ++i)
    {
        if (strcmp(entry_base[i].key, key) == 0)
        {
            return entry[i].val;
        }
    }
    //assert(0);
    return 0;
}

void Settings::setOpenalDevice(const char* device)
{
    if (device)
    {
        openalDeviceDefault = false;
        openalDeviceName = device;
    }
    else
    {
        openalDeviceDefault = true;
    }
}

unsigned int Settings::getDefault(const char* key)
{
    for (unsigned int i = 0; i != entry_size; ++i)
    {
        if (strcmp(entry_base[i].key, key) == 0)
        {
            return entry[i].defaultVal;
        }
    }
    //assert(0);
    return 0;
}

void Settings::getJoystickName(char* buffer, int size, int i)
{
    JoystickIdentifier* identifier = 0;

    bool isConnected = false;

    if (controls[i].joystickDeviceIndex >= 0)
    {
        if (controls[i].joystickDeviceIndex < static_cast<int>(joystickDevices->size()))
        {
            identifier = &((*joystickDevices)[controls[i].joystickDeviceIndex].identifier);
            isConnected = true;
        }
    }
    else
    {
        if (controls[i].joystickNotConnectedIndex < static_cast<int>(joystickNotConnectedDevices->size()))
        {
            identifier = &((*joystickNotConnectedDevices)[controls[i].joystickNotConnectedIndex]);
        }
    }
    buffer[0] = 0;
    if (identifier)
    {
        int parametersIndex = identifier->parametersIndex;
        char nbuffer[256] = {0};
        if (parametersIndex != 0)
        {
            snprintf(nbuffer, 255, " %d", parametersIndex);
        }
        snprintf(buffer, size, "%s%s%s", (isConnected ? "" : "N/C "), identifier->parameters.name.c_str(), nbuffer);
    }
}

void Settings::getControlName(char* buff, unsigned n, unsigned i, bool joystickName)
{
    if (controls[i].type == Control::E_KEYBOARD)
    {
        snprintf(buff, n, "%s", SDL_GetKeyName(static_cast<SDL_Keycode>(controls[i].i)));
    }
    else if (controls[i].type == Control::E_MBUTTON)
    {
        if (controls[i].i == SDL_BUTTON_LEFT)
        {
            snprintf(buff, n, "%s", "mouse lbtn");
        }
        else if (controls[i].i == SDL_BUTTON_MIDDLE)
        {
            snprintf(buff, n, "%s", "mouse mbtn");
        }
        else if (controls[i].i == SDL_BUTTON_RIGHT)
        {
            snprintf(buff, n, "%s", "mouse rbtn");
        }
        else
        {
            snprintf(buff, n, "mouse btn%d", controls[i].i);
        }
    }
    else if (controls[i].type == Control::E_JBUTTON)
    {
        if (joystickName)
        {
            char deviceNameBuffer[512] = {0};
            getJoystickName(deviceNameBuffer, 511, i);
            snprintf(buff, n, "button %d (%s)", controls[i].i + 1, deviceNameBuffer);
        }
        else
        {
            snprintf(buff, n, "button %d", controls[i].i + 1);
        }
    }
    else if (controls[i].type == Control::E_JHAT)
    {
        const char* hatDirectionNames[4] = {"left", "right", "up", "down"};

        int hatDirectionIndex = 0;

        if (controls[i].joystickHatValue == SDL_HAT_LEFT)
        {
            hatDirectionIndex = 0;
        }
        else if (controls[i].joystickHatValue == SDL_HAT_RIGHT)
        {
            hatDirectionIndex = 1;
        }
        else if (controls[i].joystickHatValue == SDL_HAT_UP)
        {
            hatDirectionIndex = 2;
        }
        else if (controls[i].joystickHatValue == SDL_HAT_DOWN)
        {
            hatDirectionIndex = 3;
        }
        if (joystickName)
        {
            char deviceNameBuffer[512] = {0};
            getJoystickName(deviceNameBuffer, 511, i);
            snprintf(buff, n, "hat %d %s (%s)", controls[i].i + 1, hatDirectionNames[hatDirectionIndex], deviceNameBuffer);
        }
        else
        {
            snprintf(buff, n, "hat %d %s", controls[i].i + 1, hatDirectionNames[hatDirectionIndex]);
        }
    }
    else if (controls[i].type == Control::E_JAXIS)
    {
        const char* axisValueNames[5] = {"--", "-", "...", "+", "++"};

        int axisValueIndex = controls[i].joystickAxisValue + 2;
        if (axisValueIndex < 0 || axisValueIndex > 4)
        {
            axisValueIndex = 2;
        }
        if (joystickName)
        {
            char deviceNameBuffer[512] = {0};
            getJoystickName(deviceNameBuffer, 511, i);
            snprintf(buff, n, "axis %d %s (%s)", controls[i].i + 1, axisValueNames[axisValueIndex], deviceNameBuffer);
        }
        else
        {
            snprintf(buff, n, "axis %d %s", controls[i].i + 1, axisValueNames[axisValueIndex]);
        }
    }
    else
    {
        buff[0] = 0;
    }
}
