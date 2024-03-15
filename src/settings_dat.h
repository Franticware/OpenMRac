#ifndef SETTINGS_DAT_H
#define SETTINGS_DAT_H

#include "controls.h"

struct Sett_entry_base {
    char key[256];
    unsigned int val, minval, maxval;
    char comment[256];
};

struct Sett_entry {
    unsigned int val, defaultVal;
};

class Settings {
public:
    Settings(const char* filename, /*std::vector<JoystickDevice>* joystickDevices, std::vector<JoystickIdentifier>* joystickNotConnectedDevices,*/ Control* controls);
    ~Settings() { delete[] entry; }
    int load(); // 0 - ok, jinak error
    int save();
    int set(const char* key, unsigned int val);
    unsigned int get(const char* key);
    unsigned int getDefault(const char* key);
    void getJoystickName(char* buffer, int size, int i);

    void getControlName(char* buff, unsigned n, unsigned i, bool joystickName);

    Sett_entry* entry;
    unsigned int entry_size;
    /*std::vector<JoystickDevice>* joystickDevices;
    std::vector<JoystickIdentifier>* joystickNotConnectedDevices;*/
    Control* controls;
    const char* filename;

    static const char* controlNames[16];
};

//bool read_snddev_dat(char* buff);

#endif
