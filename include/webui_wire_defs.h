#ifndef WEBUI_WIRE_DEFS_H
#define WEBUI_WIRE_DEFS_H

#define WEBUI_WIRE_STATIC true

#ifdef WEBUI_WIRE_STATIC
#define WEBUI_WIRE_EXPORT
#ifndef I_WEBUI_WIRE_EXPORT
#define I_WEBUI_WIRE_EXPORT
#endif
#else

#ifdef _WINDOWS
#ifdef LIBWEBUI_WIRE_BUILDING
#define WEBUI_WIRE_EXPORT __declspec(dllexport)
#else
#define WEBUI_WIRE_EXPORT __declspec(dllimport)
#endif
#endif

#ifdef __linux
#ifdef LIBWEBUI_WIRE_BUILDING
#define WEBUI_WIRE_EXPORT __attribute__((visibility("default")))
#else
#define WEBUI_WIRE_EXPORT __attribute__((visibility("default")))
#endif
#endif

#ifndef I_WEBUI_WIRE_EXPORT
#define I_WEBUI_WIRE_EXPORT
#endif
#endif


#endif // WEBUI_WIRE_DEFS_H
