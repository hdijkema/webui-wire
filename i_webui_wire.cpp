#include "i_webui_wire.h"
#include "webwirehandler.h"
#include "application_t.h"

extern "C" {
#include <webui.h>
}

#define VALID_HANDLE 0x3823743293821426LL

typedef struct
{
    long long       valid_handle;
    int             size_kind;
    char           *log_kind;
    int             size_msg;
    char           *log_msg;
    int             size_event;
    char           *event;
    int             size_command_result;
    char           *command_result;
    EventQueue_t   *queue;
    std::thread    *exec_thread;
    WebWireHandler *handler;
    Application_t  *app;
    bool            quit_by_exit_command;
}  _webwire_handle;

#define id_ww_log   "i-webwire-log"
#define id_ww_event "i-webwire-evt"

static _webwire_handle *_current = nullptr;

static enum_handle_status _webwire_valid_handle(webwire_handle handle, const char *func, int line, bool errmsg)
{
    if (handle == NULL) {
        if (errmsg) {
            if (func != nullptr) {
                fprintf(stderr, "%s[%d]:", func, line);
            }
            fprintf(stderr, "NULL handle\n");
            fflush(stderr);
        }
        return webwire_invalid_null_handle;
    }

    _webwire_handle *h = static_cast<_webwire_handle *>(handle);
    if (h->valid_handle != VALID_HANDLE) {
        if (errmsg) {
            if (func != nullptr) {
                fprintf(stderr, "%s[%d]:", func, line);
            }
            fprintf(stderr, "Invalid handle magic number\n");
            fflush(stderr);
        }
        return webwire_invalid_destroyed;
    }

    if (h->app == nullptr) {
        return webwire_invalid_existing_handle_destroy_this_one;
    }

    if (h->quit_by_exit_command) {
        return webwire_invalid_needs_destroying;
    }

    return webwire_valid;
}

static void webui_wire_logger(webui_log_level_t level, const char *m, void *user_data)
{
    _webwire_handle *h = static_cast<_webwire_handle *>(user_data);

    if (_webwire_valid_handle(h, __FUNCTION__, __LINE__, true) == webwire_valid) {
        WebWireHandler *handler = h->handler;

        std::string msg = m;
        trim(msg);

        switch(level) {
        case log_debug:   handler->debug("webui-debug:" + msg);
            break;
        case log_info:    handler->message("webui-info:" + msg);
            break;
        case log_warning: handler->message("webui-warning:" + msg);
            break;
        case log_error:   handler->error("webui-error:" + msg);
            break;
        case log_fatal:   handler->error("webui-fatal:" + msg);
            break;
        }
    }
}

static void _log_handler(const char *kind, const char *msg, void *user_data)
{
    _webwire_handle *h = static_cast<_webwire_handle *>(user_data);

    if (_webwire_valid_handle(h, __FUNCTION__, __LINE__, true) == webwire_valid) {
        h->queue->enqueue(Event_t(id_ww_log, nullptr) << std::string(kind) << std::string(msg));
    }
}

static void _event_handler(const char *evt, void *user_data)
{
    _webwire_handle *h = static_cast<_webwire_handle *>(user_data);

    if (_webwire_valid_handle(h, __FUNCTION__, __LINE__, true) == webwire_valid) {
        h->queue->enqueue(Event_t(id_ww_event, nullptr) << std::string(evt));
    }
}

webwire_handle webwire_new()
{
    _webwire_handle *h = static_cast<_webwire_handle *>(malloc(sizeof(_webwire_handle)));

    h->valid_handle = VALID_HANDLE;
    h->size_kind = 50;
    h->log_kind = static_cast<char *>(malloc(h->size_kind));
    h->size_msg = 256;
    h->log_msg = static_cast<char *>(malloc(h->size_msg));
    h->size_event = 10240;
    h->event = static_cast<char *>(malloc(h->size_event));
    h->size_command_result = 10240;
    h->command_result = static_cast<char *>(malloc(h->size_command_result));

    h->quit_by_exit_command = false;

    if (Application_t::current() != nullptr) {
        // Creating a new handle while another is running is invalid.
        h->app =nullptr;
        h->queue = nullptr;
        h->handler = nullptr;
        h->exec_thread = nullptr;
    } else {
        h->queue = new EventQueue_t();

        h->app = new Application_t();
        h->handler = new WebWireHandler(h->app, 0, nullptr, _log_handler, _event_handler, h);
        h->app->setHandler(h->handler);

        // Only then we can add the custom logger.
        //auto ww_log_f = std::mem_fn(&_webwire_handle::webui_wire_logger);
        //auto cb = std::bind(&_webwire_handle::webui_wire_logger, h);
        webui_set_custom_logger(webui_wire_logger, static_cast<void *>(h));
        webui_set_config(webui_config::use_cookies, true);

        h->exec_thread = new std::thread([h]() {
            h->app->exec();
            h->quit_by_exit_command = true;
        });
        setThreadName(h->exec_thread, "i-webui-wire-thread");

        _log_handler("LOG", "WebWire Handler Started", h);
    }

    _current = h;

    return static_cast<webwire_handle>(h);
}


void webwire_destroy(webwire_handle handle)
{
    enum_handle_status s = _webwire_valid_handle(handle, __FUNCTION__, __LINE__, true);

    if (s == webwire_valid || s == webwire_invalid_needs_destroying || s == webwire_invalid_existing_handle_destroy_this_one) {
        _webwire_handle *h = static_cast<_webwire_handle *>(handle);

        if (s != webwire_invalid_existing_handle_destroy_this_one) {
            if (!h->quit_by_exit_command) {
                h->handler->doQuit();   // Quitting handler
                h->app->quit();         // Quitting app
                h->exec_thread->join(); // Joining thread
                delete h->exec_thread;  // Deleting thread
            }

            delete h->handler;
            delete h->app;
            delete h->queue;
        }

        free(h->log_kind);
        free(h->log_msg);
        free(h->event);
        free(h->command_result);

        h->app = nullptr;
        h->handler = nullptr;
        h->queue = nullptr;
        h->exec_thread = nullptr;
        h->log_kind = nullptr;
        h->log_msg = nullptr;
        h->event = nullptr;
        h->command_result = nullptr;
        h->size_kind = 0;
        h->size_msg = 0;
        h->size_event = 0;
        h->size_command_result = 0;
        h->valid_handle = -1;

        free(h);

        _current = nullptr;
    }
}

enum_handle_status webwire_status(webwire_handle handle)
{
    return _webwire_valid_handle(handle, __FUNCTION__, __LINE__, false);
}

const char *webwire_command(webwire_handle handle, const char *command)
{
    if (_webwire_valid_handle(handle, __FUNCTION__, __LINE__, true) == webwire_valid) {
        _webwire_handle *h = static_cast<_webwire_handle *>(handle);
        std::string ok_m;
        h->handler->processInput(command, &ok_m);

        size_t s = ok_m.size() + 1;
        if (h->size_command_result < s) {
            h->size_command_result = s + 256;
            h->command_result = static_cast<char *>(realloc(h->command_result, h->size_command_result));
        }
        memcpy(h->command_result, ok_m.c_str(), s + 1);
        return h->command_result;
    } else {
        return "NOK::INVALID HANDLE";
    }
}

unsigned int webwire_items(webwire_handle handle)
{
    if (_webwire_valid_handle(handle, __FUNCTION__, __LINE__, true) == webwire_valid) {
        _webwire_handle *h = static_cast<_webwire_handle *>(handle);

        return h->queue->count();
    }

    return 0;
}

enum_get_result webwire_get(webwire_handle handle, char **evt, char **log_kind, char **log_msg)
{
    if (_webwire_valid_handle(handle, __FUNCTION__, __LINE__, true) == webwire_valid) {
        _webwire_handle *h = static_cast<_webwire_handle *>(handle);
        Event_t e = h->queue->dequeue();
        if (e.isNull()) {
            *evt = nullptr;
            *log_kind = nullptr;
            *log_msg = nullptr;
            return webwire_null;
        } else if (e.is_a(id_ww_event)) {
            std::string s;
            e >> s;
            const char *m = s.c_str();
            int sz = strlen(m) + 1;
            if (h->size_event < sz) {
                h->size_event = sz + 256;
                h->event = static_cast<char *>(realloc(h->event, h->size_event));
            }
            memcpy(h->event, m, sz);
            *evt = h->event;
            *log_kind = nullptr;
            *log_msg = nullptr;
            return webwire_event;
        } else if (e.is_a(id_ww_log)) {
            std::string kind;
            std::string msg;
            e >> kind;
            e >> msg;
            const char *k = kind.c_str();
            int sk = strlen(k) + 1;
            if (h->size_kind < sk) {
                h->size_kind = sk + 256;
                h->log_kind = static_cast<char *>(realloc(h->log_kind, h->size_kind));
            }
            memcpy(h->log_kind, k, sk);
            const char *m = msg.c_str();
            int sm = strlen(m) + 1;
            if (h->size_msg < sm) {
                h->size_msg = sm + 256;
                h->log_msg = static_cast<char *>(realloc(h->log_msg, h->size_msg));
            }
            memcpy(h->log_msg, m , sm);

            *evt = nullptr;
            *log_kind = h->log_kind;
            *log_msg = h->log_msg;
            return webwire_log;
        }
    }

    *evt = nullptr;
    *log_kind = nullptr;
    *log_msg = nullptr;
    return webwire_invalid_handle;
}


const char *webwire_status_string(enum_handle_status s)
{
    switch(s) {
        case webwire_valid: return "valid webwire handle";
            break;
        case webwire_invalid_destroyed: return "probably destroyed handle";
            break;
        case webwire_invalid_needs_destroying: return "handle invalidated by exit, still needs destroying";
            break;
        case webwire_invalid_null_handle: return "invalid handle, null pointer";
            break;
        case webwire_invalid_existing_handle_destroy_this_one: return "existing handle with working thread, cannot create a second, old one destroy first";
            break;
        default: return "invalid handle, unknown cause";
    }
}

webwire_handle webwire_current()
{
    return static_cast<webwire_handle>(_current);
}
