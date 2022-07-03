#include "fopendir.h"
#include <SDL/SDL.h>

// inspired by SDL 2.0

#if defined(__unix__) || defined(__linux__) || defined(__HAIKU__)
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

FILE* fopenDir(const char* filename, const char* mode, const char* org, const char* app)
{
    const char *envr = SDL_getenv("HOME");
    if (!envr)
    {
        return 0;
    }
    char filepath[1024] = {0};
    if (org && app)
    {
        snprintf(filepath, 1023, "%s/.local/share/%s/%s/%s", envr, org, app, filename);
    }
    else
    {
        snprintf(filepath, 1023, "%s/%s", envr, filename);
    }
    //fprintf(stderr, "%s\n", filepath);
    for (int i = 1; i < static_cast<int>(strlen(filepath)); ++i)
    {
        if (filepath[i] == '/')
        {
            filepath[i] = '\0';
            if (mkdir(filepath, 0700) != 0 && errno != EEXIST)
            {
                return 0;
            }
            filepath[i] = '/';
        }
    }
    return fopen(filepath, mode);
}
#elif defined(__WIN32__)
#include <windows.h>
#include <shlobj.h>

FILE* fopenDir(const char* filename, const char* mode, const char* org, const char* app)
{
    WCHAR basePath[MAX_PATH+1] = {0};
    WCHAR path[MAX_PATH+1] = {0};
    WCHAR wideMode[64] = {0};
    _snwprintf(wideMode, 63, L"%S", mode);
    if (org && app)
    {
        if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, basePath)))
        {
            _snwprintf(path, MAX_PATH, L"%ls\\%S", basePath, org);
            CreateDirectoryW(path, NULL);
            _snwprintf(path, MAX_PATH, L"%ls\\%S\\%S", basePath, org, app);
            CreateDirectoryW(path, NULL);
            _snwprintf(path, MAX_PATH, L"%ls\\%S\\%S\\%S", basePath, org, app, filename);
        }
        else
        {
            return 0;
        }
    }
    else
    {
        if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_MYPICTURES | CSIDL_FLAG_CREATE, NULL, 0, basePath)))
        {
            _snwprintf(path, MAX_PATH, L"%ls\\%S", basePath, filename);
        }
        else
        {
            return 0;
        }
    }
    return _wfopen(path, wideMode);
}
#elif defined(__MACOSX__)

// implementace v fopendir.m

#else
FILE* fopenDir(const char* filename, const char* mode, const char* org, const char* app)
{
    return fopen(filename, mode);
}
#endif
