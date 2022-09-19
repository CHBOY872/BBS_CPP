#ifndef USERDBHANDLER_HPP_SENTRY
#define USERDBHANDLER_HPP_SENTRY

#include "../DbHandler/DbHandler.hpp"

#define WRITING_FORMAT_USER "%250s %250s\n"
#define WRITING_FORMAT_LEN_USER 502

#define USER_NAME 250
#define USER_PASSWORD 250

struct UserStructure
{
    char nickname[USER_NAME];
    char password[USER_PASSWORD];

    void Clear();
    ~UserStructure() {}
};

#endif