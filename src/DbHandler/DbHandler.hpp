#ifndef DBHANDLER_SENTRY
#define DBHANDLER_SENTRY

#include "../String/String.hpp"

enum
{
    writing_format_len = 584,
    buffer_size = 250,
    max_person_in_dynamic_array = 50
};

template <class T>
class DbHandler
{
    String file_path;
    String format;
    const int format_len;
    T temp;

    DbHandler(const char *_file_path, const char *_format,
              const int _format_len)
        : file_path(_file_path), format(_format), format_len(_format_len) {}

public:
    ~DbHandler() {}
    static DbHandler<T> *Make(const char *_file_path, const char *_format,
                              const int _format_len);

    void Add(T *object) {}
    int GetByName(const char *name, T *object_to = 0) { return 0; }
    int EditByName(T *object, const char *by_name) { return 0; }
    void RemoveByName(const char *by_name) {}
    const char *GetFilePath() { return file_path; }
};

struct Object
{
    virtual void Clear() = 0;
};

#endif