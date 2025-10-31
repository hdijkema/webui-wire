#ifndef READLINEINTHREAD_H
#define READLINEINTHREAD_H

#include "webui_wire_defs.h"
#include "object_t.h"
#include "event_t.h"

#define id_readline_have_line   "readline-have-line"
#define evt_readline_have_line  Event_t(id_readline_have_line, this)

#define id_readline_eof         "readline-eof"
#define evt_readline_eof        Event_t(id_readline_eof, this)

#define id_readline_error       "readline-error"
#define evt_readline_error      Event_t(id_readline_error, this)

class WEBUI_WIRE_EXPORT ReadLineInThread : public Object_t
{
private:
    char                   *_buffer;
    int                     _buffer_len;
    bool                    _go_on;
    std::thread::id         _my_id;
    std::thread            *_thread;
    int                     _wait_ms;
    FILE                   *_in;

public:
    explicit ReadLineInThread(FILE *in);
    ~ReadLineInThread();

public:
    void quit();

private:
    void haveALine(std::string  l);
    void haveEof();
    void haveError(int error_number);

public:
    void run();
};

#endif // READLINEINTHREAD_H
