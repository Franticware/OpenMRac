/* Copyright (c) 2023, Vojtěch Salajka. All rights reserved. Use of this source code is governed by a BSD-style license that can be found in the LICENSE file. */

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <conio.h>

#include "glide3drv_h/glide3drv.h"

#define glideCount 5

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    static const char* driverFile = "glide3x.dxe";

    const unsigned char* glideData[glideCount] = {
        glide3x_v1,
        glide3x_v2,
        glide3x_v3,
        glide3x_v4,
        glide3x_vr,
    };
    const size_t glideSize[glideCount] = {
        sizeof(glide3x_v1),
        sizeof(glide3x_v2),
        sizeof(glide3x_v3),
        sizeof(glide3x_v4),
        sizeof(glide3x_vr),
    };

    size_t maxGlideSize = 0;
    for (int i = 0; i != glideCount; ++i)
    {
        if (glideSize[i] > maxGlideSize)
            maxGlideSize = glideSize[i];
    }

    maxGlideSize += 8;

    unsigned char* dataBuffer = (unsigned char*)malloc(maxGlideSize);

    int selected = 0;
    bool unknown = true;

    FILE* fin = fopen(driverFile, "rb");
    if (fin)
    {
        size_t currentGlideSize = fread(dataBuffer, 1, maxGlideSize, fin);
        fclose(fin);

        for (int i = 0; i != glideCount; ++i)
        {
            if (glideSize[i] == currentGlideSize && memcmp(dataBuffer, glideData[i], glideSize[i]) == 0)
            {
                unknown = false;
                selected = i;
                break;
            }
        }

    }

    free(dataBuffer);

    clrscr();

    cprintf("\r\n");
    cprintf(" OpenMRac 3dfx Setup\r\n");
    for (int i = 0; i != glideCount + 2; ++i)
    {
        cprintf("\r\n");
    }
    cprintf(" Select your 3dfx card and hit Enter.");
    if (unknown)
    {
        cprintf("\r\n No known driver is currently set.");
    }

    static const char* voodoos[glideCount] = {
        " Voodoo            ",
        " Voodoo 2          ",
        " Voodoo 3, Banshee ",
        " Voodoo 4, 5       ",
        " Voodoo Rush       "
    };

    bool cancel = false;

    for (;;)
    {
        for (int i = 0; i != glideCount; ++i)
        {
            gotoxy(3, 4 + i);
            if (i == selected)
            {
                textcolor(WHITE);
                textbackground(RED);
            }
            else
            {
                textcolor(LIGHTGRAY);
                textbackground(BLACK);
            }

            cprintf("%s", voodoos[i]);
        }

        gotoxy(0, 0);

        int key = getch();
        int key2 = 0;
        if (key == 0)
        {
            key2 = getch();
        }

        if (key == 0 && key2 == 72)
        {
            --selected;
            if (selected < 0) selected = 0;
        }
        else if (key == 0 && key2 == 80)
        {
            ++selected;
            if (selected > 4) selected = 4;
        }
        else if (key == 13)
        {
            break;
        }
        else if (key == 27)
        {
            cancel = true;
            break;
        }
    }

    textcolor(LIGHTGRAY);
    textbackground(BLACK);
    clrscr();

    if (cancel)
    {
        cprintf("3dfx driver was NOT set.\r\n");
    }
    else
    {
        bool ok = false;
        FILE* fout = fopen(driverFile, "wb");
        if (fout)
        {
            size_t written = fwrite(glideData[selected], 1, glideSize[selected], fout);
            fclose(fout);
            ok = written == glideSize[selected];
        }

        if (ok)
        {
            cprintf("3dfx driver was successfully set.\r\n");
        }
        else
        {
            cprintf("Error setting 3dfx driver.\r\n");
        }
    }

    return 0;
}
