#include "platform.h"

#include <cstring>

bool isSpace(int c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r';
}

bool isDigit(int c)
{
    return c >= '0' && c <= '9';
}

bool strSuff(const char* str, const char* suff)
{
    if (strlen(str) < strlen(suff))
        return false;
    return !strcmp(str+strlen(str)-strlen(suff), suff);
}

bool strstrtok1(const char* str1, const char* str2)
{
    const char* str_pos = str1;
    while ((str_pos = strstr(str_pos, str2)))
    {
        if ((str_pos == str1 || isSpace(*(str_pos-1))) && (isSpace(str_pos[strlen(str2)]) || str_pos[strlen(str2)] == '\0'))
            return true;
        ++str_pos;
    }
    return false;
}

void uncomment(char* str)
{
    bool insideString = false;
    for (unsigned i = 0; i != strlen(str); ++i)
    {
        if (str[i] == '"')
        {
            insideString = !insideString;
        }
        if (!insideString)
        {
            if (str[i] == '/' && str[i+1] == '/')
            {
                str[i] = 0;
                return;
            }
        }
    }
    /*char* str_pom = strstr(str, "//");
    if (str_pom != NULL)
        *str_pom = '\0';*/
}

void strncat1(char* str_o, const char* str1_i, const char* str2_i, size_t num)
{
    bool b1 = true;
    unsigned int i = 0;
    while (i != num)
    {
        if (b1)
        {
            if (*str1_i)
            {
                *str_o = *str1_i;
                ++str_o;
                ++str1_i;
            } else {
                b1 = false;
            }
        }
        if (!b1)
        {
            if (*str2_i)
            {
                *str_o = *str2_i;
                ++str_o;
                ++str2_i;
            } else {
                *str_o = '\0';
                break;
            }
        }
        ++i;
    }
}

bool strempty(const char* str)
{
    for (unsigned int i = 0; i != strlen(str); ++i)
    {
        if (!isSpace(str[i]))
            return false;
    }
    return true;
}

char* trim(char* str)
{
    char* ret = str;
    for (size_t i = 0; i != strlen(str); ++i)
    {
        //if (str[i] != ' ' && str[i] != '\r' &&
        if (!isSpace(str[i]))
        {
            ret = str + i;
            break;
        }
    }
    for (size_t i = strlen(ret); i != 0; --i)
    {
        if (isSpace(ret[i-1]))
        {
            ret[i-1] = 0;
        }
        else
        {
            break;
        }
    }
    return ret;
}
