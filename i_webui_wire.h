#ifndef I_WEBUI_WIRE_H
#define I_WEBUI_WIRE_H

#include "webui_wire_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    WEBWIRE_NULL = 0,
    WEBWIRE_LOG,
    WEBWIRE_EVENT
} webwire_evt_kind_t;

WEBUI_WIRE_EXPORT bool webwire_start(int queue_wait_ms, char **msg);
WEBUI_WIRE_EXPORT void webwire_stop();
WEBUI_WIRE_EXPORT void webwire_wait_for_exit();
WEBUI_WIRE_EXPORT bool webwire_command(const char *cmd, char **result);
WEBUI_WIRE_EXPORT bool webwire_poll_event(char **event);
WEBUI_WIRE_EXPORT bool webwire_poll_log(char **kind, char **msg);
WEBUI_WIRE_EXPORT webwire_evt_kind_t webwire_poll_event_or_log(char **kind, char **msg, char **event);
WEBUI_WIRE_EXPORT webwire_evt_kind_t webwire_get_event_or_log(char **kind, char **msg, char **event);
WEBUI_WIRE_EXPORT void webwire_free(char *s);

#ifdef __cplusplus
}
#endif

#endif // I_WEBUI_WIRE_H
