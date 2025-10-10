#ifndef WEBUIWINDOW_H
#define WEBUIWINDOW_H

#include "object_t.h"
#include "misc.h"
#include "url.h"
#include "fileinfo_t.h"
#include <string>
#include <functional>
extern "C" {
#include <webui.h>
}

#ifdef __linux
#include <gtk/gtk.h>
#endif

class WebWireHandler;
class WebWireProfile;

typedef enum {
    hidden = 0x000,
    shown = 0x100,
    minimized = 0x001,
    maximized = 0x002,
    normal = 0x004
} WebUiWindow_ShowState;

class WebUIWindow : public Object_t
{
private:
    WebWireProfile *_ww_profile;
    WebWireHandler *_handler;
    size_t          _webui_win;
    int             _win;
    std::string     _profile;
    std::string     _root_dir;
    std::string     _base_url;
    std::string     _standard_msg;
    bool            _closing;
    bool            _in_set_html_or_url;
    int             _current_handle;
    int             _handle_counter;
    bool            _use_browser;
    int             _webui_port;
    WebUIWindow    *_parent_win;
    bool            _disconnected;
#ifdef _WINDOWS
    HWND            _win_handle;
#else
    #ifdef __linux
        GtkWindow   *_win_handle;
    #else
        error Not implemented yet
    #endif
#endif

private:
    static const char *_default_favicon;

private:
    int newHandle();

public:
    std::string baseUrl();
    std::string rootFolder();
    std::string standardMessage();

public:
    size_t webuiWin();
    int webuiPort();

public:
    void setWindowIcon(const std::string &icn_file);
    void setWindowTitle(const std::string &title);
    void setClosing(bool y);
    int show(const std::string &msg_or_url);
    int showIfNotShown();
    void close();
    void move(int x, int y);
    void resize(int w, int h);
    void useBrowser(bool y);
    bool disconnected();
    void setShowState(WebUiWindow_ShowState st);
    int showState();

public:
#ifdef _WINDOWS
    HWND nativeHandle();
#else
#ifdef __linux
    GtkWindow *nativeHandle();
#else
#error "Not implemented"
#endif
#endif

public:
    void webuiEvent(webui_event_t *e);
    void handleWireEvent(webui_event_t *e);
    void handleResizeEvent(webui_event_t *e);
    void handleMoveEvent(webui_event_t *e);
    bool canClose();
    const void *filesHandler(const char *file, int *length);

public:
    int setUrl(const upa::url &u);
    int  setHtml(std::string url);

public:
    WebUIWindow(WebWireHandler *h, int _win, const std::string &profile, bool _use_browser, WebUIWindow *parent_win, Object_t *parent = nullptr);
    ~WebUIWindow();
};

#endif // WEBUIWINDOW_H
