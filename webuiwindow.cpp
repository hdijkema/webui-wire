#include "webuiwindow.h"
#include "webui_wire.h"
#include "webwirehandler.h"
#include "fileinfo_t.h"
#include "misc.h"
#include "execjs.h"
#include "webwireprofile.h"
#include "httpresponse_t.h"
#include "mimetypes_t.h"
#include <regex>
#include <string.h>


#define WEBWIREHANDLER ((Application_t::current() == nullptr) ? nullptr : (Application_t::current()->handler()))

static wwhash<size_t, WebUIWindow *> _windows;

static WebUIWindow *get_webui_window(size_t webui_win)
{
    if (_windows.contains(webui_win)) {
        return _windows[webui_win];
    } else {
        WebWireHandler *h = WEBWIREHANDLER;
        if (h != nullptr) {
            h->error(asprintf("Did not find webui %d", webui_win));
        }
        return nullptr;
    }
}

static void web_ui_wire_handle_event(webui_event_t *e)
{
    WebWireHandler *h = WEBWIREHANDLER;
    if (h != nullptr) {
        h->message(asprintf("web-ui-wire-handle-event: %d, %s", e->window, e->element));
        WebUIWindow *win = get_webui_window(e->window);
        if (win != nullptr) win->handleWireEvent(e);
    }
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

static bool web_ui_wire_on_navigation(size_t window)
{
    WebUIWindow *win = get_webui_window(window);
    if (win != nullptr) {
        return win->mayNavigate();
    } else { // No window found, close always permitted
        return true;
    }
}

static const void *web_ui_wire_files_handler(size_t window, const char *filename, int* length)
{
    //fprintf(stderr, "%s\n", filename);
    WebUIWindow *win = get_webui_window(window);
    if (win != nullptr) {
        return win->filesHandler(filename, length);
    } else {
        *length = 0;
        return nullptr;
    }
}

static void webui_event_handler(webui_event_t *e)
{
    size_t webui_win = e->window;
    if (_windows.contains(webui_win)) {
        WebUIWindow *win = _windows[webui_win];
        win->webuiEvent(e);
    } else {
        WebWireHandler *h = WEBWIREHANDLER;
        if (h != nullptr) {
            h->error(asprintf("Did not find webui %d", webui_win));
        }
    }
}

#ifdef _WINDOWS
#define strnicmp _strnicmp
#endif

#ifdef __linux
#define strnicmp strncasecmp
#endif

#ifdef __APPLE__
#define strnicmp strncasecmp
#endif

static bool isHTML(const char *_buf, int max_search)
{
    bool h = false;
    int i = 0;
    const unsigned char *buf = reinterpret_cast<const unsigned char*>(_buf);
    while (buf[i] != '\0' && isspace(buf[i]) && i < max_search) {
        i++;
    }

    if (buf[i] == '\0' || i >= max_search) return false;
    if (strnicmp(&_buf[i], "<html", 5) == 0) return true;

    if (strnicmp(&_buf[i], "<!", 2) == 0) {
        i += 2;
        while (buf[i] != '\0' && isspace(buf[i]) && i < max_search) { i++; }
        if (buf[i] == '\0' || i >= max_search) return false;
        if (strnicmp(&_buf[i], "DOCTYPE", 7) == 0) {
            i += 7;
            while (buf[i] != '\0' && isspace(buf[i]) && i < max_search) { i++; }
            if (buf[i] == '\0' || i >= max_search) return false;
            if (strnicmp(&_buf[i], "html", 4) == 0) return true;
        }
    }

    return false;
}


int WebUIWindow::newHandle()
{
    _handle_counter++;
    int handle = (((_handle_counter * 1000) + _win) * 1000) + _webui_win;
    return handle;
}

std::string WebUIWindow::baseUrl()
{
    //return asprintf("http://127.0.0.1:%d/", webui_get_port(_webui_win));
    //return asprintf("http://127.0.0.1:%d/", _handler->port());
    //return _handler->serverUrl() + _profile + "/";
    //return _handler->serverUrl() + asprintf("%d/", _win);
    //return asprintf("http://127.0.0.1:%d/%d/", webui_get_port(_webui_win), _win);
    int port = webui_get_port(_win);
    _handler->message(asprintf("port = %d", port));
    std::string url = asprintf("%s", webui_get_url(_win));
    _handler->message("webui_get_url = " + url);
    _handler->message("_base_url = " + _base_url);
    std::string burl = _base_url + "/"; // + asprintf("%d/", _win);
    return burl;
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

void WebUIWindow::setWindowIcon(const std::string &icn_file)
{
    FileInfo_t fi(icn_file);
    if (fi.exists() && fi.isReadable() && fi.ext() == "svg") {
        MimeTypes_t m(_handler);
        std::string type = m.mimetypeByExt(fi.ext());
        int size = fi.size();
        FILE *f;
#ifdef _WINDOWS
        fopen_s(&f, icn_file.c_str(), "rb");
#else
        f = fopen(icn_file.c_str(), "rb");
#endif
        if (f) {
            char *buf = static_cast<char *>(malloc(size));
            if (buf) {
                fread(buf, size, 1, f);
                webui_set_icon(_webui_win, buf, type.c_str());
                free(buf);
            }
            fclose(f);
        }
    } else if (fi.ext() != "svg") {
        _handler->warning("setWindowIcon can currently only handle SVG files");
    } else if (!fi.isReadable()) {
        _handler->error("setWindowIcon: file is not readable: " + icn_file);
    } else {
        _handler->error("setWindowIcon: file does not exist: " + icn_file);
    }
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
    if (_closing) {
#ifdef _WINDOWS
        if (webui_is_shown(_webui_win)) {
            if (_parent_win != nullptr) {
                HWND _parent_handle = _parent_win->_win_handle;
                if (_parent_handle) {
                    EnableWindow(_parent_handle, TRUE);
                }
            }
        }
#endif
#ifdef __linux
        // We don't need to do something here.
        // Gtk Handles this differently
#endif
        webui_close(_webui_win);
        //webui_wait();
        //webui_destroy(_webui_win);
        //webui_set_hide(_webui_win, true);
    }
}

bool WebUIWindow::canClose()
{
    _handler->evt(asprintf("close-request:%d", _win));
    if (_closing) { return true; }
    return false;
}

bool WebUIWindow::mayNavigate()
{
    if (_set_html_or_url_done) {
        _set_html_or_url_done = false;
        return true;
    }
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

std::string WebUIWindow::standardMessage()
{
    WinInfo_t *i = _handler->getWinInfo(_win);

    std::string profile_scripts;
    if (i != nullptr) {
        profile_scripts = i->profile->scriptsTag();
    }

    std::string webui_js = asprintf("/webui.js");
    std::string window = asprintf("%d", _win);


    std::string standard_msg = "<!DOCTYPE html>\n"
                               "<html>\n"
                               "<head>\n"
                               "<script>\n"
                               "window._page_handle = " + asprintf("%d", _current_handle) + ";\n"
                               "</script>\n"
                               "<script src=\"" + webui_js + "\"></script>\n" +
                               profile_scripts +
                               "<title>Web UI Wire: " WEB_WIRE_VERSION "</title>\n"
                               "</head>\n"
                               "<body><p>Web UI Wire:" WEB_WIRE_VERSION "</p>\n"
                               "<p>" WEB_WIRE_COPYRIGHT "</p>\n"
                               "<p> License: " WEB_WIRE_LICENSE "</p>\n"
                               "<p> Window: " + window + "</p>\n"
                               "</body>\n"
                               "</html>";
    return standard_msg;
}

const void *WebUIWindow::filesHandler(const char *url_path, int *length)
{
    _handler->message(std::string("Serving url path: ") + url_path);
    std::regex re("[/](.*)");
    std::smatch m;
    std::string file_path = url_path;
    std::regex_search(file_path, m, re);

    WinInfo_t *i = _handler->getWinInfo(_win);
    std::string profile_scripts;
    if (i != nullptr) {
        profile_scripts = i->profile->scriptsTag();
    }

    Timer_t *t = _handler->getTimer(_win);
    if (t != nullptr) {
        t->reset();
    }

    std::string webui_js = "/webui.js";
    std::string window = asprintf("%d", _win);

    bool root_url = false;
    bool empty_url = false;
    std::string file;
    if (m.empty()) {
        empty_url = true;
    } else {
        std::string part = trim_copy(m[1].str());
        if (part == "") {
            root_url =true;
        } else {
#ifdef _WINDOWS
            file = part;        // To prevent /C:/<path>
#else
            file = file_path;
#endif
        }
    }

    if (root_url || empty_url) {
        std::string standard_msg = standardMessage();
        HttpResponse_t resp(_handler, 200);
        resp.setContentType("text/html");
        resp.setContent(standard_msg);
        return resp.response(*length);
    } else {
        FileInfo_t fi(file);
        HttpResponse_t resp(_handler);
        if (fi.exists() && fi.isReadable()) {
            size_t file_size = fi.size();
            int max_search = (file_size > 1024) ? 1024 : file_size;
#ifdef _WINDOWS
            FILE *f;
            fopen_s(&f, file.c_str(), "rb");
#else
            FILE *f = fopen(file.c_str(), "rb");
#endif
            char *buffer = static_cast<char *>(malloc(max_search + 1));
            memset(buffer, 0, max_search + 1);
            fread(buffer, max_search, 1, f);
            buffer[max_search] = '\0';
            std::string content_type;
            if (fi.ext() == "html" || fi.ext() == "htm" || isHTML(buffer, max_search)) {
                buffer = static_cast<char *>(realloc(buffer, file_size + 1));
                fread(buffer + max_search, file_size - max_search, 1, f);
                fclose(f);
                buffer[file_size] = '\0';

                std::string body(buffer);
                free(buffer);
                body = replace(body, "<head>", "<head>\n"
                                               "<script>\n"
                                               "window._page_handle = " + asprintf("%d", _current_handle) + ";\n"
                                               "</script>\n"
                                               "<script src=\"" + webui_js + "\"></script>\n"
                                               + profile_scripts
                               );
                content_type = "text/html";

                resp.setContentType(content_type);
                resp.setResponseCode(200);
                resp.setContent(body);
            } else {
                fclose(f);
                free(buffer);
                resp.setFile(file);
            }
            return resp.response(*length);
        } else if (file.rfind("favicon.", 0) == 0) {
            return nullptr;
        } else {
            HttpResponse_t resp(_handler, 404);
            resp.setContentType("text/html");
            resp.setContent("<!DOCTYPE html>\n"
                           "<html>\n"
                           "<head>\n"
                           "<script>\n"
                            "window._page_handle = " + asprintf("%d", _current_handle) + ";\n"
                           "</script>\n"
                           "<script src=\"" + webui_js + "\"></script>\n" +
                           profile_scripts +
                           "</head>"
                           "<body><p>Web UI Wire:" WEB_WIRE_VERSION "</p>"
                           "<p>Not found: " + file + "</p>"
                            "</body>"
                            "</html>"
                          );
            return resp.response(*length);
        }
    }
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
        _disconnected = false;
        Timer_t *t = _handler->getTimer(_win);
        if (t != nullptr) {
            t->stop();
        }
        _handler->message(asprintf("Window %d (%d) connected - clientid = %d", _win, _webui_win, e->client_id));
        return;
    } else if (e->event_type == WEBUI_EVENT_DISCONNECTED) {
        _handler->message(asprintf("Window %d (%d) disconnected - clientid = %d", _win, _webui_win, e->client_id));
        _disconnected = true;
        if (!_closing && !_in_set_html_or_url) {
            Timer_t *t = _handler->getTimer(_win);
            if (t != nullptr) {
                t->setSingleShot(true);
                t->setTimeout(500);            // Check if still disconnected after 1.5seconds
                t->start();
            }
        }
        return;
    } else if (e->event_type == WEBUI_EVENT_MOUSE_CLICK) {
        _handler->message(asprintf("Window %d (%d) mouseclick - clientid = %d", _win, _webui_win, e->client_id));
        return;
    } else if (e->event_type == WEBUI_EVENT_NAVIGATION) {
        const char* url = webui_get_string(e);
         _handler->message(asprintf("Window %d (%d) navigation - clientid = %d, url %s",
                                   _win, _webui_win, e->client_id,
                                   url));
        std::string r_u = url; //replace(url, "\"", "\\\"");
        std::string kind = "set-url";
        if (r_u.rfind(baseUrl(), 0) == 0) {
            kind = "set-html";
            r_u = r_u.substr(baseUrl().length());
        }
        json j;
        j["url"] = r_u;
        j["navigation-type"] = "standard";
        j["navigation-kind"] = kind;
        std::string evt = asprintf("navigate:%d:%s", _win, j.dump().c_str());
        _handler->message(evt);
        _handler->evt(evt);
        return;
    }
    _handler->message(asprintf("webui-event: %s: %d %d", e->element, e->event_type, e->event_number));
}

void WebUIWindow::handleWireEvent(webui_event_t *e)
{
    std::string event = webui_get_string(e);
    _handler->message(event);
    json j = json::parse(event);
    std::string evt = j["evt"];
    if (evt == "script-result") {
        if (_exec_js != nullptr) {
            // always expect string result here
            _exec_js->setResult(j["result"], j["result_ok"], j["result_msg"]);
            _exec_js = nullptr;
        } else {
            _handler->error(asprintf("handleWireEvent: Unexpected script-result: %s", event.c_str()));
        }
    } else {
        _handler->evt(evt + ":" + asprintf("%d", _win) + ":" + event);
    }
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

WebUIWindow::WebUIWindow(WebWireHandler *h, int win, const std::string &p, bool use_browser, WebUIWindow *parent_win, Object_t *parent)
    : Object_t(parent)
{
    _win = win;
    _webui_win = win;
    _handler = h;
    _profile = p;
    _ww_profile = nullptr;
    _closing = false;
    _in_set_html_or_url = false;
    _set_html_or_url_done = false;
    _win_handle = NULL;
    _use_browser = use_browser;
    _handler = h;
    _parent_win = parent_win;
    _disconnected = false;
    _current_handle = -1;
    _handle_counter = 0;
    _exec_js = nullptr;

    _webui_win = webui_new_window();
    h->message(asprintf("_webui_win = %d", _webui_win));
    _windows[_webui_win] = this;

    webui_set_file_handler_window(_webui_win, web_ui_wire_files_handler);
    webui_set_close_handler_wv(_webui_win, web_ui_wire_on_close);
    //webui_set_navigation_handler_wv(_webui_win, web_ui_wire_on_navigation);
    webui_set_icon(_webui_win, _default_favicon, "image/svg+xml");

    webui_bind(_webui_win, "", webui_event_handler);
    webui_bind(_webui_win, "web_ui_wire_handle_event", web_ui_wire_handle_event);
    webui_bind(_webui_win, "web_ui_wire_resize_event", web_ui_wire_handle_resize);
    webui_bind(_webui_win, "web_uit_wire_move_event", web_ui_wire_handle_move);

    //show(standardMessage());

    std::string root_path = "/";
    _base_url = webui_start_server(_webui_win, root_path.c_str());
    //_base_url = webui_get_url(_webui_win);
    //int prt = webui_get_port(_webui_win);
    _handler->message("webui reports url : " + _base_url);
    //_handler->message(asprintf("webui reports port: %d", prt));
    std::regex re("[^:]+[:]([0-9]+)");
    std::smatch m;
    std::regex_search(_base_url, m, re);
    if (!m.empty()) {
        _base_url = std::string("http://127.0.0.1:") + m[1].str();
    }

    show(_base_url);

    //_webui_port = _handler->serverPort() + _win;
    //_handler->message(asprintf("webui_set_port %d", _webui_port));
    //webui_set_port(_webui_win, _webui_port);
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

void WebUIWindow::setShowState(WebUiWindow_ShowState st)
{
    if (st == hidden) {
#ifdef _WINDOWS
        HWND handle = this->nativeHandle();
        ShowWindow(handle, SW_HIDE);
#else
#endif
    } else if (st == shown) {
#ifdef _WINDOWS
        HWND handle = this->nativeHandle();
        ShowWindow(handle, SW_SHOW);
#else
#endif
    } else if (st == minimized) {
        webui_minimize(_webui_win);
    } else if (st == maximized) {
        webui_maximize(_webui_win);
    } else if (st == st_normal) {
#ifdef _WINDOWS
        HWND handle = this->nativeHandle();
        ShowWindow(handle, SW_SHOWNORMAL);
#else
#endif
    }
}

int WebUIWindow::showState()
{
#ifdef _WINDOWS
    HWND handle = this->nativeHandle();
    int v = 0;
    if (IsWindowVisible(handle)) {
        v = WebUiWindow_ShowState::shown;
    }

    if (IsIconic(handle)) {
        v += WebUiWindow_ShowState::minimized;
    } else if (IsZoomed(handle)) {
        v += WebUiWindow_ShowState::maximized;
    } else {
        v += WebUiWindow_ShowState::st_normal;
    }

    return v;
#else
    return shown;
#endif
}

void WebUIWindow::setExecJs(ExecJs *e)
{
    _exec_js = e;
}

bool WebUIWindow::disconnected()
{
    return _disconnected;
}

#ifdef __linux
GtkWindow *WebUIWindow::nativeHandle()
#else
#ifdef _WINDOWS
    HWND WebUIWindow::nativeHandle()
#else
    NSWindow *WebUIWindow::nativeHandle()
#endif
#endif
{
    return _win_handle;
}

int WebUIWindow::show(const std::string &msg_or_url)
{
    int handle = newHandle();
    _in_set_html_or_url = true;
    _set_html_or_url_done = true;
    _current_handle = handle;
    if (_use_browser) {
        webui_show_browser(_webui_win, msg_or_url.c_str(), webui_browser::ChromiumBased);
    } else {
        webui_show_wv(_webui_win, msg_or_url.c_str());
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
#ifdef __linux
    _win_handle = static_cast<GtkWindow *>(webui_get_hwnd(_webui_win));
    if (_parent_win != nullptr) {
        GtkWindow * _parent_handle = _parent_win->_win_handle;
        if (_parent_handle) {
            gtk_window_set_transient_for(_win_handle, _parent_handle);
            gtk_window_set_modal(_win_handle, true);
        }
    }
#endif
    _in_set_html_or_url = false;
    return handle;
}

int WebUIWindow::showIfNotShown()
{
    if (!webui_is_shown(_webui_win)) {
        return show(baseUrl());
    }
    return -1;
}

int WebUIWindow::setUrl(const upa::url &u)
{
    std::string s_u = u.to_string();

    if (!webui_is_shown(_webui_win)) {
        return show(baseUrl());
    }

    int handle = newHandle();
    webui_navigate(_webui_win, s_u.c_str());
    return handle;
}

int WebUIWindow::setHtml(std::string url)
{
    return show(url);
}


const char *WebUIWindow::_default_favicon =
    "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n"
    "<svg\n"
    "   width=\"512\"\n"
    "   height=\"512\"\n"
    "   viewBox=\"0 0 135.46666 135.46667\"\n"
    "   version=\"1.1\"\n"
    "   id=\"svg5\"\n"
    "   xml:space=\"preserve\"\n"
    "   inkscape:version=\"1.2.2 (b0a8486541, 2022-12-01)\"\n"
    "   sodipodi:docname=\"icon.svg\"\n"
    "   inkscape:export-filename=\"icon.png\"\n"
    "   inkscape:export-xdpi=\"96\"\n"
    "   inkscape:export-ydpi=\"96\"\n"
    "   xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\"\n"
    "   xmlns:sodipodi=\"http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd\"\n"
    "   xmlns:xlink=\"http://www.w3.org/1999/xlink\"\n"
    "   xmlns=\"http://www.w3.org/2000/svg\"\n"
    "   xmlns:svg=\"http://www.w3.org/2000/svg\"><sodipodi:namedview\n"
    "     id=\"namedview7\"\n"
    "     pagecolor=\"#ffffff\"\n"
    "     bordercolor=\"#000000\"\n"
    "     borderopacity=\"0.25\"\n"
    "     inkscape:showpageshadow=\"2\"\n"
    "     inkscape:pageopacity=\"0.0\"\n"
    "     inkscape:pagecheckerboard=\"0\"\n"
    "     inkscape:deskcolor=\"#d1d1d1\"\n"
    "     inkscape:document-units=\"mm\"\n"
    "     showgrid=\"false\"\n"
    "     inkscape:zoom=\"1.829812\"\n"
    "     inkscape:cx=\"278.17065\"\n"
    "     inkscape:cy=\"409.33167\"\n"
    "     inkscape:window-width=\"2232\"\n"
    "     inkscape:window-height=\"1808\"\n"
    "     inkscape:window-x=\"756\"\n"
    "     inkscape:window-y=\"210\"\n"
    "     inkscape:window-maximized=\"0\"\n"
    "     inkscape:current-layer=\"layer1\" /><defs\n"
    "     id=\"defs2\"><rect\n"
    "       x=\"149.77185\"\n"
    "       y=\"134.79895\"\n"
    "       width=\"49.260365\"\n"
    "       height=\"71.158693\"\n"
    "       id=\"rect474\" /><clipPath\n"
    "       id=\"a\"><path\n"
    "         d=\"M 2,1 H 15 V 33 H 28 V 17 h 13 v 48 h 13 v 64 H 2 Z\"\n"
    "         id=\"path587\" /></clipPath></defs><g\n"
    "     inkscape:label=\"Laag 1\"\n"
    "     inkscape:groupmode=\"layer\"\n"
    "     id=\"layer1\"><rect\n"
    "       style=\"fill:#333333;stroke:#1a1a1a;stroke-width:0.609071\"\n"
    "       id=\"rect1000\"\n"
    "       width=\"134.77257\"\n"
    "       height=\"134.8334\"\n"
    "       x=\"0.43407014\"\n"
    "       y=\"0.49147958\"\n"
    "       ry=\"25.336727\" /><g\n"
    "       transform=\"matrix(3.9029613,0,0,-3.9029613,-1025.5226,337.56542)\"\n"
    "       id=\"g4677\" /><g\n"
    "       clip-path=\"url(#a)\"\n"
    "       id=\"g661\"\n"
    "       transform=\"matrix(0,-0.77086785,-0.90514765,0,125.60251,128.00586)\"><g\n"
    "         id=\"f\"><path\n"
    "           d=\"M 2.75,34.6438 V 46.3 h 8.24375 V 34.6438 Z m 0,16 V 62.3 h 8.24375 V 50.6438 Z m 0,16 V 78.3 h 8.24375 V 66.6438 Z\"\n"
    "           fill=\"#ffff01\"\n"
    "           id=\"path648\" /><path\n"
    "           d=\"M 2.75,82.6438 V 94.3 h 8.24375 V 82.6438 Z m 0,16 V 110.3 h 8.24375 V 98.6438 Z m 0,16 V 126.3 h 8.24375 v -11.6562 z\"\n"
    "           fill=\"#2ea02a\"\n"
    "           id=\"path650\" /><path\n"
    "           d=\"m 2.75,2.8625 v 11.6562 h 8.24375 V 2.8625 Z m 0,15.7812 v 11.6562 h 8.24375 V 18.6437 Z\"\n"
    "           fill=\"#ff0000\"\n"
    "           id=\"path652\" /></g><use\n"
    "         height=\"130\"\n"
    "         transform=\"translate(13)\"\n"
    "         width=\"265\"\n"
    "         xlink:href=\"#f\"\n"
    "         id=\"use655\" /><use\n"
    "         height=\"130\"\n"
    "         transform=\"translate(26)\"\n"
    "         width=\"265\"\n"
    "         xlink:href=\"#f\"\n"
    "         id=\"use657\" /><use\n"
    "         height=\"130\"\n"
    "         transform=\"translate(39)\"\n"
    "         width=\"265\"\n"
    "         xlink:href=\"#f\"\n"
    "         id=\"use659\" /></g><text\n"
    "       xml:space=\"preserve\"\n"
    "       transform=\"scale(0.26458333)\"\n"
    "       id=\"text472\"\n"
    "       style=\"white-space:pre;shape-inside:url(#rect474);display:inline;fill:#ffffff;stroke:#1a1a1a;stroke-width:2.302\" /><text\n"
    "       xml:space=\"preserve\"\n"
    "       style=\"font-size:51.0415px;fill:#9cffff;fill-opacity:1;stroke:#437dce;stroke-width:1.46472;stroke-dasharray:none;stroke-opacity:1\"\n"
    "       x=\"15.069036\"\n"
    "       y=\"61.443043\"\n"
    "       id=\"text1176\"\n"
    "       transform=\"scale(0.85145276,1.1744633)\"><tspan\n"
    "         sodipodi:role=\"line\"\n"
    "         id=\"tspan1174\"\n"
    "         style=\"fill:#9cffff;fill-opacity:1;stroke:#437dce;stroke-width:1.46472;stroke-dasharray:none;stroke-opacity:1\"\n"
    "         x=\"15.069036\"\n"
    "         y=\"61.443043\">Yeah!</tspan></text></g></svg>\n"
    "\n";
