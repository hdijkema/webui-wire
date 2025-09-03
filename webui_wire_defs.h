#ifndef WEBUI_WIRE_DEFS_H
#define WEBUI_WIRE_DEFS_H


#ifdef _WINDOWS
#ifdef LIBWEBUI_WIRE_BUILDING
#define WEBUI_WIRE_EXPORT __declspec(dllexport)
#else
#define WEBUI_WIRE_EXPORT __declspec(dllimport)
#endif
#endif

#ifdef _LINUX
#ifdef LIBWEBUI_WIRE_BUILDING
#define WEBUI_WIRE_EXPORT __attribute__((visibility("default")))
#else
#define WEBUI_WIRE_EXPORT __attribute__((visibility("default")))
#endif
#endif

#ifndef I_WEBUI_WIRE_EXPORT
#define I_WEBUI_WIRE_EXPORT
#endif


#endif // WEBUI_WIRE_DEFS_H
