#include "String.hpp"
#include <string.h>

String::String() : str(0), len(0) {}

String::String(const char *_str)
{
    len = strlen(_str) + 1;
    str = new char[len];
    strcpy(str, _str);
    str[len - 1] = 0;
}

String::String(const String &a) : len(a.len)
{
    str = new char[a.len];
    strcpy(str, a.str);
    str[a.len - 1] = 0;
}

String::~String()
{
    if (str)
        delete[] str;
}

String::operator const char *() { return static_cast<const char *>(str); }
String::operator char *() { return str; }
String &String::operator=(const String &a)
{
    if (&a != this)
    {
        if (str)
            delete[] str;
        str = new char[a.len];
        strcpy(str, a.str);
        str[a.len - 1] = 0;
    }
    return *this;
}