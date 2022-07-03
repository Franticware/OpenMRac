#if defined(__MACOSX__)
#include <cstdio>
#include <setjmp.h>

#include <SDL/SDL_rwops.h>
#include <SDL_image/SDL_image.h>

#include "pict2.h"

int Pict2::loadjpeg(const char* fname, unsigned int mode)
{
    return loadjpeg_pom(true, fname, 0, mode);
}

int Pict2::loadjpeg(const void* data, unsigned int size, unsigned int mode)
{
    return loadjpeg_pom(false, data, size, mode);
}

inline Uint32 getSurfacePixel(SDL_Surface *surface, int x, int y)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to retrieve */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
	
    switch(bpp) {
		case 1:
			return *p;
			break;
			
		case 2:
			return *(Uint16 *)p;
			break;
			
		case 3:
			if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
				return p[0] << 16 | p[1] << 8 | p[2];
			else
				return p[0] | p[1] << 8 | p[2] << 16;
			break;
			
		case 4:
			return *(Uint32 *)p;
			break;
			
		default:
			return 0;       /* shouldn't happen, but avoids warnings */
    }
	return 0;
}

int Pict2::loadjpeg_pom(bool bfile, const void* fname_data, unsigned int data_size, unsigned int mode)
{
	if (bfile)
	{
		return 0;
	}
	
	if (mode != 1 && mode != 3 && mode != 4)
	{
		return 0;
	}
	
	SDL_Surface* imageSurface = IMG_LoadTyped_RW(SDL_RWFromConstMem(fname_data, data_size), 1, "JPG");
	
	create(imageSurface->w, imageSurface->h, mode, 0);
	
	SDL_LockSurface(imageSurface);
	Uint8 components[4];
	for (int y = 0; y != h(); ++y)
	{
		for (int x = 0; x != w(); ++x)
		{
			Uint32 pixel = getSurfacePixel(imageSurface, x, h() - y - 1);
			SDL_GetRGBA(pixel, imageSurface->format, components, components+1, components+2, components+3);
			components[3] = 255;
			for (int c = 0; c != mode; ++c)
			{
				px()[(x + y * w()) * mode + c] = components[c];
			}
		}
	}
	SDL_UnlockSurface(imageSurface);
	SDL_FreeSurface(imageSurface);
	
	return 1;
}
#endif
