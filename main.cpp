#include "i_webui_wire.h"
#include "object_t.h"
#include "readlineinthread.h"
#include "misc.h"
#include "event_t.h"


#ifdef __APPLE__
#include "apple_utils.h"
#endif

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

#ifdef __linux
void webui_gtk_log_handler(const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer unused_data)
{
    const char *level = "Gtk-Unknown";
    if (log_level & GLogLevelFlags::G_LOG_LEVEL_CRITICAL) { level = "Gtk-Critical"; }
    else if (log_level & GLogLevelFlags::G_LOG_LEVEL_DEBUG) { level = "Gtk-Debug"; }
    else if (log_level & GLogLevelFlags::G_LOG_LEVEL_ERROR) { level = "Gtk-Error"; }
    else if (log_level & GLogLevelFlags::G_LOG_LEVEL_INFO) { level = "Gtk-Info"; }
    else if (log_level & GLogLevelFlags::G_LOG_LEVEL_MESSAGE) { level = "Gtk-Message"; }
    else if (log_level & GLogLevelFlags::G_LOG_LEVEL_WARNING) { level = "Gtk-Warning"; }
    else if (log_level & GLogLevelFlags::G_LOG_FLAG_FATAL) { level = "Gtk-Fatal"; }
    else if (log_level & GLogLevelFlags::G_LOG_FLAG_RECURSION) { level = "Gtk-Recursion"; }

    std::string kind = "Gtk";
    std::string msg = std::string(log_domain) + "-" + level + "-" + message;
    log(kind.c_str(), msg.c_str());
}
#endif


static void mainLoop(webwire_handle handle, bool &go_on)
{
    int buf_size = 1024;
    char *out_buf = static_cast<char *>(malloc(buf_size + 1));

    auto check_buf_size = [&buf_size, &out_buf](int len) {
        if (len > 0 && len > buf_size) {
            buf_size = len * 2;
            out_buf = static_cast<char *>(realloc(out_buf, buf_size + 1));
            if (out_buf == nullptr) {
                exit(2);
            }
            return true;
        }
        return false;
    };

    FILE *fh_log = fopen("/tmp/webui_wire.log", "wt");

    auto do_log = [fh_log](const char *kind, int len, const char *msg) {
        fprintf(fh_log, "%s-%08d:%s\n", kind, len, msg);
        fflush(fh_log);
    };

    while (go_on) {
        Event_t evt = _queue.dequeue();
        if (!evt.isNull()) {
            if (evt.is_a(id_log)) {
                std::string kind;
                std::string msg;
                evt >> kind;
                evt >> msg;
                trim(msg);
                int len = snprintf(out_buf, buf_size, "%s:%s", kind.c_str(), msg.c_str());
                if (check_buf_size(len)) {
                    snprintf(out_buf, buf_size, "%s:%s", kind.c_str(), msg.c_str());
                }
                fprintf(stderr, "%08d:%s\n", len, out_buf);
                fflush(stderr);
                do_log("stderr", len, out_buf);
            } else if (evt.is_a(id_evt)) {
                std::string event;
                evt >> event;
                trim(event);
                int len = snprintf(out_buf, buf_size, "%s:%s", "EVENT", event.c_str());
                if (check_buf_size(len)) {
                    snprintf(out_buf, buf_size, "%s:%s", "EVENT", event.c_str());
                }
                fprintf(stderr, "%08d:%s\n", len, out_buf);
                fflush(stderr);
                do_log("stderr", len, out_buf);
            } else if (evt.is_a(id_readline_have_line)) {
                std::string line;
                evt >> line;
                do_log("stdin ", line.length(), line.c_str());
//#ifdef __APPLE__
//                const char *result = webwire_command_apple(handle, line.c_str());
//#else
                const char *result = webwire_command(handle, line.c_str());
//#endif
                int len = snprintf(out_buf, buf_size, "%s", result);
                if (check_buf_size(len)) {
                    snprintf(out_buf, buf_size, "%s", result);
                }
                fprintf(stdout, "%08d:%s\n", len, result);
                fflush(stdout);
                do_log("stdout", len, out_buf);
                if (trim_copy(line) == "exit") {
                    go_on = false;
                }
            } else if (evt.is_a(id_readline_eof)) {
                int len = snprintf(out_buf, buf_size, "EVENT:readline:EOF");
                if (check_buf_size(len)) {
                    snprintf(out_buf, buf_size, "EVENT:readline:EOF");
                }
                fprintf(stderr, "%08d:%s\n", len, out_buf);
                fflush(stderr);
                go_on = false;
            } else if (evt.is_a(id_readline_error)) {
                std::string errmsg;
                int no;
                evt >> no;
                evt >> errmsg;
                int len = snprintf(out_buf, buf_size, "EVENT:readline error:%d:%s", no, errmsg.c_str());
                if (check_buf_size(len)) {
                    snprintf(out_buf, buf_size, "EVENT:readline error:%d:%s", no, errmsg.c_str());
                }
                fprintf(stderr, "%08d:%s\n", len, out_buf);
                fflush(stderr);
                do_log("stderr", len, out_buf);
                go_on = false;
            }
        }
#ifdef __linux
        else {  // Idle processing, process Gtk events.
            while (gtk_events_pending()) {
                gtk_main_iteration_do(0);
            }
        }
#endif
#ifdef __APPLE__
        else {
            process_events_apple();
        }
#endif
    }

    int len = snprintf(out_buf, buf_size, "EVENT:exiting");
    if (check_buf_size(len)) {
        snprintf(out_buf, buf_size, "EVENT:exiting");
    }
    fprintf(stderr, "%08d:%s\n", len, out_buf);
    fflush(stderr);

    fclose(fh_log);

    free(out_buf);

//#ifdef __APPLE__
//    stop_main_app_loop_apple();
//#endif
}

int main(int argc, char *argv[])
{
    // dup stdout and stderr and make sure they are reopend to a temporary file

#ifdef __linux
    static bool initialized = false;
    if (!initialized) {
        gtk_init(&argc, &argv);
        initialized = true;
    }

    // Gtk log handler
    g_log_set_default_handler(webui_gtk_log_handler, NULL);
#endif

    // create an event queue and wait for lines

#if defined(__linux) || defined(__APPLE__)
    _queue.setWait(1);
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

//#ifdef __APPLE__
//    std::thread ml([handle, &go_on]() {
//        mainLoop(handle, go_on);
//    });
//    run_main_app_loop_apple();
//    //webui_wait();
//#else
    mainLoop(handle, go_on);
//#endif

    msg_thread.join();
    webwire_destroy(handle);
    delete reader;

    return 0;
}
