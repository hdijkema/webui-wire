#include "i_webui_wire.h"
#include "object_t.h"
#include "readlineinthread.h"
#include "misc.h"

class StdWebWire : public Object_t
{
private:
    bool &_go_on;
public:
    StdWebWire(bool &go_on, Object_t *parent = nullptr) : Object_t(parent), _go_on(go_on)
    {
    }

    // Object_t interface
public:
    void event(Event_t msg) {
        if (msg.is_a(id_readline_have_line)) {
            std::string line;
            msg >> line;
            char *result;
            bool ok = webwire_command(line.c_str(), &result);
            fprintf(stdout, "%s\n", result);
            if (trim_copy(line) == "exit") {
                _go_on = false;
            }
            free(result);
        } else if (msg.is_a(id_readline_eof)) {
            fprintf(stdout, "readline: eof\n");
            _go_on = false;
        } else if (msg.is_a(id_readline_error)) {
            std::string errmsg;
            int no;
            msg >> no;
            msg >> errmsg;
            fprintf(stdout, "readline error: %d, %s", no, errmsg.c_str());
            _go_on = false;
        }
    }
};


static void log(const char *kind, const char *msg)
{
    int l = strlen(msg);
    if (l > 0) {
        const char *format = (msg[l - 1] == '\n') ? "%s:%s" : "%s:%s\n";
        fprintf(stderr, format, kind, msg);
    }
}

static void log_and_free(char *kind, char *msg)
{
    log(kind, msg);
    free(kind);
    free(msg);
}

static void evt_and_free(char *evt)
{
    log("EVENT", evt);
    free(evt);
}

int main(int argc, char *argv[])
{
    // create an event queue and wait for lines

    char *msg;
    bool ok;
    ok = webwire_start(250, &msg);

    log_and_free(_strdup("webwire-start"), msg);

    if (ok) {
        char *kind, *msg, *event;
        bool go_on = true;

        StdWebWire std_ww(go_on);
        ReadLineInThread reader;
        connect(&reader, id_readline_eof, &std_ww);
        connect(&reader, id_readline_error, &std_ww);
        connect(&reader, id_readline_have_line, &std_ww);

        while (go_on) {
            webwire_evt_kind_t k;
            k = webwire_get_event_or_log(&kind, &msg, &event);
            switch(k) {
            case WEBWIRE_LOG: log_and_free(kind, msg);
                break;
            case WEBWIRE_EVENT: evt_and_free(event);
                break;
            case WEBWIRE_NULL: // do nothing
                break;
            }
        }
    } else {
        log("webwire-start", "Cannot start Web UI Wire!");
        return 1;
    }

    /*
    Application_t app;
    WebWireHandler handler(&app, argc, argv);

    app.setHandler(&handler);

    ReadLineInThread reader;
    connect(&reader, id_readline_have_line, &handler);
    connect(&reader, id_readline_eof, &handler);                // handle in event()
    connect(&reader, id_readline_error, &handler);              // handle in event()

    webui_set_custom_logger(webui_wire_logger);

    webui_set_config(webui_config::use_cookies, true);

    app.exec();
    */

    return 0;
}
