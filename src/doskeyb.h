#ifndef KEYB_H
#define KEYB_H

#include <stdint.h>

#include "scancodes.h"

extern unsigned char keyb_state_int[KEYB_SIZE];

int keyb_get(uint8_t* keydown, uint8_t* code);
int keyb_hook_int(void);
void keyb_unhook_int(void);

#endif // KEYB_H
