#ifndef HLIDAC_CSTRING1_H
#define HLIDAC_CSTRING1_H

#include <cstring>

bool isSpace(int c);
bool isDigit(int c);
bool strSuff(const char* str, const char* suff);
bool strstrtok1(const char* str1, const char* str2);
void uncomment(char* str);
void strncat1(char* str_o, const char* str1_i, const char* str2_i, size_t num);
bool strempty(const char* str);
char* trim(char* str);

#endif
