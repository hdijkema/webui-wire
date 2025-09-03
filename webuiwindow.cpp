#include "webuiwindow.h"
#include "webui_wire.h"
#include "webwirehandler.h"
#include "fileinfo_t.h"
#include "misc.h"
#include "execjs.h"

static wwhash<size_t, WebUIWindow *> _windows;

static WebUIWindow *get_webui_window(size_t webui_win)
{
    if (_windows.contains(webui_win)) {
        return _windows[webui_win];
    } else {
        WebWireHandler *h = Application_t::current()->handler();
        h->error(asprintf("Did not find webui %d", webui_win));
        return nullptr;
    }
}

static void web_ui_wire_handle_event(webui_event_t *e)
{
    WebWireHandler *h = Application_t::current()->handler();
    h->message(asprintf("web-ui-wire-handle-event: %d, %s", e->window, e->element));
    WebUIWindow *win = get_webui_window(e->window);
    if (win != nullptr) win->handleWireEvent(e);
}

static void web_ui_wire_handle_resize(webui_event_t *e)
{
    WebUIWindow *win = get_webui_window(e->window);
    if (win != nullptr) win->handleResizeEvent(e);
}

static void web_ui_wire_handle_move(webui_event_t *e)
{
    WebUIWindow *win = get_webui_window(e->window);
    if (win != nullptr) win->handleMoveEvent(e);
}

static bool web_ui_wire_on_close(size_t window)
{
    WebUIWindow *win = get_webui_window(window);
    if (win != nullptr) {
        return win->canClose();
    } else { // No window found, close always permitted
        return true;
    }
}

static void webui_event_handler(webui_event_t *e)
{
    size_t webui_win = e->window;
    if (_windows.contains(webui_win)) {
        WebUIWindow *win = _windows[webui_win];
        win->webuiEvent(e);
    } else {
        WebWireHandler *h = Application_t::current()->handler();
        h->error(asprintf("Did not find webui %d", webui_win));
    }
}

std::string WebUIWindow::baseUrl()
{
    //return asprintf("http://127.0.0.1:%d/", webui_get_port(_webui_win));
    //return asprintf("http://127.0.0.1:%d/", _handler->port());
    //return _handler->serverUrl() + _profile + "/";
    return _handler->serverUrl() + asprintf("%d/", _win);
}

size_t WebUIWindow::webuiWin()
{
    return _webui_win;
}

int WebUIWindow::webuiPort()
{
    return _webui_port;
}

std::string WebUIWindow::rootFolder()
{
    return _root_dir;
}

void WebUIWindow::setWindowIcon(const FileInfo_t &icn_file)
{

}

void WebUIWindow::setWindowTitle(const std::string &title)
{
    std::string code = "document.title = '" + ExecJs::esc_quote(title) + "';";
    _handler->execJs(_win, code, "set-title");
}

void WebUIWindow::setClosing(bool y)
{
    _closing = y;
}

void WebUIWindow::close()
{
#ifdef _WINDOWS
    _win_handle = static_cast<HWND>(webui_win32_get_hwnd(_webui_win));
    if (_parent_win != nullptr) {
        HWND _parent_handle = _parent_win->_win_handle;
        if (_parent_handle) {
            EnableWindow(_parent_handle, TRUE);
        }
    }
#endif
    webui_close(_webui_win);
}

bool WebUIWindow::canClose()
{
    if (_closing) { return true; }
    _handler->evt(asprintf("close-request:%d", _win));
    return false;
}

void WebUIWindow::move(int x, int y)
{
    webui_set_position(_webui_win, x, y);
}

void WebUIWindow::resize(int w, int h)
{
    webui_set_size(_webui_win, w, h);
}

/*
typedef struct webui_event_t {
    size_t window;          // The window object number
    size_t event_type;      // Event type
    char* element;          // HTML element ID
    size_t event_number;    // Internal WebUI
    size_t bind_id;         // Bind ID
    size_t client_id;       // Client's unique ID
    size_t connection_id;   // Client's connection ID
    char* cookies;          // Client's full cookies
} webui_event_t;
*/

void WebUIWindow::webuiEvent(webui_event_t *e)
{
    if (e->event_type == WEBUI_EVENT_CONNECTED) {
        _handler->message(asprintf("Window %d (%d) connected - clientid = %d", _win, _webui_win, e->client_id));
        return;
    } else if (e->event_type == WEBUI_EVENT_DISCONNECTED) {
        _handler->message(asprintf("Window %d (%d) disconnected - clientid = %d", _win, _webui_win, e->client_id));
        return;
    } else if (e->event_type == WEBUI_EVENT_MOUSE_CLICK) {
        _handler->message(asprintf("Window %d (%d) mouseclick - clientid = %d", _win, _webui_win, e->client_id));
        return;
    } else if (e->event_type == WEBUI_EVENT_NAVIGATION) {
        const char* url = webui_get_string(e);
        std::string r_u = replace(url, "\"", "\\\"");
        std::string kind = "set-url";
        if (r_u.rfind(baseUrl(), 0) == 0) {
            kind = "set-html";
        }
        _handler->evt(asprintf("navigate:%d:%s:%s:%s", e->window, url, "standard", kind.c_str()));
        return;
    }
    _handler->message(asprintf("webui-event: %s: %d %d", e->element, e->event_type, e->event_number));
}

void WebUIWindow::handleWireEvent(webui_event_t *e)
{
    std::string event = webui_get_string(e);
    _handler->evt(event);
}

void WebUIWindow::handleResizeEvent(webui_event_t *e)
{
    int w = webui_get_int_at(e, 0);
    int h = webui_get_int_at(e, 1);
    _handler->windowResized(_win, w, h);
}

void WebUIWindow::handleMoveEvent(webui_event_t *e)
{
    int x = webui_get_int_at(e, 0);
    int y = webui_get_int_at(e, 1);
    _handler->windowMoved(_win, x, y);
}

WebUIWindow::WebUIWindow(WebWireHandler *h, int win, const std::string &p, WebUIWindow *parent_win, Object_t *parent)
    : Object_t(parent)
{
    _win = win;
    _webui_win = win;
    _handler = h;
    _profile = p;
    _ww_profile = nullptr;
    _closing = false;
    _win_handle = NULL;
    _use_browser = false;
    _handler = h;
    _parent_win = parent_win;

    _webui_win = webui_new_window();
    h->message(asprintf("_webui_win = %d", _webui_win));
    _windows[_webui_win] = this;

    webui_set_close_handler(_webui_win, web_ui_wire_on_close);

    webui_bind(_webui_win, "", webui_event_handler);
    webui_bind(_webui_win, "web_ui_wire_handle_event", web_ui_wire_handle_event);
    webui_bind(_webui_win, "web_ui_wire_resize_event", web_ui_wire_handle_resize);
    webui_bind(_webui_win, "web_uit_wire_move_event", web_ui_wire_handle_move);

    _webui_port = _handler->serverPort() + _win;
    _handler->message(asprintf("webui_set_port %d", _webui_port));
    webui_set_port(_webui_win, _webui_port);
}

WebUIWindow::~WebUIWindow()
{
    _windows.erase(_webui_win);
}

void WebUIWindow::useBrowser(bool y)
{
    if (!webui_is_shown(_webui_win)) {
        _use_browser = y;
    } else {
        _handler->error("You cannot use 'useBrowser' after a window has been shown");
    }
}

HWND WebUIWindow::nativeHandle()
{
    return _win_handle;
}

void WebUIWindow::show(const std::string &msg_or_url)
{
    if (_use_browser) {
        webui_show_browser(_webui_win, msg_or_url.c_str(), webui_browser::ChromiumBased);
    } else {
        webui_show(_webui_win, msg_or_url.c_str());
    }
#ifdef _WINDOWS
    //TODO: do this onces and position window in center of parent window.
    _win_handle = static_cast<HWND>(webui_win32_get_hwnd(_webui_win));
    if (_parent_win != nullptr) {
        HWND _parent_handle = _parent_win->_win_handle;
        if (_parent_handle) {
            EnableWindow(_parent_handle, FALSE);
        }
    }
#endif
}

void WebUIWindow::showIfNotShown()
{
    if (!webui_is_shown(_webui_win)) {
        show(baseUrl());
    }
}

void WebUIWindow::setUrl(const upa::url &u)
{
    std::string s_u = u.to_string();

    if (!webui_is_shown(_webui_win)) {
        show(baseUrl());
    }

    webui_navigate(_webui_win, s_u.c_str());
}

void WebUIWindow::setHtml(std::string url)
{
    show(url);
}
