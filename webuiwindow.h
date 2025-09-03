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

class WebWireHandler;
class WebWireProfile;

class WebUIWindow : public Object_t
{
private:
    WebWireProfile *_ww_profile;
    WebWireHandler *_handler;
    size_t          _webui_win;
    int             _win;
    std::string     _profile;
    std::string     _root_dir;
    std::string     _standard_msg;
    bool            _closing;
    bool            _use_browser;
    int             _webui_port;
    WebUIWindow    *_parent_win;
#ifdef _WINDOWS
    HWND            _win_handle;
#else
    error Not implemented yet
#endif

public:
    std::string baseUrl();
    std::string rootFolder();

public:
    size_t webuiWin();
    int webuiPort();

public:
    void setWindowIcon(const FileInfo_t &icn_file);
    void setWindowTitle(const std::string &title);
    void setClosing(bool y);
    void show(const std::string &msg_or_url);
    void showIfNotShown();
    void close();
    void move(int x, int y);
    void resize(int w, int h);
    void useBrowser(bool y);

public:
#ifdef _WINDOWS
    HWND nativeHandle();
#endif

public:
    void webuiEvent(webui_event_t *e);
    void handleWireEvent(webui_event_t *e);
    void handleResizeEvent(webui_event_t *e);
    void handleMoveEvent(webui_event_t *e);
    bool canClose();

public:
    void setUrl(const upa::url &u);
    void setHtml(std::string url);

public:
    WebUIWindow(WebWireHandler *h, int _win, const std::string &profile, WebUIWindow *parent_win, Object_t *parent = nullptr);
    ~WebUIWindow();
};

#endif // WEBUIWINDOW_H
