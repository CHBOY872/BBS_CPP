#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "FileDbHandler.hpp"

template <>
DbHandler<FileStructure>::DbHandler(const char *_file_path, const char *_format,
                                    const int _format_len)
    : file_path(_file_path), format(_format), format_len(_format_len) {}

template <>
DbHandler<FileStructure>::~DbHandler()
{
}

template <>
DbHandler<FileStructure> *DbHandler<FileStructure>::Make(const char *_file_path,
                                                         const char *_format,
                                                         const int _format_len)
{
    return new DbHandler<FileStructure>(_file_path, _format, _format_len);
}

template <>
int DbHandler<FileStructure>::GetByName(const char *name,
                                        FileStructure *object_to)
{
    FileStructure *tmp = object_to;
    if (!tmp)
    {
        tmp = &temp;
        tmp->Clear();
    }

    FILE *f = fopen(file_path, "r");
    if (!f)
        return -1;

    int i = 0;
    while (fscanf(f, format, tmp->file_name,
                  tmp->author_nickname, &tmp->perms) != EOF)
    {
        if (!strcmp((char *)name, tmp->file_name))
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
void DbHandler<FileStructure>::Add(FileStructure *object)
{
    FILE *to = fopen(file_path, "a");
    if (!to)
        return;
    fprintf(to, format,
            object->file_name, object->author_nickname, object->perms);
    fclose(to);
}

template <>
int DbHandler<FileStructure>::EditByName(FileStructure *object,
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
    fprintf(where, format, object->file_name,
            object->author_nickname, object->perms);
    fclose(where);
    return stat;
}

template <>
void DbHandler<FileStructure>::RemoveByName(const char *by_name)
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
    fprintf(where, format, "NULL", "NULL", 0000);
    fclose(where);
}

void FileStructure::Clear()
{
    memset(author_nickname, 0, USER_NICKNAME_LEN);
    memset(file_name, 0, FILE_NAME_LEN);
    perms = 0;
}