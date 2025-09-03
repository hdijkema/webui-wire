#include "i_webui_wire.h"
#include "webwirehandler.h"
#include "application_t.h"

extern "C" {
#include <webui.h>
}

static Application_t  *_app = nullptr;
static WebWireHandler *_handler = nullptr;
static std::thread    *_exec_thread = nullptr;

static EventQueue_t    _event_queue;

static void (*_log_handler)(const char *log_kind, const char *msg) = nullptr;
static void (*_evt_handler)(const char *msg) = nullptr;

#define id_ww_log   "i-webwire-log"
#define id_ww_event "i-webwire-evt"

// Webui Logging

void webui_wire_logger(webui_log_level_t level, const char *m)
{
    WebWireHandler *handler = Application_t::current()->handler();

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

// This is executed in the Application exec thread.
static void log_handler(const char *log_kind, const char *msg)
{
    std::string lk = log_kind;
    std::string m = msg;
    _event_queue.enqueue(Event_t(id_ww_log, nullptr) << lk << m);
}

// This is executed in the Application exec thread.
static void evt_handler(const char *msg)
{
    std::string m = msg;
    _event_queue.enqueue(Event_t(id_ww_event, nullptr) << m);
}

static inline void strOut(char **buf, const std::string &s)
{
    int sz = s.size() + 1;
    char *b = static_cast<char *>(malloc(sz));
    memcpy(b, s.c_str(), sz);
    *buf = b;
}

///////////// i_webwire C interface

bool webwire_start(int queue_wait_ms, char **msg)
{
    if (_handler == nullptr) {

        // First initialize the application and WebWireHandler
        _app = new Application_t();
        _handler = new WebWireHandler(_app, 0, nullptr, log_handler, evt_handler);
        _app->setHandler(_handler);

        // Only then we can add the custom logger.
        webui_set_custom_logger(webui_wire_logger);
        webui_set_config(webui_config::use_cookies, true);

        _exec_thread = new std::thread([]() { _app->exec(); });
        _log_handler = log_handler;
        _evt_handler = evt_handler;

        strOut(msg, "WebWire Handler Started");
        return true;
    }

    _event_queue.setWait(queue_wait_ms);

    strOut(msg, "WebWire Handler Already Started");
    return false;
}

void webwire_stop()
{
    _app->quit();
}

void webwire_wait_for_exit()
{
    _exec_thread->join();
    delete _exec_thread;
}

bool webwire_poll_event(char **event)
{
    if (_event_queue.empty()) {
        *event = nullptr;
        return false;
    }

    Event_t e = _event_queue.dequeue_if(id_ww_event);
    if (EventQueue_t::isNull(e)) {
        *event = nullptr;
        return false;
    }

    std::string m;
    e >> m;
    strOut(event, m);
    return true;
}

bool webwire_poll_log(char **kind, char **msg)
{
    if (_event_queue.empty()) {
        *kind = nullptr;
        *msg = nullptr;

        return false;
    }

    Event_t e = _event_queue.dequeue_if(id_ww_log);
    if (EventQueue_t::isNull(e)) {
        *kind = nullptr;
        *msg = nullptr;
        return false;
    }

    std::string k;
    e >> k;
    strOut(kind, k);

    std::string m;
    e >> m;
    strOut(msg, m);

    return true;
}

bool webwire_command(const char *command, char **result)
{
    std::string ok_m;
    _handler->processInput(command, &ok_m);

    strOut(result, ok_m);

    if (ok_m.rfind("OK:", 0) == 0) {
        return true;
    } else {
        return false;
    }
}


webwire_evt_kind_t webwire_poll_event_or_log(char **kind, char **msg, char **event)
{
    if (_event_queue.empty()) {
        *kind = nullptr;
        *msg = nullptr;
        *event = nullptr;
        return WEBWIRE_NULL;
    }

    return webwire_get_event_or_log(kind, msg, event);
}

webwire_evt_kind_t webwire_get_event_or_log(char **kind, char **msg, char **event)
{
    Event_t e = _event_queue.dequeue();

    if (EventQueue_t::isNull(e)) {
        *kind = nullptr;
        *msg = nullptr;
        *event = nullptr;
        return WEBWIRE_NULL;
    }

    if (e.is_a(id_ww_log)) {
        *event = nullptr;

        std::string k;
        e >> k;
        strOut(kind, k);

        std::string m;
        e >> m;
        strOut(msg, m);

        return WEBWIRE_LOG;
    }

    if (e.is_a(id_ww_event)) {
        *kind = nullptr;
        *msg = nullptr;

        std::string evt;
        e >> evt;
        strOut(event, evt);

        return WEBWIRE_EVENT;
    }

    // Unexpected, discard event e.

    *kind = nullptr;
    *msg = nullptr;
    *event = nullptr;
    return WEBWIRE_NULL;
}

void webwire_free(char *s)
{
    free(s);
}
