#ifndef FILEDBHANDLER_HPP_SENTRY
#define FILEDBHANDLER_HPP_SENTRY

#include "../DbHandler/DbHandler.hpp"

#define WRITING_FORMAT_FILE "%250s %250s %8o\n"
#define WRITING_FORMAT_LEN_FILE 511

#define FILE_NAME_LEN 250
#define USER_NICKNAME_LEN 250

struct FileStructure : Object
{
    char file_name[FILE_NAME_LEN];
    char author_nickname[USER_NICKNAME_LEN];
    int perms;

    virtual void Clear();
    virtual ~FileStructure() {}
};

#endif