#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>

#include "DbHandler/DbHandler.hpp"
#include "FileDbHandler/FileDbHandler.hpp"
#include "UserDbHandler/UserDbHandler.hpp"
#include "Server/Server.hpp"
#include "String/String.hpp"

static int port = 8808;

enum
{
    max_path_len = 256
};

static const char list_of_users[] = "list_of_users.txt";
static const char list_of_files[] = "list_of_files.txt";
static const char downloadings_path[] = "downloadings";

static const char *main_dir_path = NULL;

int init_file(const char *path, int flags, int perms)
{
    int fd = open(path, flags, perms);
    if (-1 == fd)
        return -1;
    close(fd);
    return 0;
}

int init_directory(const char *path, int perms)
{
    DIR *download_dir = opendir(path);
    if (!download_dir)
    {
        int stat = mkdir(path, perms);
        if (-1 == stat)
            return -1;
    }
    else
    {
        chmod(path, 0777);
        closedir(download_dir);
    }
    return 0;
}

int init_programm(char *file_file_path, char *user_file_path,
                  char *directory_path)
{
    int len;
    len = sprintf(user_file_path, "%s/%s", main_dir_path, list_of_users);

    if (len >= max_path_len)
        return -1;

    user_file_path[len] = 0;
    if (init_file(user_file_path, O_CREAT | O_RDWR, 0666))
        return -1;

    len = sprintf(file_file_path, "%s/%s", main_dir_path, list_of_files);
    if (len >= max_path_len)
        return -1;

    file_file_path[len] = 0;
    if (init_file(file_file_path, O_CREAT | O_RDWR, 0666))
        return -1;

    len = sprintf(directory_path, "%s/%s", main_dir_path, downloadings_path);
    if (len >= max_path_len)
        return -1;

    directory_path[len] = 0;
    if (init_directory(directory_path, 0777))
        return -1;

    return 0;
}

int main(int argc, const char **argv)
{
    char file_path[max_path_len];
    char user_path[max_path_len];
    char directory_path[max_path_len];
    if (argc != 2)
    {
        fprintf(stderr, "Usage: <directory>\n");
        return 1;
    }
    DIR *dir = opendir(argv[1]);
    if (!dir)
    {
        fprintf(stderr, "No such directory\n");
        return 2;
    }
    closedir(dir);
    main_dir_path = argv[1];
    chmod(main_dir_path, 0777);
    if (-1 == init_programm(file_path, user_path, directory_path))
    {
        perror("init");
        return 3;
    }

    DbHandler<FileStructure> *file_db =
        DbHandler<FileStructure>::Make(file_path, WRITING_FORMAT_FILE,
                                       WRITING_FORMAT_LEN_FILE);
    DbHandler<UserStructure> *user_db =
        DbHandler<UserStructure>::Make(user_path, WRITING_FORMAT_USER,
                                       WRITING_FORMAT_LEN_USER);

    EventSelector *sel = new EventSelector;
    Server *serv = Server::Start(port, sel, file_db, user_db, directory_path);
    if (!serv)
    {
        perror("server");
        return 1;
    }
    sel->Run();
    delete sel;
    delete serv;
    return 0;
}
