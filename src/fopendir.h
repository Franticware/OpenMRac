#ifndef FOPENDIR_H
#define FOPENDIR_H

#include <cstdio>

#if defined(__cplusplus) && defined(__MACOSX__)
extern "C" {
#endif

// pokud je org == 0 nebo app == 0, otevři soubor v Dokumentech (Windows) nebo v home (Linux, OS X)
// jinak v "data aplikací"
FILE* fopenDir(const char* filename, const char* mode, const char* org = 0, const char* app = 0);

#if defined(__cplusplus) && defined(__MACOSX__)
}
#endif
	
#endif // FOPENDIR_H
