#include "i_webui_wire.h"
#include "object_t.h"
#include "readlineinthread.h"
#include "misc.h"
#include "event_t.h"

#ifdef __linux
#include <gtk/gtk.h>
#endif

#define id_log  "log-message"
#define evt_log Event_t(id_log, nullptr)

#define id_evt  "event"
#define evt_event Event_t(id_evt, nullptr)

static EventQueue_t _queue;

class StdWebWire : public Object_t
{
public:
    StdWebWire(Object_t *parent = nullptr)
        : Object_t(parent)
    {
    }

    // Object_t interface
public:
    void event(Event_t msg) {
        _queue.enqueue(msg);
    }
};


static void log(const char *kind, const char *msg)
{
    std::string k(kind);
    std::string m(msg);

    _queue.enqueue(evt_log << k << m);
}

static void evt(const char *evt)
{
    std::string e(evt);
    _queue.enqueue(evt_event << e);
}

int main(int argc, char *argv[])
{
    // create an event queue and wait for lines

#ifdef __linux
    _queue.setWait(2);
#else
    _queue.setWait(500);
#endif

    webwire_handle handle = webwire_new();
    enum_handle_status s = webwire_status(handle);
    if (s != webwire_valid) {
        fprintf(stderr, "Cannot start WebWire CLI, status = %s\n", webwire_status_string(s));
        exit(1);
    }

    StdWebWire std_ww;
    ReadLineInThread *reader = new ReadLineInThread();
    connect(reader, id_readline_eof, &std_ww);
    connect(reader, id_readline_error, &std_ww);
    connect(reader, id_readline_have_line, &std_ww);

    bool go_on = true;

    std::thread msg_thread([handle, &go_on]() {
        while(go_on) {
            char *e_evt, *e_kind, *e_msg;
            enum_get_result gr = webwire_get(handle, &e_evt, &e_kind, &e_msg);
            //fprintf(stderr, "get: %d, status %s\n", gr, webwire_status_string(webwire_status(handle)));
            switch(gr) {
            case webwire_null: // do nothing
                break;
            case webwire_event:
                evt(e_evt);
                break;
            case webwire_log:
                log(e_kind, e_msg);
                break;
            case webwire_invalid_handle:
                go_on = false;
                break;
            }
        }
    });
    setThreadName(&msg_thread, "webui-wire-msg-thread");

    while (go_on) {
        Event_t evt = _queue.dequeue();
        if (!evt.isNull()) {
            if (evt.is_a(id_log)) {
                std::string kind;
                std::string msg;
                evt >> kind;
                evt >> msg;
                trim(msg);
                fprintf(stdout, "%s:%s\n", kind.c_str(), msg.c_str());
                fflush(stdout);
            } else if (evt.is_a(id_evt)) {
                std::string event;
                evt >> event;
                trim(event);
                fprintf(stdout, "%s:%s\n", "EVENT", event.c_str());
                fflush(stdout);
            } else if (evt.is_a(id_readline_have_line)) {
                std::string line;
                evt >> line;
                const char *result = webwire_command(handle, line.c_str());
                fprintf(stdout, "%s\n", result);
                if (trim_copy(line) == "exit") {
                    go_on = false;
                }
            } else if (evt.is_a(id_readline_eof)) {
                fprintf(stdout, "readline: eof\n");
                go_on = false;
            } else if (evt.is_a(id_readline_error)) {
                std::string errmsg;
                int no;
                evt >> no;
                evt >> errmsg;
                fprintf(stdout, "readline error: %d, %s", no, errmsg.c_str());
                go_on = false;
            }
        }
#ifdef __linux
        else {  // Idle processing, process Gtk events.
            static bool initialized = false;
            if (!initialized) {
                gtk_init(&argc, &argv);
                initialized = true;
            }
            while (gtk_events_pending()) {
                gtk_main_iteration_do(0);
            }
        }
#endif
    }

    fprintf(stderr, "here1\n");
    msg_thread.join();
    fprintf(stderr, "here2\n");
    webwire_destroy(handle);
    fprintf(stderr, "here3\n");
    delete reader;

    fprintf(stderr, "ok - klaar...>");

    return 0;
}
