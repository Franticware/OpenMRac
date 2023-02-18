#ifndef CONTROLS_H
#define CONTROLS_H

#include <string>
#include <vector>

#include <SDL2/SDL.h>

struct Control
{
    enum {E_NONE, E_KEYBOARD, E_MBUTTON, E_JAXIS, E_JHAT, E_JBUTTON};
    enum {JAXIS_NEGATIVE = -2, JAXIS_NEGATIVE_HALF = -1, JAXIS_POSITIVE_HALF = 1, JAXIS_POSITIVE = 2};
    Control() : type(E_NONE) { }
    Control(unsigned sym) : type(E_KEYBOARD), i(sym) { }
    int type; // <- enum
    unsigned i; // key, mouse button, joystick axis, joystick hat, or joystick button
    int joystickDeviceIndex; // -1 = not connected
    int joystickNotConnectedIndex; // index do seznamu nepřipojených joysticků
    int joystickAxisValue; // -2, -1, 1, 2
    unsigned joystickHatValue;
};

struct JoystickParameters
{
    int axes, balls, hats, buttons;
    std::string name;

    bool operator==(const JoystickParameters& other) const
    {
        return axes == other.axes && balls == other.balls && hats == other.hats && buttons == other.buttons && name == other.name;
    }
};

struct AxisState // for axis range detection
{
    enum {E_INITIAL_NONE, E_INITIAL_NEGATIVE, E_INITIAL_ZERO, E_INITIAL_POSITIVE, E_ENTER_POSITIVE, E_ENTER_NEGATIVE};
    /* return value:
     * -2  +-
     * -1  0-
     *  0  none
     *  1  0+
     *  2  -+
     */
    int updateState(Sint16 value, bool enterMode);

    void resetEnterMode();

    static int convertValue(Sint16 value); // -1, 0, 1 ... -, center, +; -2 ... undefined

    AxisState() : lastPosition(-2), state(E_INITIAL_NONE) { }
    int lastPosition;
    int state;
};

struct JoystickIdentifier
{
    JoystickParameters parameters;
    int parametersIndex; // identical devices indexing

    bool operator==(const JoystickIdentifier& other) const
    {
        return parameters == other.parameters && parametersIndex == other.parametersIndex;
    }
};

struct JoystickDevice
{
    JoystickIdentifier identifier;
    std::vector<AxisState> axesStates;
    SDL_Joystick* device;

    void open(int index);
    void close();
};

void initializeParametersIndices(std::vector<JoystickDevice>& devices);

#endif // CONTROLS_H
