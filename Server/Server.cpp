#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/select.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

static const char *commands[] = {"login",    /* log in to account */
                                 "logout",   /* logout from an account */
                                 "q",        /* quit */
                                 "register", /* register an acoount */
                                 "password", /* change password */
                                 "put",      /* put a file */
                                 "get",      /* get a file */
                                 "remove"};  /* remove a file */

static const char *responds[] = {"REGISTER\n",
                                 "NOREGISTER\n",
                                 "WRITE\n",
                                 "READ\n",
                                 "DIALOG\n",
                                 "ENDDIALOG\n"};

static char greetings_msg[] = "Welcome!\n";
static char account_have_msg[] = "Do you have an account? ([Y/y] - yes,"
                                 " [N/n] - no) : ";
static char no_authorized_msg[] = "You are connected as NOauthorized user\n";
static char login_msg[] = "Type your nickname: ";
static char password_msg[] = "Type your password: ";

static char type_nickname_msg[] = "Please type a nickname without any spaces\n";
static char type_password_msg[] = "Please type a password without any spaces\n";
static char type_a_file_name_msg[] = "Please type a password without any "
                                     "spaces and '/' '.'\n";
static char type_another_file_name_msg[] = "Type another file name: ";

static char incorrect_cred[] = "Incorrect password or user not found\n";

static char unknown_msg[] = "Unknown...\n";

static char user_is_exist_msg[] = "User with that nickname has already exist\n";
static char success_registation_msg[] = "Registation successfull\n";
static char change_pass_msg[] = "Type a new password: ";
static char reset_pass_fail_msg[] = "Failed reseting password\n";

static char write_name_file_msg[] = "Write a name of file: ";
static char set_perms_msg[] = "Please set permissions to file: ";
static char write_complete_perm_msg[] = "Write complete permission: ";

#include "Server.hpp"

EventSelector::~EventSelector()
{
    if (fd_array)
        delete[] fd_array;
}

void EventSelector::Add(FdHandler *fh)
{
    int fd = fh->GetFd();
    int i;
    if (fd_array)
    {
        fd_array_len = fd < current_max_count ? current_max_count : fd + 1;
        fd_array = new FdHandler *[fd_array_len];
        for (i = 0; i < fd_array_len; i++)
            fd_array[i] = 0;
        max_fd = -1;
    }
    if (fd >= fd_array_len)
    {
        FdHandler **tmp = new FdHandler *[fd + 1];
        for (i = 0; i < fd; i++)
            tmp[i] = i < fd_array_len ? fd_array[i] : 0;
        fd_array_len = fd + 1;
        delete fd_array;
        fd_array = tmp;
    }
    if (fd > max_fd)
        max_fd = fd;
    fd_array[fd] = fh;
}

void EventSelector::Remove(FdHandler *fh)
{
    int fd = fh->GetFd();
    if (fd >= fd_array_len || fh != fd_array[fd])
        return;
    fd_array[fd] = 0;
    if (fd == max_fd)
    {
        while (max_fd >= 0 && !fd_array[max_fd])
            max_fd--;
    }
}

void EventSelector::Run()
{
    do
    {
        int i, res;
        fd_set rds, wrs;
        FD_ZERO(&rds);
        FD_ZERO(&wrs);
        for (i = 0; i <= max_fd; i++)
        {
            if (fd_array[i])
            {
                if (fd_array[i]->WantRead())
                    FD_SET(fd_array[i]->GetFd(), &rds);
                if (fd_array[i]->WantWrite())
                    FD_SET(fd_array[i]->GetFd(), &wrs);
            }
        }
        res = select(max_fd + 1, &rds, &wrs, 0, 0);
        if (res <= 0)
            return;
        for (i = 0; i <= max_fd; i++)
        {
            if (fd_array[i])
            {
                bool r = FD_ISSET(fd_array[i]->GetFd(), &rds);
                bool w = FD_ISSET(fd_array[i]->GetFd(), &wrs);
                if (r || w)
                    fd_array[i]->Handle(r, w);
            }
        }
    } while (true);
}

////////////////////////////////////////

Server::Server(int fd, EventSelector *_the_selector,
               DbHandler<FileStructure> *_file_db_handler,
               DbHandler<UserStructure> *_user_db_handler,
               const char *_directory_path)
    : FdHandler(fd),
      the_selector(_the_selector),
      file_db_handler(_file_db_handler),
      user_db_handler(_user_db_handler),
      directory_path(_directory_path),
      first(0)
{
    _the_selector->Add(this);
}

Server::~Server()
{
    while (first)
    {
        item *tmp = first;
        first = first->next;
        the_selector->Remove(tmp->cl);
        delete tmp->cl;
        delete tmp;
    }
}

Server *Server::Start(int port, EventSelector *_the_selector,
                      DbHandler<FileStructure> *_file_db_handler,
                      DbHandler<UserStructure> *_user_db_handler,
                      const char *_directory_path)
{
    int fd_s, stat, opt;
    fd_s = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == fd_s)
        return 0;
    opt = 1;
    setsockopt(fd_s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    stat = bind(fd_s, (struct sockaddr *)&addr, sizeof(addr));
    if (-1 == stat)
        return 0;

    stat = listen(fd_s, current_max_count);
    if (-1 == stat)
        return 0;

    return new Server(fd_s, _the_selector, _file_db_handler,
                      _user_db_handler, _directory_path);
}

void Server::RemoveClient(Client *cl)
{
    item **p;
    for (p = &first; *p; p = &((*p)->next))
    {
        if (cl == (*p)->cl)
        {
            item *tmp = *p;
            *p = tmp->next;
            delete tmp->cl;
            delete tmp;
            return;
        }
    }
}

void Server::Handle(bool r, bool w)
{
    if (!r)
        return;

    struct sockaddr_in addr;
    socklen_t len;
    int cl_fd = accept(GetFd(), (struct sockaddr *)&addr, &len);
    if (-1 == cl_fd)
        return;

    item *tmp = new item;
    tmp->cl = new Client(cl_fd, this);
    tmp->next = first;
    first = tmp;

    SendMsg(cl_fd, responds[4], sizeof(responds[4])); /* DIALOG */
    SendMsg(cl_fd, greetings_msg, sizeof(greetings_msg));
    SendMsg(cl_fd, account_have_msg, sizeof(account_have_msg));
}

void Server::SendMsg(int fd, const char *msg, int len)
{
    write(fd, msg, len);
}

////////////////////////////////////////

Client::Client(int _fd, Server *_the_master)
    : FdHandler(_fd),
      the_master(_the_master),
      user(0),
      name(0),
      step(step_authorization_uninitialized),
      prev_step(step_authorization_uninitialized),
      file_fd(-1),
      buf_used(0),
      reg_step(step_registration_no),
      file(0)
{
    memset(buf, 0, BUFFERSIZE);
}

Client::~Client()
{
    if (user)
        delete user;
    if (name)
        delete[] name;
    if (file)
        delete file;
}

void Client::Handle(bool r, bool w)
{
    if (r)
    {
        int fd = GetFd();
        int i, pos = -1;
        int rc = read(fd, buf, BUFFERSIZE - buf_used);
        if (rc < 0 && step != step_is_put)
            the_master->RemoveClient(this);
        if (rc + buf_used > BUFFERSIZE)
            the_master->RemoveClient(this);
        if (step == step_is_put)
        {
            write(file_fd, buf, rc);
            if (rc != BUFFERSIZE)
            {
                close(file_fd);
                file_fd = -1;
                step = prev_step;
            }
        }
        for (i = 0; i < rc; i++)
        {
            if (buf[buf_used + i] == '\n')
            {
                pos = i;
                break;
            }
        }
        if (pos == -1)
            buf_used += rc;

        char *str = new char[pos + buf_used + 1];
        memcpy(str, buf, buf_used + pos + 1);
        str[buf_used + pos] = 0;
        int res = ClientHandle(str);
        delete[] str;
        memset(buf, 0, buf_used + rc);
        buf_used = 0;
        if (res == -1)
            the_master->RemoveClient(this);
    }
    if (w)
    {
        int rc = read(file_fd, buf, BUFFERSIZE - buf_used);
        write(GetFd(), buf, rc);
        if (rc != BUFFERSIZE)
        {
            close(file_fd);
            write(GetFd(), "", 0);
            SetWrite(false);
            step = prev_step;
        }
    }
}

void Client::RemoveFile(const char *str)
{
    if (-1 != the_master->file_db_handler->GetByName(str))
    {
        if (!strcmp(name, file->author_nickname))
        {
            char *file_name = new char[strlen(str) + 2 +
                                       strlen(the_master->GetDirectoryPath())];
            sprintf(file_name, "%s/%s", the_master->GetDirectoryPath(), str);
            int stat = remove(file_name);
            if (-1 == stat)
            {
                step = prev_step;
                the_master->SendMsg(GetFd(), responds[5],
                                    strlen(responds[5]) + 1);
            }
            else
                the_master->file_db_handler->RemoveByName(str);
            delete[] file_name;
            step = prev_step;
            the_master->SendMsg(GetFd(), responds[5], strlen(responds[5]) + 1);
        }
        else
        {
            step = prev_step;
            the_master->SendMsg(GetFd(), responds[5], strlen(responds[5]) + 1);
        }
    }
    else
    {
        step = prev_step;
        the_master->SendMsg(GetFd(), responds[5], strlen(responds[5]) + 1);
    }
}

void Client::RegisterHandle(const char *str)
{
    switch (reg_step)
    {
    case step_registration_login:
        if (!strcmp("", str) || strstr(str, " "))
            the_master->SendMsg(GetFd(), type_nickname_msg,
                                sizeof(type_nickname_msg));
        else
        {
            user = new UserStructure;
            if (-1 != the_master->user_db_handler->GetByName(str, user))
                the_master->SendMsg(GetFd(), user_is_exist_msg,
                                    sizeof(user_is_exist_msg));
            else
            {
                memset(user->nickname, 0, USER_NAME);
                memset(user->password, 0, USER_PASSWORD);
                strcpy(user->nickname, str);
                reg_step = step_registration_password;
                the_master->SendMsg(GetFd(), password_msg,
                                    sizeof(password_msg));
            }
        }
        break;
    case step_registration_password:
        if (!strcmp("", str) || strstr(str, " "))
            the_master->SendMsg(GetFd(), type_password_msg,
                                sizeof(type_password_msg));
        else
        {
            strcpy(user->password, str);
            the_master->user_db_handler->Add(user);
            reg_step = step_registration_no;
            step = step_authorization_noauthorized;
            char *to_msg = new char[sizeof(success_registation_msg) +
                                    strlen(responds[5]) + 1];
            sprintf(to_msg, "%s%s", success_registation_msg, responds[5]);
            the_master->SendMsg(GetFd(), to_msg, strlen(to_msg) + 1);
            delete[] to_msg;
            delete user;
            user = 0;
        }
        break;
    default:
        break;
    }
}

void Client::StartAuthorization(const char *str)
{
    if (!strcmp("Y", str) || !strcmp("y", str))
    {
        step = step_authorization_unauthorized_login;
        the_master->SendMsg(GetFd(), login_msg, sizeof(login_msg));
    }
    else if (!strcmp("N", str) || !strcmp("n", str))
    {
        step = step_authorization_noauthorized;
        char *to_msg = new char[sizeof(no_authorized_msg) +
                                strlen(responds[1]) +
                                strlen(responds[5]) + 1];
        sprintf(to_msg, "%s%s%s", responds[1], responds[5],
                no_authorized_msg);
        the_master->SendMsg(GetFd(), to_msg, strlen(to_msg) + 1);
        delete[] to_msg;
    }
    else
        the_master->SendMsg(GetFd(), unknown_msg, sizeof(unknown_msg));
}

void Client::Login(const char *str)
{
    if (!strcmp("", str) || strstr(str, " "))
        the_master->SendMsg(GetFd(), type_nickname_msg,
                            sizeof(type_nickname_msg));
    else
    {
        int name_len = strlen(str);
        name = new char[name_len + 1];
        strcpy(name, str);
        name[name_len] = 0;
        step = step_authorization_unauthorized_password;
        the_master->SendMsg(GetFd(), password_msg, sizeof(password_msg));
    }
}

void Client::CheckPasswordCredentials(const char *str)
{
    if (!strcmp("", str) || strstr(str, " "))
        the_master->SendMsg(GetFd(), type_password_msg,
                            sizeof(type_password_msg));
    else
    {
        if (-1 == the_master->user_db_handler->GetByName(name))
        {
            delete[] name;
            name = 0;
            step = step_authorization_uninitialized;
            char *to_msg = new char[sizeof(incorrect_cred) +
                                    sizeof(account_have_msg)];
            sprintf(to_msg, "%s%s", incorrect_cred, account_have_msg);
            the_master->SendMsg(GetFd(), to_msg, strlen(to_msg) + 1);
            delete[] to_msg;
        }
        else
        {
            if (!strcmp(str, user->password))
            {
                char *to_msg = new char[strlen(responds[5]) +
                                        strlen(responds[0]) + 1];
                sprintf(to_msg, "%s%s", responds[5], responds[0]);
                the_master->SendMsg(GetFd(), to_msg, strlen(to_msg) + 1);
                delete[] to_msg;
                step = step_authorization_authorized;
            }
            else
            {
                char *to_msg = new char[sizeof(incorrect_cred) +
                                        sizeof(account_have_msg)];
                sprintf(to_msg, "%s%s", incorrect_cred, account_have_msg);
                the_master->SendMsg(GetFd(), to_msg, strlen(to_msg) + 1);
                delete[] to_msg;
                step = step_authorization_uninitialized;
            }
        }
    }
}

void Client::CommandHandle(const char *str, char *q_flag)
{
    switch (step)
    {
    case step_authorization_authorized:
        if (!strcmp(str, commands[1])) /* logout */
        {
            delete[] name;
            name = 0;
            step = step_authorization_noauthorized;
            char *to_msg = new char[sizeof(no_authorized_msg) +
                                    strlen(responds[1]) + 1];
            sprintf(to_msg, "%s%s", responds[1], no_authorized_msg);
            the_master->SendMsg(GetFd(), to_msg, strlen(to_msg) + 1);
            delete[] to_msg;
        }
        else if (!strcmp(str, commands[4])) /* password */
        {
            step = step_authorization_change_password;
            char *to_msg = new char[sizeof(change_pass_msg) +
                                    strlen(responds[4]) + 1];
            sprintf(to_msg, "%s%s", responds[4], change_pass_msg);
            the_master->SendMsg(GetFd(), to_msg, strlen(to_msg) + 1);
            delete[] to_msg;
        }
        else if (!strcmp(str, commands[5])) /* put */
        {
            prev_step = step;
            step = step_want_put;
            char *to_msg = new char[sizeof(write_name_file_msg) +
                                    strlen(responds[4]) + 1];
            sprintf(to_msg, "%s%s", responds[4], write_name_file_msg);
            the_master->SendMsg(GetFd(), to_msg, strlen(to_msg) + 1);
            delete[] to_msg;
        }
        else if (!strcmp(str, commands[7])) /* remove */
        {
            prev_step = step;
            step = step_want_remove;
            char *to_msg = new char[sizeof(write_name_file_msg) +
                                    strlen(responds[4]) + 1];
            sprintf(to_msg, "%s%s", responds[4], write_name_file_msg);
            the_master->SendMsg(GetFd(), to_msg, strlen(to_msg) + 1);
            delete[] to_msg;
        }

    case step_authorization_noauthorized:
        if (!strcmp(str, commands[2])) /* q */
        {
            *q_flag = -1;
            return;
        }
        else if (!strcmp(str, commands[0])) /* login */
        {
            step = step_authorization_unauthorized_login;
            char *to_msg = new char[strlen(responds[4]) +
                                    sizeof(login_msg) + 1];
            sprintf(to_msg, "%s%s", responds[4], login_msg);
            the_master->SendMsg(GetFd(), to_msg, strlen(to_msg) + 1);
            free(to_msg);
        }
        else if (!strcmp(str, commands[3])) /* register */
        {
            step = step_authorization_register;
            reg_step = step_registration_login;
            char *to_msg = new char[sizeof(login_msg) +
                                    strlen(responds[4]) +
                                    strlen(responds[1]) + 1];
            sprintf(to_msg, "%s%s%s", responds[4], responds[1], login_msg);
            the_master->SendMsg(GetFd(), to_msg, strlen(to_msg) + 1);
            delete[] to_msg;
        }
        else if (!strcmp(str, commands[6])) /* get */
        {
            prev_step = step;
            step = step_want_get;
            the_master->SendMsg(GetFd(), write_name_file_msg,
                                sizeof(write_name_file_msg));
        }
        break;
    default:
        break;
    }
}

void Client::ChangePassword(const char *str)
{
    strcpy(user->nickname, name);
    strcpy(user->password, str);
    if (-1 == the_master->user_db_handler->EditByName(user, name))
    {
        step = step_authorization_authorized;
        the_master->SendMsg(GetFd(), reset_pass_fail_msg,
                            sizeof(reset_pass_fail_msg));
    }
    the_master->SendMsg(GetFd(), responds[5], sizeof(responds[5]));
    step = step_authorization_authorized;
}

void Client::CreateFile(const char *str)
{
    if (strstr(str, " ") || strstr(str, "/") || str[0] == '.')
        the_master->SendMsg(GetFd(), type_a_file_name_msg,
                            sizeof(type_a_file_name_msg));
    else
    {
        char *file_name = new char[strlen(str) + 2 +
                                   strlen(the_master->GetDirectoryPath())];
        sprintf(file_name, "%s/%s", the_master->GetDirectoryPath(), str);
        int fd;
        if ((fd = open(file_name, O_RDONLY, 0666)) == -1)
        {
            file_fd = open(file_name, O_CREAT | O_WRONLY, 0666);
            file = new FileStructure;
            strcpy(file->author_nickname, name);
            strcpy(file->file_name, str);
            file->perms = 0;
            step = step_set_perms;
            the_master->SendMsg(GetFd(), set_perms_msg, sizeof(set_perms_msg));
        }
        else
        {
            the_master->SendMsg(GetFd(), type_another_file_name_msg,
                                sizeof(type_another_file_name_msg));
            close(fd);
        }
        delete[] file_name;
    }
}

void Client::SetPermissions(const char *str)
{
    if (sscanf(str, "%4o", &(file->perms)) == EOF)
        the_master->SendMsg(GetFd(), write_complete_perm_msg,
                            sizeof(write_complete_perm_msg));
    else
    {
        the_master->file_db_handler->Add(file);
        delete file;
        char *to_msg = new char[strlen(responds[5]) +
                                strlen(responds[2]) + 1];
        sprintf(to_msg, "%s%s", responds[2], responds[5]);
        the_master->SendMsg(GetFd(), to_msg, strlen(to_msg) + 1);
        delete[] to_msg;
        step = step_is_put;
    }
}

void Client::GetFile(const char *str)
{
    file = new FileStructure;
    if (-1 == the_master->file_db_handler->GetByName(str, file))
    {
        step = prev_step;
        char *to_msg = new char[strlen(responds[5]) + 1];
        sprintf(to_msg, "%s", responds[5]);
        the_master->SendMsg(GetFd(), to_msg, strlen(to_msg) + 1);
        delete[] to_msg;
    }
    else
    {
        char *file_name = new char[strlen(str) + 2 +
                                   strlen(the_master->GetDirectoryPath())];
        sprintf(file_name, "%s/%s", the_master->GetDirectoryPath(), str);
        file_fd = open(file_name, O_RDONLY, 0666);
        delete[] file_name;

        if (prev_step == step_authorization_authorized)
        {
            if (!strcmp(file->author_nickname, name) ||
                (file->perms & 010) == 010)
            {
                step = step_is_get;
                the_master->SendMsg(GetFd(), responds[3],
                                    strlen(responds[3]) + 1);
            }
            else
            {
                step = prev_step;
                the_master->SendMsg(GetFd(), responds[5],
                                    strlen(responds[5]) + 1);
            }
        }
        else if ((file->perms & 001) == 001)
        {
            step = step_is_get;
            the_master->SendMsg(GetFd(), responds[3], strlen(responds[3]) + 1);
        }
        else
        {
            step = prev_step;
            the_master->SendMsg(GetFd(), responds[5], strlen(responds[5]) + 1);
        }
    }
    delete file;
}

int Client::ClientHandle(char *str)
{
    char q_flag = 0;
    switch (step)
    {
    case step_authorization_register:
        RegisterHandle(str);
        break;
    case step_authorization_uninitialized:
        StartAuthorization(str);
        break;
    case step_authorization_unauthorized_login:
        Login(str);
        break;
    case step_authorization_unauthorized_password:
        CheckPasswordCredentials(str);
        break;
    case step_authorization_authorized:
    case step_authorization_noauthorized:
        CommandHandle(str, &q_flag);
        if (q_flag == -1)
            return -1;
        break;
    case step_authorization_change_password:
        ChangePassword(str);
        break;
    case step_want_put:
        CreateFile(str);
        break;
    case step_set_perms:
        SetPermissions(str);
        break;
    case step_want_get:
        GetFile(str);
        break;
    case step_is_get:
        SetWrite(true);
        break;
    case step_want_remove:
        RemoveFile(str);
        break;
    default:
        break;
    }
    return 0;
}