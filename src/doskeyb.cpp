#include "doskeyb.h"

#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <sys/nearptr.h>
#include <dpmi.h>
#include <go32.h>

unsigned char keyb_state_int[KEYB_SIZE] = {0};

typedef struct QueueT {
    uint8_t begin;
    uint8_t end;
    uint16_t data[0x100];
} Queue;

#define QueueNotEmpty(q) ((q).begin != (q).end)
#define QueueGet(q) ((q).data[(q).begin++])
#define QueuePut(q, d) do { (q).data[(q).end++] = (d); } while (0)

Queue keyb_queue = { /*.begin =*/ 0, /*.end =*/ 0, {0}};

int keyb_get(uint8_t* keydown, uint8_t* code)
{
    if (QueueNotEmpty(keyb_queue))
    {
        uint16_t v = QueueGet(keyb_queue);
        *keydown = (uint8_t)(v >> 8);
        *code = (uint8_t)v;
        return 1;
    }
    else
    {
        return 0;
    }
}

#define LOCK_VARIABLE(x)    _go32_dpmi_lock_data((void *)&x,(long)sizeof(x));
#define LOCK_FUNCTION(x)    _go32_dpmi_lock_code((void*)x,(long)x##_end - (long)x);
#define END_OF_FUNCTION(x)    void x##_end(void) { }

_go32_dpmi_seginfo old_keyb_handler_seginfo, new_keyb_handler_seginfo;

void keyb_handler(void)
{
    static unsigned char buffer = 0;
    unsigned char rawcode;
    unsigned char make_break;
    unsigned char scancode;
    uint16_t v;
    rawcode = inportb(0x60); /* read scancode from keyboard controller */
    make_break = !(rawcode & 0x80); /* bit 7: 0 = make, 1 = break */
    scancode = rawcode & 0x7F;
    if (buffer == 0xE0) { /* second byte of an extended key */
        if (scancode < 0x60) {
            if (keyb_state_int[scancode + KEYB_E] != !!make_break)
            {
                keyb_state_int[scancode + KEYB_E] = !!make_break;
                v = ((uint16_t)(scancode + KEYB_E) | ((uint16_t)(!!make_break) << 8));
                QueuePut(keyb_queue, v);
            }
        }
        buffer = 0;
    } else if (buffer >= 0xE1 && buffer <= 0xE2) {
        buffer = 0; /* ingore these extended keys */
    } else if (rawcode >= 0xE0 && rawcode <= 0xE2) {
        buffer = rawcode; /* first byte of an extended key */
    } else if (scancode < 0x60) {
        if (keyb_state_int[scancode] != !!make_break)
        {
            keyb_state_int[scancode] = !!make_break;
            v = ((uint16_t)(scancode) | ((uint16_t)(!!make_break) << 8));
            QueuePut(keyb_queue, v);
        }
    }
    outportb(0x20, 0x20); /* must send EOI to finish interrupt */
}
END_OF_FUNCTION(keyb_handler)

int keyb_hook_int(void)
{
    _go32_dpmi_lock_data( (char *)&keyb_queue, sizeof(Queue) );	// locks the memory region of the keyb array
    _go32_dpmi_lock_data( (char *)keyb_state_int, sizeof(KEYB_SIZE) );	// locks the memory region of the keyb array
    _go32_dpmi_lock_code((void*)keyb_handler, (unsigned long)keyb_handler_end - (unsigned long)keyb_handler);		// locks the code region of keyb_handler
    _go32_dpmi_get_protected_mode_interrupt_vector(9, &old_keyb_handler_seginfo);	// store old keyboard handler
    new_keyb_handler_seginfo.pm_offset = (int)keyb_handler;	// function to call if a key is pressed is keyb_handler
    if (_go32_dpmi_allocate_iret_wrapper(&new_keyb_handler_seginfo) != 0)
    {   // register this function
        return 1;
    }
    if (_go32_dpmi_set_protected_mode_interrupt_vector(9, &new_keyb_handler_seginfo) != 0)
    {	// another part of it
        _go32_dpmi_free_iret_wrapper(&new_keyb_handler_seginfo);
        return 1;
    }
    return 0;
}

void keyb_unhook_int(void)
{	// disables they interrupt handler
    _go32_dpmi_set_protected_mode_interrupt_vector(9, &old_keyb_handler_seginfo);	// restore old handler
    _go32_dpmi_free_iret_wrapper(&new_keyb_handler_seginfo);	// free interrupt capturing
}
