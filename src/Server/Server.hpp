#ifndef SERVER_HPP_SENTRY
#define SERVER_HPP_SENTRY

// #define QLEN 15
#ifndef BUFFERSIZE
#define BUFFERSIZE 256
#endif

enum steps
{
    step_authorization_register, /* registration process */

    step_authorization_uninitialized,
    step_authorization_noauthorized, /* Connected without an account */

    step_authorization_unauthorized_login,
    step_authorization_unauthorized_password,
    step_authorization_authorized, /* Connected with an account */

    step_authorization_change_password,

    step_want_put,
    step_set_perms,
    step_is_put,

    step_want_remove,

    step_want_get,
    step_is_get
};

enum registration_step
{
    step_registration_no,
    step_registration_login,
    step_registration_password
};

#include "../DbHandler/DbHandler.hpp"
#include "../UserDbHandler/UserDbHandler.hpp"
#include "../FileDbHandler/FileDbHandler.hpp"

enum
{
    current_max_count = 16
};

class FdHandler
{
    int fd;
    bool want_write;
    bool want_read;

public:
    FdHandler(int _fd, bool _want_write = false, bool _want_read = true)
        : fd(_fd), want_write(_want_write), want_read(_want_read) {}
    virtual ~FdHandler() { close(fd); }
    int GetFd() { return fd; }
    bool WantWrite() { return want_write; }
    bool WantRead() { return want_read; }
    void SetWrite(bool new_write) { want_write = new_write; }
    void SetRead(bool new_read) { want_read = new_read; }

    virtual void Handle(bool r, bool w) = 0;
};

class EventSelector
{
    FdHandler **fd_array;
    int fd_array_len;
    int max_fd;

public:
    EventSelector() : fd_array(0), fd_array_len(0), max_fd(-1) {}
    ~EventSelector();

    void Add(FdHandler *fh);
    void Remove(FdHandler *fh);

    void Run();
};

////////////////////////////////////////

class Client;

class Server : FdHandler
{
    friend class Client;

    EventSelector *the_selector;
    DbHandler<FileStructure> *file_db_handler;
    DbHandler<UserStructure> *user_db_handler;
    String directory_path;

    struct item
    {
        Client *cl;
        item *next;
    };
    item *first;

    Server(int fd, EventSelector *_the_selector,
           DbHandler<FileStructure> *_file_db_handler,
           DbHandler<UserStructure> *_user_db_handler,
           const char *_directory_path);

public:
    virtual ~Server();
    static Server *Start(int port, EventSelector *_the_selector,
                         DbHandler<FileStructure> *_file_db_handler,
                         DbHandler<UserStructure> *_user_db_handler,
                         const char *_directory_path);

    void RemoveClient(Client *cl);
    virtual void Handle(bool r, bool w);
    const char *GetDirectoryPath() { return directory_path; }
    void SendMsg(int fd, const char *msg, int len);
};

class Client : FdHandler
{
    friend class Server;

    Server *the_master;
    UserStructure *user;
    char *name;
    steps step;
    steps prev_step;

    int file_fd;

    char buf[BUFFERSIZE];
    int buf_used;

    registration_step reg_step;

    FileStructure *file;

public:
    Client(int _fd, Server *_the_master);
    virtual ~Client();
    virtual void Handle(bool r, bool w);

private:
    int ClientHandle(char *str);
    void RegisterHandle(const char *str);
    void RemoveFile(const char *str);
    void StartAuthorization(const char *str);
    void Login(const char *str);
    void CheckPasswordCredentials(const char *str);
    void CommandHandle(const char *str, char *q_flag);
    void ChangePassword(const char *str);
    void CreateFile(const char *str);
    void SetPermissions(const char *str);
    void GetFile(const char *str);
};

#endif
