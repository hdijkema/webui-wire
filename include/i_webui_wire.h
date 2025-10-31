#ifndef I_WEBUI_WIRE_H
#define I_WEBUI_WIRE_H

#include "webui_wire_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    webwire_null = 0,
    webwire_event,
    webwire_log,
    webwire_invalid_handle = 256,
} enum_get_result;

typedef enum {
    webwire_valid = 1,
    webwire_invalid_destroyed,
    webwire_invalid_needs_destroying,
    webwire_invalid_null_handle,
    webwire_invalid_existing_handle_destroy_this_one,
    webwire_invalid_unexpected
} enum_handle_status;

typedef void* webwire_handle;

WEBUI_WIRE_EXPORT webwire_handle webwire_new();
WEBUI_WIRE_EXPORT int webwire_handle_id(webwire_handle h);
WEBUI_WIRE_EXPORT void webwire_set_signaller(webwire_handle h, void (*signal_item)(int queue_count));
WEBUI_WIRE_EXPORT bool webwire_set_handlers(webwire_handle h, void (*evt_handler)(const char *event),
                                                              void (*log_handler)(const char *kind, const char *msg));

WEBUI_WIRE_EXPORT bool webwire_process_gui(webwire_handle h);

WEBUI_WIRE_EXPORT webwire_handle webwire_current();
WEBUI_WIRE_EXPORT void webwire_destroy(webwire_handle h);
WEBUI_WIRE_EXPORT const char *webwire_command(webwire_handle h, const char *command);
WEBUI_WIRE_EXPORT unsigned int webwire_items(webwire_handle handle);
WEBUI_WIRE_EXPORT enum_get_result webwire_get(webwire_handle handle, char **evt, char **log_kind, char **log_msg);
WEBUI_WIRE_EXPORT enum_handle_status webwire_status(webwire_handle h);
WEBUI_WIRE_EXPORT const char *webwire_status_string(enum_handle_status s);

#ifdef __cplusplus
}
#endif

#endif // I_WEBUI_WIRE_H
