#include "minisdl.h"
#include "doskeyb.h"

#include <cstdio>
#include <cstring>

const char* SDL_GetKeyName(int key)
{
    switch (key)
    {
    case K_ESC:
        return "esc";
    case K_F1:
        return "f1";
    case K_F2:
        return "f2";
    case K_F3:
        return "f3";
    case K_F4:
        return "f4";
    case K_F5:
        return "f5";
    case K_F6:
        return "f6";
    case K_F7:
        return "f7";
    case K_F8:
        return "f8";
    case K_F9:
        return "f9";
    case K_F10:
        return "f10";
    case K_F11:
        return "f11";
    case K_F12:
        return "f12";
    case K_SCROLL_LOCK:
        return "scroll lock";
    case K_ACUTE:
        return "`";
    case K_1:
        return "1";
    case K_2:
        return "2";
    case K_3:
        return "3";
    case K_4:
        return "4";
    case K_5:
        return "5";
    case K_6:
        return "6";
    case K_7:
        return "7";
    case K_8:
        return "8";
    case K_9:
        return "9";
    case K_0:
        return "0";
    case K_MINUS:
        return "-";
    case K_EQUALS:
        return "=";
    case K_BACKSPACE:
        return "backspace";
    case K_TAB:
        return "tab";
    case K_Q:
        return "q";
    case K_W:
        return "w";
    case K_E:
        return "e";
    case K_R:
        return "r";
    case K_T:
        return "t";
    case K_Y:
        return "y";
    case K_U:
        return "u";
    case K_I:
        return "i";
    case K_O:
        return "o";
    case K_P:
        return "p";
    case K_LSBR:
        return "[";
    case K_RSBR:
        return "]";
    case K_ENTER:
        return "enter";
    case K_CAPS:
        return "caps lock";
    case K_A:
        return "a";
    case K_S:
        return "s";
    case K_D:
        return "d";
    case K_F:
        return "f";
    case K_G:
        return "g";
    case K_H:
        return "h";
    case K_J:
        return "j";
    case K_K:
        return "k";
    case K_L:
        return "l";
    case K_SEMICOLON:
        return ";";
    case K_SQUOTE:
        return "'";
    case K_BACKSLASH:
        return "\\";
    case K_LSHIFT:
        return "lshift";
    case K_AUX0:
        return "|";
    case K_Z:
        return "z";
    case K_X:
        return "x";
    case K_C:
        return "c";
    case K_V:
        return "v";
    case K_B:
        return "b";
    case K_N:
        return "n";
    case K_M:
        return "m";
    case K_COMMA:
        return ",";
    case K_DOT:
        return ".";
    case K_SLASH:
        return "/";
    case K_RSHIFT:
        return "rshift";
    case K_LCTRL:
        return "lctrl";
    case K_LALT:
        return "lalt";
    case K_SPACE:
        return "space";
    case K_RALT:
        return "ralt";
    case K_RCTRL:
        return "rctrl";
    case K_INS:
        return "ins";
    case K_DEL:
        return "del";
    case K_HOME:
        return "home";
    case K_END:
        return "end";
    case K_PGUP:
        return "pgup";
    case K_PGDN:
        return "pgdn";
    case K_LEFT:
        return "left";
    case K_RIGHT:
        return "right";
    case K_UP:
        return "up";
    case K_DOWN:
        return "down";
    case K_NDIV:
        return "num /";
    case K_NMUL:
        return "num *";
    case K_NMINUS:
        return "num -";
    case K_NPLUS:
        return "num +";
    case K_NENTER:
        return "num enter";
    case K_NCOMMA:
        return "num ,";
    case K_N1:
        return "num 1";
    case K_N2:
        return "num 2";
    case K_N3:
        return "num 3";
    case K_N4:
        return "num 4";
    case K_N5:
        return "num 5";
    case K_N6:
        return "num 6";
    case K_N7:
        return "num 7";
    case K_N8:
        return "num 8";
    case K_N9:
        return "num 9";
    case K_N0:
        return "num 0";
    default:
        return "??";
    }
}

int SDL_PollEvent(SDL_Event* event)
{
    uint8_t keydown, code;
    if (keyb_get(&keydown, &code))
    {
        event->type = keydown ? SDL_KEYDOWN : SDL_KEYUP;
        event->key.keysym.sym = code;
        return 1;
    }
    return 0;
}
