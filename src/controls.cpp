#include "controls.h"

    /* return value:
     * -2  +-
     * -1  0-
     *  0  none
     *  1  0+
     *  2  -+
     */
int AxisState::updateState(Sint16 value, bool enterMode)
{
    int position = convertValue(value);
    if (position >= -1 && position <= 1)
    {
        lastPosition = position;
    }
    if (enterMode)
    {
        if (state == E_INITIAL_NONE)
        {
            if (position == -1)
            {
                state = E_INITIAL_NEGATIVE;
            }
            else if (position == 0)
            {
                state = E_INITIAL_ZERO;
            }
            else if (position == 1)
            {
                state = E_INITIAL_POSITIVE;
            }
        }
        else if (state == E_INITIAL_ZERO)
        {
            if (position == 1)
            {
                return 1;
            }
            else if (position == -1)
            {
                return -1;
            }
        }
        else if (state == E_INITIAL_POSITIVE)
        {
            if (position == 0)
            {
                state = E_ENTER_NEGATIVE;
            }
        }
        else if (state == E_INITIAL_NEGATIVE)
        {
            if (position == 0)
            {
                state = E_ENTER_POSITIVE;
            }
        }
        else if (state == E_ENTER_POSITIVE)
        {
            if (position == 1)
            {
                return 2;
            }
        }
        else if (state == E_ENTER_NEGATIVE)
        {
            if (position == -1)
            {
                return -2;
            }
        }
    }
    else
    {
        if (position == -1)
        {
            state = E_INITIAL_NEGATIVE;
        }
        else if (position == 0)
        {
            state = E_INITIAL_ZERO;
        }
        else if (position == 1)
        {
            state = E_INITIAL_POSITIVE;
        }
    }
    return 0;
}

void AxisState::resetEnterMode()
{
    if (lastPosition == -1)
    {
        state = E_INITIAL_NEGATIVE;
    }
    else if (lastPosition == 0)
    {
        state = E_INITIAL_ZERO;
    }
    else if (lastPosition == 1)
    {
        state = E_INITIAL_POSITIVE;
    }
}

int AxisState::convertValue(Sint16 value) // -1, 0, 1 ... -, center, +; -2 ... undefined
{
    if (value < -24576)
    {
        return -1;
    }
    else if (value > -8192 && value < 8192)
    {
        return 0;
    }
    else if (value > 24576)
    {
        return 1;
    }
    else
    {
        return -2;
    }
}

void JoystickDevice::open(int index)
{
    device = SDL_JoystickOpen(index);
    identifier.parameters.name = SDL_JoystickName(index);
    identifier.parameters.axes = SDL_JoystickNumAxes(device);
    identifier.parameters.balls = SDL_JoystickNumBalls(device);
    identifier.parameters.hats = SDL_JoystickNumHats(device);
    identifier.parameters.buttons = SDL_JoystickNumButtons(device);
    identifier.parametersIndex = -1;
    axesStates.resize(identifier.parameters.axes);
}

void JoystickDevice::close()
{
    if (device)
    {
        SDL_JoystickClose(device);
    }
}

void initializeParametersIndices(std::vector<JoystickDevice>& devices)
{
    for (unsigned i = 0; i != devices.size(); ++i)
    {
        int index = 0;
        if (devices[i].identifier.parametersIndex == -1)
        {
            for (unsigned j = i; j != devices.size(); ++j)
            {
                if (devices[i].identifier.parameters == devices[j].identifier.parameters)
                {
                    devices[j].identifier.parametersIndex = index;
                }
            }
        }
    }
}
