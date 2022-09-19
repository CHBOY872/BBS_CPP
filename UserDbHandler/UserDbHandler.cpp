#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../DbHandler/DbHandler.hpp"
#include "UserDbHandler.hpp"

template <>
DbHandler<UserStructure>::DbHandler(const char *_file_path, const char *_format,
                                    const int _format_len)
    : file_path(_file_path), format(_format), format_len(_format_len) {}

template <>
DbHandler<UserStructure>::~DbHandler()
{
}

template <>
DbHandler<UserStructure> *DbHandler<UserStructure>::Make(const char *_file_path,
                                                         const char *_format,
                                                         const int _format_len)
{
    return new DbHandler<UserStructure>(_file_path, _format, _format_len);
}

template <>
int DbHandler<UserStructure>::GetByName(const char *name,
                                        UserStructure *object_to)
{
    UserStructure *tmp = object_to;
    if (!tmp)
    {
        tmp = &temp;
        tmp->Clear();
    }

    FILE *f = fopen(file_path, "r");
    if (!f)
        return -1;

    int i = 0;
    while (fscanf(f, format, tmp->nickname, tmp->password) != EOF)
    {
        if (!strcmp((char *)name, tmp->nickname))
        {
            fclose(f);
            return i;
        }
        i++;
    }
    fclose(f);
    return -1;
}

template <>
void DbHandler<UserStructure>::Add(UserStructure *object)
{
    FILE *to = fopen(file_path, "a");
    if (!to)
        return;
    fprintf(to, format, object->nickname, object->password);
    fclose(to);
}

template <>
int DbHandler<UserStructure>::EditByName(UserStructure *object,
                                         const char *by_name)
{
    int stat = GetByName(by_name, object);
    if (stat == -1)
        return -1;
    FILE *where = fopen(file_path, "r+");
    if (!where)
    {
        perror("edit file");
        return -1;
    }

    fseek(where, format_len * stat, SEEK_SET);
    fprintf(where, format, object->nickname, object->password);
    fclose(where);
    return stat;
}

template <>
void DbHandler<UserStructure>::RemoveByName(const char *by_name)
{
    int stat = GetByName(by_name, &temp);
    if (stat == -1)
        return;
    FILE *where = fopen(file_path, "r+");
    if (!where)
    {
        perror("delete file");
        return;
    }

    fseek(where, format_len * stat, SEEK_SET);
    fprintf(where, format, "NULL", "NULL");
    fclose(where);
}

template <>
const char *DbHandler<UserStructure>::GetFilePath() { return file_path; }

void UserStructure::Clear()
{
    memset(nickname, 0, USER_NAME);
    memset(password, 0, USER_PASSWORD);
}