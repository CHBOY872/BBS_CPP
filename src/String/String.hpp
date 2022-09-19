#ifndef STRING_HPP_SENTRY
#define STRING_HPP_SENTRY

class String
{
    char *str;
    int len;

public:
    String();
    String(const char *_str);
    String(const String &);
    ~String();
    operator const char *();
    operator char *();
    String &operator=(const String &);
};

#endif