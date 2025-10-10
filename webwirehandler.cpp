#include "webwirehandler.h"

#include "webui_wire.h"
#include <filesystem>
#include <regex>
#include "fileinfo_t.h"
#include "utf8_strings.h"
#include "application_t.h"
#include "default_css.h"
#include "webwireprofile.h"
#include "webuiwindow.h"
#include "webwirestandarddialogs.h"
#include "execjs.h"

namespace fs = std::filesystem;

// Command handling

#ifdef Q_OS_LINUX
#include <unistd.h>
#endif

#define defun(name)         static void name(std::string cmd, WebWireHandler *h, const std::stringlist &args)

#define r_ok(str)           h->addOk(str)
#define r_nok(str)          h->addNOk(str)
#define r_err(str)          h->addErr(str)
#define msg(str)            h->message(str);

#define view(win)           h->getView(win)

#define var(type, v)        Var(type, v, #v)
#define opt(type, v, d)     Var(type, v, #v, true, d)

#define check(cmd, vars)    h->getArgs(cmd, win, wwlist<Var>() << vars, args)
#define checkWin            WebUIWindow *w = h->getWindow(win); \
                            if (w == nullptr) { \
                               r_err(cmd + ": window " + asprintf("%d", win) + " does not exist"); \
                               r_nok(cmd + ":" + asprintf("%d", win)); \
                               return; \
                            }

#define js_el(id, code)     std::string("let el = document.getElementById('") + id + "');" + \
                            std::string("if (el !== null) { ") + code + "}"

defun(cmdSetUrl)
{
    int win = -1;
    url url_location;

    if (check("set-url", var(t_int, win) << var(t_url, url_location))) {
        checkWin;

        w->setUrl(url_location);
        r_ok(asprintf("set-url:%d:done", win));
    }
}

defun(cmdExecJs)
{
    int win = -1;
    std::string code;
    if (check("exec-js", var(t_int, win) << var(t_string, code))) {
        checkWin;
        bool ok;
        std::string result;
        h->execJs(win, code, ok, result);
        if (ok) {
            r_ok(asprintf("exec-js:%d:", win) + ExecJs::esc_dquote(result));
        } else {
            r_nok(asprintf("exec-js:%d", win));
        }
    }
}

defun(cmdUseBrowser)
{
    int win = -1;
    bool browser = false;
    if (check("use-browser", var(t_int, win) << var(t_bool, browser))) {
        checkWin;

        w->useBrowser(browser);
        r_ok(asprintf("use-browser:%d", win));
    }
}

defun(cmdSetHtml)
{
    int win = -1;
    std::string file;
    if (check("set-html", var(t_int, win) << var(t_string, file))) {
        checkWin;

        FileInfo_t f(file);
        if (f.exists()) {
            if (!f.isReadable()) {
                r_err(asprintf("set-html:%d:file ", win) + file + " is not readable");
                r_nok(asprintf("set-html:%d", win));
            } else {

                std::filesystem::path the_file = std::filesystem::absolute(file);

                h->message(std::string("the_file: ") + the_file.string());

                std::string base_url = w->baseUrl();
                std::string url = base_url + the_file.string();
                upa::url u(url);
                std::string p_url = u.to_string();
                h->message(std::string("requesting: ") + p_url);

                int handle = w->setHtml(p_url);
                r_ok(asprintf("set-html:%d:%d", win, handle));
            }
        } else {
            r_err(asprintf("set-html:%d:file ", win) + file + " does not exist");
            r_nok(asprintf("set-html:%d", win));
        }
    }
}

defun(cmdSetInnerHtml)
{
    int win = -1;
    std::string id;
    std::string data;
    if (check("set-inner-html", var(t_int, win) << var(t_string, id) << var(t_string, data))) {
        FileInfo_t f(data);
        bool is_file = false;
        if (f.exists()) {
            if (f.isReadable()) {
                is_file = true;
            }
        }

        checkWin;

        WinInfo_t *i = h->getWinInfo(win);
        if (is_file) {
            std::string base_url = w->baseUrl();
            std::string url = base_url + data;
            upa::url u(url);
            std::string p_url = u.to_string();
            i->profile->set_html(h, win, id, p_url, true);
        } else {
            i->profile->set_html(h, win, id, data, false);
        }
        r_ok(asprintf("set-inner-html:%d:", win));
    }
}

defun(cmdGetInnerHtml)
{
    int win = -1;
    std::string id;
    if (check("get-inner-html", var(t_int, win) << var(t_string, id))) {
        checkWin;

        int r_handle;
        WinInfo_t *i = h->getWinInfo(win);
        bool ok;
        std::string result = i->profile->get_html(h, win, id, ok);
        if (ok) {
            r_ok(asprintf("get-inner-html:%d:", win) + ExecJs::esc_dquote(result));
        } else {
            r_nok(asprintf("get-inner-html:%d", win));
        }
    }
}

defun(cmdSetAttr)
{
    int win = -1;
    std::string id;
    std::string attr;
    std::string val;

    if (check("set-attr", var(t_int, win) << var(t_string, id) << var(t_string, attr) << var(t_string, val))) {
        checkWin;

        int r_handle;
        WinInfo_t *i = h->getWinInfo(win);
        i->profile->set_attr(h, win, id, attr, val);
        r_ok(asprintf("set-attr:%d", win));
    }
}

defun(cmdGetAttr)
{
    int win = -1;
    std::string id;
    std::string attr;

    if (check("get-attr", var(t_int, win) << var(t_string, id) << var(t_string, attr))) {
        checkWin;

        WinInfo_t *i = h->getWinInfo(win);

        bool ok;
        std::string result = i->profile->get_attr(h, win, id, attr, ok);
        if (ok) {
            r_ok(asprintf("get-attr:%d:", win) + ExecJs::esc_dquote(result));
        } else {
            r_nok(asprintf("get-attr:%d", win));
        }
    }
}

defun(cmdGetAttrs)
{
    int win = -1;
    std::string id;

    if (check("get-attrs", var(t_int, win) << var(t_string, id))) {
        checkWin;

        WinInfo_t *i = h->getWinInfo(win);

        bool ok;
        std::string result = i->profile->get_attrs(h, win, id, ok);
        if (ok) {
            r_ok(asprintf("get-attrs:%d:", win) + ExecJs::esc_dquote(result));
        } else {
            r_nok(asprintf("get-attrs:%d", win));
        }
    }
}

defun(cmdGetElements)
{
    int win = -1;
    std::string selector;

    if (check("get-attrs", var(t_int, win) << var(t_string, selector))) {
        checkWin;

        WinInfo_t *i = h->getWinInfo(win);
        bool ok;
        std::string result = i->profile->get_elements(h, win, selector, ok);
        if (ok) {
            r_ok(asprintf("get-elements:%d:", win) + result);
        } else {
            r_nok(asprintf("get-elements:%d", win));
        }
    }
}

defun(cmdDelAttr)
{
    int win = -1;
    std::string id;
    std::string attr;

    if (check("del-attr", var(t_int, win) << var(t_string, id) << var(t_string, attr))) {
        checkWin;

        WinInfo_t *i = h->getWinInfo(win);

        i->profile->del_attr(h, win, id, attr);
        r_ok(asprintf("del-attr:%d", win));
    }
}

defun(cmdSetStyle)
{
    int win = -1;
    std::string id;
    std::string val;

    if (check("set-style", var(t_int, win) << var(t_string, id) << var(t_string, val))) {
        checkWin;

        WinInfo_t *i = h->getWinInfo(win);
        i->profile->set_style(h, win, id, val);
        r_ok(asprintf("set-style:%d", win));
    }
}

defun(cmdAddStyle)
{
    int win = -1;
    std::string id;
    std::string val;

    if (check(cmd, var(t_int, win) << var(t_string, id) << var(t_string, val))) {
        checkWin;

        WinInfo_t *i = h->getWinInfo(win);
        i->profile->add_style(h, win, id, val);
        r_ok(asprintf("add-style:%d", win));
    }
}

defun(cmdGetStyle)
{
    int win = -1;
    std::string id;

    if (check("get-styler", var(t_int, win) << var(t_string, id))) {
        checkWin;

        WinInfo_t *i = h->getWinInfo(win);

        bool ok;
        std::string result = i->profile->get_style(h, win, id, ok);

        if (ok) {
            r_ok(asprintf("get-style:%d:", win) + ExecJs::esc_dquote(result));
        } else {
            r_nok(asprintf("get-style:%d", win));
        }
    }
}

defun(cmdOn)
{
    int win = -1;
    std::string event;
    std::string id;

    if (check("on", var(t_int, win) << var(t_string, event) << var(t_string, id))) {
        checkWin;

        id = replace(id, "'", "\\'");
        std::string js_set_on_evt   = "{"
                                  "  let el = document.getElementById('" + id + "');"
                                  "  if (el !== undefined) {"
                                  "     el.addEventListener("
                                  "        '" + event + "', "
                                  "        function(e) {"
                                  "           let obj = {evt: '" + event + "', id: '" + id +"', js_evt: window._web_wire_event_info('" + event + "', '" + id + "', e) };"
                                  "           window._web_wire_put_evt(obj);"
                                  "        }"
                                  "     );"
                                  "  }"
                                  "}";

        h->execJs(win, js_set_on_evt, "on");
        r_ok(asprintf("on:%d:%s", win, event.c_str()));
    }
}

defun(cmdBind)
{
    int win = -1;
    std::string event;
    std::string selector;

    if (check("bind", var(t_int, win) << var(t_string, event) << var(t_string, selector))) {
        checkWin;

        selector = replace(selector, "'", "\\'");
        event = replace(event, "'", "\\'");

        std::string js_bind_evt = "return window._web_wire_bind_evt_ids('" + selector + "', '" + event + "');";
        bool ok;
        std::string result;
        h->execJs(win, js_bind_evt, ok, result, "bind");
        if (ok) {
            r_ok(asprintf("bind:%d:", win) + ExecJs::esc_dquote(result));
        } else {
            r_nok(asprintf("bind:%d", win));
        }
    }
}

defun(cmdElementInfo)
{
    int win = -1;
    std::string id;

    if (check("element-info", var(t_int, win) << var(t_string, id))) {
        checkWin;

        std::string _id = id;
        _id = replace(_id, "'", "\\'");

        std::string js = "{"
                     "  let el = document.getElementById('" + _id + "');"
                     "  let obj;"
                     "  if ((el === null) || (el === undefined)) {"
                     "    obj = [ '" + _id + "', '', '', false ];"
                     "  } else {"
                     "    let type = el.getAttribute('type');"
                     "    if (type === null) { type = ''; }"
                     "    obj = [ '" + _id + "', el.nodeName, type, true ];"
                     "  }"
                     "  let ret = 'json:' + JSON.stringify(obj);"
                     "  return ret;"
                     "}";

        bool ok;
        std::string result;
        h->execJs(win, js, ok, result, "element-info");
        if (ok) {
            id = ExecJs::esc_dquote(id);
            r_ok(asprintf("element-info:%d:", win) + ExecJs::esc_dquote(result));
        } else {
            r_nok(asprintf("element-info:%d:", win) + id);
        }
    }
}

defun(cmdValue)
{
    int win = -1;
    std::string id;
    std::string val;
    std::string dummy = "@@WEB-WIRE-NONE-GIVEN@@";

    if (check("value", var(t_int, win) << var(t_string, id) << opt(t_string, val, dummy))) {
        checkWin;

        std::string _id = id;
        _id = replace(_id, "'", "\\'");

        std::string js_value;
        if (val == dummy) {
            js_value = "{"
                       "   let el = document.getElementById('" + _id + "');"
                       "   return el.value;"
                       "}";
        } else {
            val = replace(val, "'", "\\'");
            js_value = "{"
                       "   let el = document.getElementById('" + _id + "');"
                       "   el.value = '" + val + "';"
                       "   return el.value;"
                       "}";
        }
        bool ok;
        std::string result;

        h->execJs(win, js_value, ok, result, "value");
        id = ExecJs::esc_dquote(id);
        if (ok) {
            r_ok(asprintf("value:%d:", win) + id + ":" + ExecJs::esc_dquote(result));
        } else {
            r_nok(asprintf("value:%d:", win) + id);
        }
    }
}

defun(cmdAddClass)
{
    int win = -1;
    std::string id;
    std::string cl;

    if (check(cmd, var(t_int, win) << var(t_string, id) << var(t_string, cl))) {
        checkWin;

        std::string _id = id;
        _id = replace(_id, "'", "\\'");

        std::string _cl = cl;
        _cl = replace(_cl, "'", "\\'");

        std::string js = std::string("{") +
                     js_el(_id,
                        "let cl = el.getAttribute('class');"
                        "if (cl === null) { cl = ''; }"
                        "let _cl = '" + _cl + "';"
                        "cl = cl.replace(_cl, '');"
                        "cl += ' ' + _cl;"
                        "cl = cl.replace(/\\s+/g, ' ');"
                        "el.setAttribute('class', cl);"
                    ) + "}";

        h->execJs(win, js, "add-class");
        r_ok(asprintf("add-class:%d", win));
    }
}

defun(cmdRemoveClass)
{
    int win = -1;
    std::string id;
    std::string cl;

    if (check(cmd, var(t_int, win) << var(t_string, id) << var(t_string, cl))) {
        checkWin;

        std::string _id = id;
        _id = replace(_id, "'", "\\'");

        std::string _cl = cl;
        _cl = replace(_cl, "'", "\\'");

        std::string js = std::string("{") +
                     js_el(_id,
                           "let cl = el.getAttribute('class');"
                           "if (cl === null) { cl = ''; }"
                           "let _cl = '" + _cl + "';"
                           "cl = cl.replace(_cl, '');"
                           "cl = cl.replace(/\\s+/g, ' ');"
                           "el.setAttribute('class', cl);"
                    ) + "}";
        h->execJs(win, js, "remove-class");
        r_ok(asprintf("remove-class:%d", win));
    }
}

defun(cmdSetMenu)
{
    int win = -1;
    std::string menu;

    if (check("set-menu", var(t_int, win) << var(t_json_string, menu))) {
        checkWin;

        if (h->setMenu(win, menu)) {
            r_ok(asprintf("set-menu:%d", win));
        } else {
            r_nok(asprintf("set-menu:%d", win));
        }
    }
}


defun(cmdExit)
{
    r_ok("exit:done");
    h->closeListener();
    h->doQuit();
}

defun(cmdDebug)
{
    int win = -1;
    if (check("debug", var(t_int, win))) {
        checkWin;

        h->debugWin(win);

        r_ok(asprintf("debug:%d", win));
    }
}


defun(cmdMove)
{
    int win = -1;
    int x = -1;
    int y = -1;

    if (check("move", var(t_int, win) << var(t_int, x) << var(t_int, y))) {
        checkWin

        h->moveWindow(win, x, y);
        r_ok(asprintf("move:%d", win));
    }

}

defun(cmdClose)
{
    int win = -1;
    if (check("close", var(t_int, win))) {
        checkWin

        h->closeWindow(win);
        r_ok(asprintf("close:%d", win));
    }
}

defun(cmdSetShowState)
{
    int win = -1;
    std::string state;
    if (check("set-show-state", var(t_int, win) << var(t_string, state))) {
        checkWin;

        if (state != "minimized" && state != "maximized" && state != "normal" &&
            state != "shown" && state != "hidden") { // && state != "fullscreen") {
            r_err(asprintf("set-show-state:%d:", win) + std::string("State '") + state + "' is not correct");
            r_nok(asprintf("set-show-state:%d", win));
        } else {
            h->setShowState(win, state);
            r_ok(asprintf("set-show-state:%d:%s", win, h->showState(win).c_str()));
        }
    }
}

defun(cmdShowState)
{
    int win = -1;
    if (check("show-state", var(t_int, win))) {
        checkWin;

        std::string st = h->showState(win);
        r_ok(asprintf("show-state:%d:%s", win, st.c_str()));
    }
}

defun(cmdResize)
{
    int win = -1, width = -1, height = -1;
    if (check("resize", var(t_int, win) << var(t_int, width) << var(t_int, height))) {
        checkWin

        h->resizeWindow(win, width, height);
        r_ok(asprintf("resize:%d", win));
    }

}

defun(cmdSetTitle)
{
    int win = -1;
    std::string title;
    if (check("set-title", var(t_int, win) << var(t_string, title))) {
        checkWin

        h->setWindowTitle(win, title);
        r_ok(asprintf("set-title:%d", win));
    }
}

defun(cmdSetIcon)
{
    int win = -1;
    std::string icon_file;
    if (check("set-icon", var(t_int, win) << var(t_string, icon_file))) {
        checkWin

        FileInfo_t f(icon_file);
        if (f.exists() && f.isReadable()) {
            //QIcon icn(icon_file);
            h->setWindowIcon(win, icon_file);
            r_ok(asprintf("set-icon:%d", win));
        } else {
            if (!f.exists()) {
                r_err(asprintf("set-icon:%d:Icon File '", win) + icon_file + "' does not exist");
                r_nok(asprintf("set-icon:%d", win));
                return;
            }

            if (!f.isReadable()) {
                r_err(asprintf("set-icon:%d:Icon file '", win) + icon_file + "' is not readable");
                r_nok(asprintf("set-icon:%d", win));
                return;
            }
        }
    }
}

defun(cmdFileOpen)
{
    int win = -1;
    std::string title;
    std::string directory;
    std::string file_types;
    if (check("file-open", var(t_int, win) << var(t_string, title) << var(t_string, directory) << var(t_string, file_types))) {
        checkWin;

        bool ok;
        std::string result = h->fileOpen(win, title, directory, file_types, ok);
        if (ok) {
            std::string r = std::string("\"") + replace(result, "\"", "\\\"") + "\"";
            r_ok(asprintf("file-open:%d:%s", win, r.c_str()));
        } else {
            r_nok(asprintf("file-open:%d", win));
        }
    }
}

defun(cmdFileSave)
{
    int win = -1;
    std::string title;
    std::string directory;
    std::string file_types;
    bool overwrite = false;

    if (check("file-save", var(t_int, win) << var(t_string, title) << var(t_string, directory) << var(t_string, file_types) << opt(t_bool, overwrite, false))) {
        checkWin;

        bool ok;
        std::string result = h->fileSave(win, title, directory, file_types, overwrite, ok);
        if (ok) {
            std::string r = std::string("\"") + replace(result, "\"", "\\\"") + "\"";
            r_ok(asprintf("file-save:%d:%s", win, r.c_str()));
        } else {
            r_nok(asprintf("file-save:%d", win));
        }
    }
}

defun(cmdChooseDir)
{
    int win = -1;
    std::string title;
    std::string directory;
    if (check("choose-dir", var(t_int, win) << var(t_string, title) << var(t_string, directory))) {
        checkWin;

        bool ok;
        std::string result = h->chooseDir(win, title, directory, ok);
        if (ok) {
            std::string r = std::string("\"") + replace(result, "\"", "\\\"") + "\"";
            r_ok(asprintf("file-open:%d:%s", win, r.c_str()));
        } else {
            r_nok(asprintf("file-open:%d", win));
        }
    }
}


defun(cmdNewWindow)
{
    std::string profile;
    int parent_win_id = -1;
    int win = 0;
    bool in_browser = false;
    if (check("new", var(t_string, profile) << opt(t_bool, in_browser, false) << opt(t_int, parent_win_id, -1))) {
        int win = h->newWindow(profile, in_browser, parent_win_id);
        r_ok(asprintf("new:%d:%d", win, win));
    }
}

defun(cmdShow)
{
    int win = -1;
    if (check("show", var(t_int, win))) {
        checkWin;
        w->showIfNotShown();
        r_ok(asprintf("show:%d", win));
    }
}

defun(cmdCwd)
{
    std::string cwd;
    int win = 0;
    if (check("cwd", opt(t_string, cwd, ""))) {
        if (cwd != "") {
            std::error_code ec;
            std::filesystem::current_path(cwd, ec);
        }
        std::string d = std::filesystem::current_path().string();
        r_ok("cwd:0:\"" + d + "\"");
    }
}

defun(cmdProtocol)
{
    json j;
    r_ok(asprintf("protocol:0:%d", WEB_WIRE_PROTOCOL_VERSION));
}

defun(cmdLogLevel)
{
    std::string level;
    int win = 0;
    if (check("log-level", opt(t_string, level, ""))) {
        std::string l = lcase(trim_copy(level));
        WebWireLogLevel_t lvl;

        if (l == "") { lvl = h->logLevel();
            if (lvl == WebWireLogLevel_t::debug) { l = "debug"; }
            else if (lvl == WebWireLogLevel_t::info) { l = "info"; }
            else if (lvl == WebWireLogLevel_t::warning) { l = "warning"; }
            else if (lvl == WebWireLogLevel_t::error) { l = "error"; }
            else if (lvl == WebWireLogLevel_t::fatal) { l = "fatal"; }
            else if (lvl == WebWireLogLevel_t::debug_detail) { l = "detail"; }
        }
        else if (l == "debug") { lvl = WebWireLogLevel_t::debug; }
        else if (l == "info") { lvl = WebWireLogLevel_t::info; }
        else if (l == "warning") { lvl = WebWireLogLevel_t::warning; }
        else if (l == "error") { lvl = WebWireLogLevel_t::error; }
        else if (l == "fatal") { lvl = WebWireLogLevel_t::fatal; }
        else if (l == "detail") { lvl = WebWireLogLevel_t::debug_detail; }
        else {
            r_nok("log-level:Unknown log level '" + l + "'");
            return;
        }

        h->setLogLevel(lvl);
        r_ok("loglevel:0:" + l);
    }
}

defun(cmdSetStylesheet)
{
    std::string json_css;
    int win = 0;
    if (check("set-stylesheet", var(t_string, json_css))) {
        try {
            json j = json::parse(json_css);
            std::string css = j["css"];
            h->setStylesheet(css);
            r_ok(std::string("set-stylesheet:0:done"));
        } catch(json::parse_error e) {
            r_nok(std::string("set-stylesheet:0:error") + e.what());
        }
    }
}

defun(cmdGetStyleheet)
{
    std::string css = h->getStylesheet();
    json j;
    j["css"] = css;
    std::string js = j.dump();
    r_ok(std::string("get-stylesheet:0:") + js);
}

defun(cmdHelp)
{
    msg("new <profile> [<win-id>] -> <win-id> - opens a new web wire window with given profile (for cookie storage).");
    msg("                                       The optional <win-id> is a parent window, in which case a modal dialog");
    msg("                                       will be created.");
    msg("close <win> - closes window <win>. It cannot be used after that");
    msg("move <win> <x> <y> - moves window <win> to screen coordinates x, y");
    msg("resize win <width> <height> - resizes window <win> to width, height");
    msg("set-title <title> - sets the window title");
    msg("set-icon <file path:<png|jpg|svg>> - sets the window icon to this bitmap file");
    msg("");
    msg("set-url <win-id> <url> - set webviewer <win-id> to load the given <url>");
    msg("set-html <win-id> <file> - set the html content of the web-wire window <win-id> to file <file>.");
    msg("set-inner-html <win-id> <id> <file|html> - set the inner html of the dom element with id <id> to the contents of <html|file>.");
    msg("get-inner-html <win-id> <id> - get the inner html of the dom element with id <id>.");
    msg("");
    msg("on <win-id> <event> <id> - make the <id> of the html of <win-id> trigger a <event>, ");
    msg("                           event can be any javascript DOM event, e.g. click, input, mousemove, etc.");
    msg("value <win-id> <id> [<value>] - get or set the value of id, always returns the current value by event");
    msg("");
    msg("exit - exit web racket");

    r_ok("help::0:given");
}


#undef msg
#undef view

#define fun(kind, name) if (cmd == kind) { name(cmd, this, args); }
#define efun(kind, name) else if (cmd == kind) { name(cmd, this, args); }

void WebWireHandler::processCommand(const std::string &cmd, const std::stringlist &args)
{
    fun("set-url", cmdSetUrl)
    efun("exit", cmdExit)
    efun("help", cmdHelp)
    efun("move", cmdMove)
    efun("resize", cmdResize)
    efun("close", cmdClose)
    efun("set-title", cmdSetTitle)
    efun("set-icon", cmdSetIcon)
    efun("new", cmdNewWindow)
    efun("set-html", cmdSetHtml)
    efun("show", cmdShow)
    efun("exec-js", cmdExecJs)
    efun("set-inner-html", cmdSetInnerHtml)
    efun("get-inner-html", cmdGetInnerHtml)
    efun("set-attr", cmdSetAttr)
    efun("get-attr", cmdGetAttr)
    efun("get-attrs", cmdGetAttrs)
    efun("get-elements", cmdGetElements)
    efun("del-attr", cmdDelAttr)
    efun("add-style", cmdAddStyle)
    efun("set-style", cmdSetStyle)
    efun("get-style", cmdGetStyle)
    efun("cwd", cmdCwd)
    efun("on", cmdOn)
    efun("bind", cmdBind)
    efun("element-info", cmdElementInfo)
    efun("value", cmdValue)
    efun("set-menu", cmdSetMenu)
    efun("protocol", cmdProtocol)
    efun("add-class", cmdAddClass)
    efun("remove-class", cmdRemoveClass)
    efun("debug", cmdDebug)
    efun("set-show-state", cmdSetShowState)
    efun("show-state", cmdShowState)
    efun("set-stylesheet", cmdSetStylesheet)
    efun("get-stylesheet", cmdGetStyleheet)
    efun("file-open", cmdFileOpen)
    efun("file-save", cmdFileSave)
    efun("choose-dir", cmdChooseDir)
    efun("use-browser", cmdUseBrowser)
    efun("loglevel", cmdLogLevel)
    else {
        WebWireHandler *h = this;
        r_err(asprintf("Unknown command '%s'", cmd.c_str()));
        r_nok(cmd + ":unknown:Unknown command");
    }
}

void WebWireHandler::event(Event_t msg)
{
    //std::cout << "WEBWIREHANDLER:" << msg.event() << msg.seqNr() << "\n";
    if (msg.is_a(id_timeout)) {
        handleTimer(msg);
    } else if (msg.is_a(id_readline_have_line)) {
        std::string line;
        msg >> line;
        processInput(line);
    } else if (msg.is_a(id_readline_eof) || msg.is_a(id_readline_error)) {
        this->inputStopped(msg);
    } else if (msg.is_a(id_handler_log)) {
        FILE *std_f;
        const char *kind;
        std::string m;
        msg >> std_f;
        msg >> kind;
        msg >> m;
        if (_log_handler != nullptr && _evt_handler != nullptr) {
            if (strcmp(kind, "EVENT") == 0) {
                _evt_handler(m.c_str(), _user_data);
            } else {
                _log_handler(kind, m.c_str(), _user_data);
            }
        } else {
            log(std_f, _log_fh, kind, m.c_str());
        }
    }
    Object_t::event(msg);
}

void WebWireHandler::addErr(const std::string &msg)
{
    _reasons.push_back(msg);
}

void WebWireHandler::addOk(const std::string &msg)
{
    _responses.push_back("OK:" + msg);
}

void WebWireHandler::addNOk(const std::string &msg)
{
    _responses.push_back("NOK:" + msg);
}

void WebWireHandler::msg(const std::string &msg)
{
    message(msg);
}

std::stringlist WebWireHandler::splitArgs(std::string l)
{
    int from = 0;
    int i, N;
    bool in_str = false;
    from = 0;
    std::stringlist r;

    auto append = [this, &r](const std::string &s) {
        r.append(s);
        //message(std::string("Appending: ") + s);
    };
\
    bool prev_escape = false;
    for(i = 0, N = l.length(); i < N; ) {
        if (std::isspace(l[i]) && !in_str) {
            append(l.substr(from, i - from));
            while (i < N && std::isspace(l[i])) { i++; }
            from = i;
        } else if (l[i] == '\"') {
            if (in_str) {
                if (!prev_escape) {
                    std::string s = replace(l.substr(from, i - from), "\\\"", "\"");
                    append(s);
                    i += 1;
                    while (i < N && std::isspace(l[i])) { i++; }
                    from = i;
                    in_str = false;
                } else {
                    i++;
                    prev_escape = false;
                }
            } else {
                in_str = true;
                i++;
                from = i;
            }
        } else if (l[i] == '\\') {
            if (in_str) { prev_escape = true; }
            i++;
        } else {
            if (in_str) { prev_escape = false; }
            i++;
        }
    }
    if (from != N) {
        append(l.substr(from));
    }

    /*{
        int i;
        for(i = 0; i < r.size(); i++) {
            msg(r[i]);
        }
    }*/

    return r;
}

WebWireLogLevel_t WebWireHandler::logLevel()
{
    return _min_log_level;
}

void WebWireHandler::setLogLevel(WebWireLogLevel_t l)
{
    _min_log_level = l;
}

void WebWireHandler::processInput(const std::string &line, std::string *ok_msg)
{
    _reasons.clear();
    _responses.clear();

    std::string l = trim_copy(line);
    std::stringlist expr = splitArgs(l);

    //if (!expr.empty() && expr.last() == "") {
    //    expr.removeLast();
    //}
    if (expr.size() > 0) {
        std::string cmd = lcase(expr.front());
        expr.pop_front();
        processCommand(cmd, expr);
    } else {
        _reasons.append("Does not compute");
        _responses.append("NOK:" + l);
    }

    if (_reasons.size() > 0) {
        std::stringlist::iterator it = _reasons.begin();
        for(; it != _reasons.end(); it++) {
            std::string msg = *it;
            error(msg);
        }
    }

    if (_responses.size() > 0) {
        std::string msg = _responses.join(", ");
        if (ok_msg != nullptr) {
            *ok_msg = msg;
        } else {
            if (msg.rfind("OK:", 0) == 0) {
                emit(evt_handler_log << stdout << "OK" << msg.substr(3));
            } else if (msg.rfind("NOK:", 0) == 0) {
                emit(evt_handler_log << stdout << "NOK" << msg.substr(4));
            } else {
                emit(evt_handler_log << stdout << "OK" << msg);
            }
        }
    }
}

void WebWireHandler::inputStopped(const Event_t &e)
{
    FILE *ff = nullptr;
    emit(evt_handler_log << ff << "Unexpected:%s\n" << std::string("Input has stopped"));
    //log(nullptr, _log_fh, "Unexpected:%s", "Input has stopped");
    closeListener();
    doQuit();
}

bool WebWireHandler::getArgs(std::string cmd, int win, wwlist<Var> types, std::stringlist args)
{
    auto get_min_arg_count = [types]() {
        wwlist<Var>::const_iterator it;
        int i;
        for(i = 0, it = types.begin(); it != types.end() && !(*it).optional; it++, i++);
        return i;
    };

    auto mkerr = [this, cmd, types, args](std::string msg) {
        addErr(cmd + asprintf(": ") + msg);
        std::string vl = cmd;
        wwlist<Var>::const_iterator it;
        for(it = types.begin(); it != types.end(); it++) {
            if (it->optional) vl += " [";
            else vl += " <";
            vl += it->name + ":";
            switch(it->type) {
            case t_string: vl += "string"; break;
            case t_int: vl += "integer"; break;
            case t_bool: vl += "boolean"; break;
            case t_double: vl += "real"; break;
            case t_url: vl += "url"; break;
            case t_json_string: vl += "json-string"; break;
            default: vl += "<undef>"; break;
            }
            if (it->optional) vl += "]";
            else vl += ">";
        }
        std::stringlist::const_iterator a_it;
        std::string gl = cmd;
        for(a_it = args.begin(); a_it != args.end(); a_it++) {
            gl += " ";
            gl += *a_it;
        }

        addErr("syntax: " + vl);
        addErr("got   : " + gl);
    };

    if (args.size() < get_min_arg_count()) {
        mkerr(asprintf(": incorrect number of arguments %lld, minimal expected %d", args.size(), get_min_arg_count()));
        addNOk(cmd + asprintf(":%d", win));
        return false;
    }
    int i;
    int N = args.size();
    wwlist<Var>::iterator it;
    std::stringlist::iterator a_it;
    for(i = 0, it = types.begin(), a_it = args.begin(); it != types.end(); it++, i++) {
        VarType t = (*it).type;
        if (t == t_int) {
            bool ok = true;
            *(*it).i = (i < N) ? toInt((*a_it), &ok) : (*it).d_i;
            if (!ok) {
                mkerr((*it).name + ": expected integer, got " + *a_it);
                addNOk(cmd + asprintf(":%d", win));
                return false;
            }
        } else if (t == t_string) {
            *((*it).s) = (i < N) ? *a_it : (*it).d_s;
        } else if (t == t_url) {
            url u = (i < N) ? url(*a_it) : (*it).d_u;
            if (!u.is_valid()) {
                mkerr((*it).name + ": url expected, got " + *a_it);
                addNOk(cmd + asprintf(":%d", win));
                return false;
            }
            *((*it).u) = u;
        } else if (t == t_json_string) {
            std::string s = (i < N) ? *a_it : (*it).d_s;
            json j;
            try {
                j = json::parse(s);
            } catch(const json::parse_error &e) {
                mkerr((*it).name + ": Json Parse Error '" + e.what() + "'");
                addNOk(cmd + asprintf(":%d", win));
                return false;
            }
            *((*it).s) = s;
        } else if (t == t_bool) {
            bool ok = true;
            *(*it).b = (i < N) ? toBool((*a_it), &ok) : (*it).d_b;
            if (!ok) {
                mkerr((*it).name + ": expected boolean, got " + *a_it);
                addNOk(cmd + asprintf(":%d", win));
                return false;
            }
        }

        if (i < N) { a_it++; }
    }

    return true;
}

void WebWireHandler::log(FILE *fh, FILE *log_fh, const char *kind, const char *msg)
{
    int i, N, nl;
    for(i = 0, N = strlen(msg), nl = 1; i < N; i++) {
        if (msg[i] == '\n') nl++;
    }

    const char *format = "%s(%d):%s\n";
    if (log_fh != nullptr) {
        fprintf(log_fh, format, kind, nl, msg);
        fflush(log_fh);
    }

    if (fh != nullptr) {
        fprintf(fh, format, kind, nl, msg);
        fflush(fh);
    }
}

void WebWireHandler::error(const std::string &msg)
{
    if (_min_log_level <= WebWireLogLevel_t::error) {
        emit(evt_handler_log << stderr << "ERR" << msg );
    }
}

void WebWireHandler::evt(const std::string &msg)
{
    emit(evt_handler_log << stderr << "EVENT" << msg);
}

void WebWireHandler::message(const std::string &msg)
{
    if (_min_log_level <= WebWireLogLevel_t::info) {
        emit(evt_handler_log << stderr << "MSG" << msg);
    }
}

void WebWireHandler::warning(const std::string &msg)
{
    if (_min_log_level <= WebWireLogLevel_t::warning) {
        emit(evt_handler_log << stderr << "WARN" << msg);
    }
}

void WebWireHandler::debug(const std::string &msg)
{
    if (_min_log_level <= WebWireLogLevel_t::debug) {
        emit(evt_handler_log << stderr << "DBG" << msg);
    }
}

void WebWireHandler::debugDetail(const std::string &msg)
{
    if (_min_log_level <= WebWireLogLevel_t::debug_detail) {
        emit(evt_handler_log << stderr << "DBG" << msg);
    }
}

void WebWireHandler::closeListener()
{
    //_listener->close();
}

void WebWireHandler::doQuit()
{
    wwlist<int> wins = _windows.keys();
    wwlist<int>::iterator it;
    for(it = wins.begin(); it != wins.end(); it++) {
        //windowCloses(*it, true);
        closeWindow(*it);
    }
    //webui_wait();   // After closing al windows, we wait untill it's done
    _app->quit();
}

void WebWireHandler::setStylesheet(const std::string &css)
{
    setDefaultCss(css);
    wwlist<int> wins = _windows.keys();
    wwlist<int>::iterator it;
    for(it = wins.begin(); it != wins.end(); it++) {
        int win = *it;
        WinInfo_t *i = getWinInfo(win);
        i->profile->set_css(this, win, css);
    }
}

std::string WebWireHandler::getStylesheet()
{
    return defaultCss();
}


static std::string pid()
{
#ifdef _WINDOWS
    int _p = _getpid();
#else
    int _p = getpid();
#endif
    unsigned long long p = _p;
    return asprintf("%llu", p);
}

WebWireHandler::WebWireHandler(Application_t *app, int argc, char *argv[],
                               void (*log_handler)(const char *, const char *, void *),
                               void (*evt_handler)(const char *, void *),
                               void *user_data)
    : Object_t(), _log_handler(log_handler), _evt_handler(evt_handler), _user_data(user_data)
{
    connect(this, id_handler_log, this);    // handle log events in the main thread

    _app = app;

    _window_nr = 0;
    _code_handle = 0;
    _min_log_level = WebWireLogLevel_t::debug;

    std::error_code ec;
    std::filesystem::path tmp_dir = std::filesystem::temp_directory_path(ec);
    std::filesystem::path wr_dir = tmp_dir.append("web-ui-wire");
    std::filesystem::path pid_dir = wr_dir.append(pid());
    _my_dir = std::filesystem::absolute(pid_dir);
    FileInfo_t fi(_my_dir);
    if (!fi.exists()) {
        fi.mkPath();
    }

    std::string log_file = _my_dir.append("webracket.log").string();
#ifdef _WINDOWS
    _log_fh = _fsopen(log_file.c_str(), "wt", _SH_DENYNO);
#else
    _log_fh = fopen(log_file.c_str(), "wt");
#endif

    //_server = nullptr;
    //_server = new HttpServer_t(this, this);
    //connect(_server, id_httpserver_log, this);
    //_server->start();
    //bool listens = _server->listens();
    //if (!listens) {
    //    _port = -1;
    //    _webui_port = -1;
    //} else {
    //    _port = _server->port(); //tcp_server->serverPort();
    //    _webui_port = _server->webuiPort();
    //}
    bool listens = true;

    msg(std::string("Web UI Wire - v") + WEB_WIRE_VERSION + " - " + WEB_WIRE_COPYRIGHT + " - " + WEB_WIRE_LICENSE);
    msg("Web Wire file store: " + std::filesystem::absolute(_my_dir).string());
    msg("Web Wire log file: " + log_file);

    if (!listens) {
        evt("no-http-service");
    } else {
        msg(asprintf("Web Wire Http Server on http://127.0.0.1:%d", _port));
    }

    //_server->router()->clearConverters();
    //_server->router()->addConverter(QMetaType(QMetaType::std::string), ".*");
}

WebWireHandler::~WebWireHandler()
{
    std::list<AtDelete_t *>::const_iterator it;
    for(it = _to_inform_at_delete.begin(); it != _to_inform_at_delete.end(); it++) {
        (*it)->deleteInProgress("WebWireHandler");
    }

    std::error_code ec;
    std::filesystem::path tmp_dir = std::filesystem::temp_directory_path(ec);
    std::filesystem::path wr_dir = tmp_dir.append("web-ui-wire");
    std::filesystem::path pid_dir = wr_dir.append(pid());
    std::filesystem::path last = wr_dir.append("last");
    last = std::filesystem::absolute(last);

    FileInfo_t fi_last(last);
    if (fi_last.exists()) {
        fi_last.removeRecursively();
    }

    std::filesystem::path from_dir = std::filesystem::absolute(_my_dir);
    std::filesystem::path to_dir = last;

    msg("my dir   = " + from_dir.string());
    msg("last dir = " + to_dir.string());

    fclose(_log_fh);

    std::filesystem::rename(from_dir, to_dir, ec);
}


void WebWireHandler::removeAtDelete(AtDelete_t *obj)
{
    _to_inform_at_delete.remove(obj);
}

void WebWireHandler::addAtDelete(AtDelete_t *obj)
{
    _to_inform_at_delete.push_back(obj);
}

void WebWireHandler::windowCloses(int win, bool do_close)
{

    WebUIWindow *w = getWindow(win);
    if (w != nullptr) {
        Timer_t *t = _timers[win];
        WinInfo_t *i = _infos[win];

        _windows.erase(win);
        _timers.erase(win);
        _infos.erase(win);

        if (do_close) {
            //w->dontCallback();
            w->setClosing(true);
            w->close();
        }

        delete t;
        delete w;

        WebWireProfile *p = i->profile;
        if (p != nullptr) {
            if (p->usage() == 1) {  // usage will drop to 0 when i is deleted.
                _profiles.erase(p->profileName());
            }
        }
        delete i;   // delete i after w, because otherwise the WebEnginProfile gets deleted before the WebEnginePage.

        evt(asprintf("closed:%d", win));

        // If no windows left, call webui_clean.
        if (_windows.empty()) {
           // webui_clean();
        }
    }

//    evt(asprintf("closed:%d", win));
}

void WebWireHandler::requestClose(int win)
{
    evt(asprintf("request-close:%d", win));
}

void WebWireHandler::windowResized(int win, int w, int h)
{
    // We get these resizes from javascript and they are
    // "cleaned", i.e. will trigger not often
    WinInfo_t *i = _infos[win];
    i->size = Size_t(w,h);
    json j;
    j["width"] = i->size.width();
    j["height"] = i->size.height();
    evt(asprintf("resized:%d:%s", win, j.dump().c_str()));
}

void WebWireHandler::windowMoved(int win, int x, int y)
{
    WinInfo_t *i = _infos[win];
    i->pos = Point_t(x, y);
    json j;
    j["x"] = i->pos.x();
    j["y"] = i->pos.y();
    evt(asprintf("moved:%d:%s", win, j.dump().c_str()));
}

void WebWireHandler::handleTimer(Event_t e)
{
    std::string timer_name;
    e >> timer_name;

    Timer_t *t = static_cast<Timer_t *>(e.sender());
    int win = t->property("win").toInt();

    message(asprintf("Window %d: Timer %s fired (timeout ms = %d), check if (still) disconnected",
                    win,
                    timer_name.c_str(),
                    t->interval()
                     ));

    WebUIWindow *w = getWindow(win);
    if (w != nullptr) {
        // Check if we're still disconnected
        if (w->disconnected()) {
            closeWindow(win);
        }
    }

}

int WebWireHandler::newWindow(const std::string &profile, bool in_browser, int parent_win_id)
{
    ++_window_nr;

    Timer_t *t = new Timer_t("window-close-timer");
    _timers[_window_nr] = t;
    t->setProperty("win", _window_nr);
    t->setSingleShot(true);
    connect(t, id_timeout, this);

    WinInfo_t *i = new WinInfo_t();
    _infos[_window_nr] = i;

    i->app_name = profile;

    static std::regex re_ws("\\s+");
    std::string app_internal_profile = std::regex_replace(trim_copy(profile), re_ws, "_");
    //i->base_url = asprintf("http://127.0.0.1:%d/", _port) + app_internal_profile + "/";

    if (_profiles.contains(app_internal_profile)) {
        WebWireProfile *p = _profiles[app_internal_profile];
        p->incUsage();
        i->profile = p;
    } else {
        WebWireProfile *p = new WebWireProfile(app_internal_profile, defaultCss(), this);
        p->incUsage();
        i->profile = p;
    }

    WebUIWindow *parent_win = nullptr;
    if (parent_win_id > 0) {
        parent_win = getWindow(parent_win_id);
    }

    WebUIWindow *w = new WebUIWindow(this, _window_nr, profile, in_browser, parent_win, this);
    _windows[_window_nr] = w;

    return _window_nr;
}

WebUIWindow *WebWireHandler::getWindow(int win)
{
    if (_windows.contains(win)) {
        WebUIWindow *w = _windows[win];
        return w;
    } else {
        _reasons.append(asprintf("Window %d not found", win));
       return nullptr;
    }
}

WinInfo_t *WebWireHandler::getWinInfo(int win)
{
    if (_infos.contains(win)) {
        return _infos[win];
    } else {
        _reasons.append(asprintf("Windows Info for window %d not there, unexpected!", win));
        return nullptr;
    }
}

Timer_t *WebWireHandler::getTimer(int win)
{
    if (_timers.contains(win)) {
        return _timers[win];
    } else {
        _reasons.append(asprintf("Timer for window %d not there, unexpected!", win));
        return nullptr;
    }
}

std::string WebWireHandler::serverUrl()
{
    return asprintf("http://127.0.0.1:%d/", _port);
}

int WebWireHandler::serverPort()
{
    return _port;
}

int WebWireHandler::webuiPort()
{
    return _port + 1;
}

bool WebWireHandler::closeWindow(int win)
{
    WebUIWindow *w = getWindow(win);
    if (w != nullptr) {
        Timer_t *t = getTimer(win);
        t->stop();
        w->setClosing(true);
        w->close();
        windowCloses(win);
    }
    return w != nullptr;
}

void WebWireHandler::debugWin(int win)
{
    /*WebUIWindow *w = getWindow(win);
    WebWireView *v = w->view();
    WebWirePage *p = v->page();

    DevToolsWindow *devtools_win = new DevToolsWindow();
    p->setDevToolsPage(devtools_win->page());
    devtools_win->show();
    */
}

bool WebWireHandler::moveWindow(int win, int x, int y)
{
    WebUIWindow *w = getWindow(win);
    if (w != nullptr) w->move(x, y);
    return w != nullptr;
}

bool WebWireHandler::resizeWindow(int win, int width, int height)
{
    WebUIWindow *w = getWindow(win);
    if (w != nullptr) w->resize(width, height);
    return w != nullptr;
}

bool WebWireHandler::setWindowTitle(int win, const std::string &title)
{
    WebUIWindow *w = getWindow(win);
    if (w != nullptr) w->setWindowTitle(title);
    return w != nullptr;
}

bool WebWireHandler::setWindowIcon(int win, const std::string &icn_file)
{
    WebUIWindow *w = getWindow(win);
    if (w != nullptr) w->setWindowIcon(icn_file);
    return w != nullptr;
}

#define m_chk(cond, msg) \
   if (!(cond)) { \
      addErr(asprintf("set-menu:%d:", win) + msg); \
      return false; \
   }

/*
bool WebWireHandler::makeSubMenu(int win, QMenu *m, QJsonArray &a)
{
    int i;
    for(i = 0; i < a.count(); i++) {
        m_chk(a[i].isArray(), "Expected menu entry or (sub) menu");

        QJsonArray entry = a[i].toArray();
        m_chk(entry.count() == 2, "Entry consists of a name + action-id or submenu");

        std::string name = entry[0].toString().trimmed();
        m_chk(name != "", "Empty menu entry name");

        if (entry[1].isArray()) {
            QMenu *submenu = m->addMenu(name);
            QJsonArray sm_a = entry[1].toArray();
            if (!makeSubMenu(win, submenu, sm_a)) {
                return false;
            }
        } else {
            std::string id = entry[1].toString();
            QAction *a = m->addAction(name);
            a->setData(asprintf("menu-item-choosen:%d:", win) + id);
            connect(a, &QAction::triggered, this, [this](bool checked) {
                QAction *a = qobject_cast<QAction *>(sender());
                std::string event = a->data().toString();
                evt(event);
            });
        }
    }

    return true;
}

bool WebWireHandler::makeMenuBar(int win, QMenuBar *b, QJsonArray &a)
{
    int i;
    for(i = 0; i < a.count(); i++) {
        m_chk(a[i].isArray(), "Expected (sub)menu");

        QJsonArray entry = a[i].toArray();
        m_chk(entry.count() == 2, "Entry consists of name + submenu");

        std::string name = entry[0].toString().trimmed();
        m_chk(name != "", "Empty menu-bar name for entry");
        m_chk(entry[1].isArray(), "A top level menu entry must contain a name and a list of menu items");

        QMenu *m = b->addMenu(name);
        QJsonArray m_a = entry[1].toArray();
        if (!makeSubMenu(win, m, m_a)) {
            return false;
        }
    }

    return true;
}
*/

#undef m_chk

bool WebWireHandler::setMenu(int win, const std::string &menu)
{
    ExecJs js(this, win, "set-menu", true);
    js.run("window._web_wire_menu(JSON.parse('" + ExecJs::esc_quote(menu) + "'));");
    return true;
}

void WebWireHandler::setShowState(int win, const std::string &state)
{
    WebUIWindow *w = getWindow(win);

    WebUiWindow_ShowState st;

    if (state == "minimized") { st = minimized; }
    else if (state == "maximized") { st = maximized; }
    else if (state == "normal") { st = normal; }
    else if (state == "shown") { st = shown; }
    else if (state == "hidden") { st = hidden; }

    w->setShowState(st);
}

std::string WebWireHandler::showState(int win)
{
    WebUIWindow *w = getWindow(win);
    int v = w->showState();
    if (v & WebUiWindow_ShowState::shown) {
        v -= WebUiWindow_ShowState::shown;
        if (v == WebUiWindow_ShowState::minimized) {
            return "minimized";
        } else if (v == WebUiWindow_ShowState::maximized) {
            return "maximized";
        } else {
            return "normal";
        }
    } else {
        return "hidden";
    }
}


std::string WebWireHandler::fileOpen(int win, const std::string &title, const std::string &dir, const std::string &file_types, bool &ok)
{
    WebUIWindow *w = getWindow(win);
    WebWireStandardDialogs dlgs;

    PathFilterList filters = dlgs.filtersFromString(file_types);

    bool cancelled;
    std::string fn = dlgs.openFileDialog(this, w, title, dir, filters, cancelled);

    if (cancelled) {
        ok = false;
    } else {
        ok = true;
    }

    return fn;
}

std::string WebWireHandler::fileSave(int win, const std::string &title, const std::string &dir, const std::string &file_types, bool overwrite, bool &ok)
{
    WebUIWindow *w = getWindow(win);
    WebWireStandardDialogs dlgs;

    PathFilterList filters = dlgs.filtersFromString(file_types);

    bool cancelled;
    std::string fn = dlgs.saveFileDialog(this, w, title, dir, filters, cancelled);

    if (cancelled) {
        ok = false;
    } else {
        ok = true;
    }

    return fn;
}

std::string WebWireHandler::chooseDir(int win, const std::string &title, const std::string &dir, bool &ok)
{
    WebUIWindow *w = getWindow(win);
    WebWireStandardDialogs dlgs;

    bool cancelled;
    std::string fn = dlgs.getDirectoryDialog(this, w, title, dir, cancelled);

    if (cancelled) {
        ok = false;
    } else {
        ok = true;
    }

    return fn;
}

void WebWireHandler::execJs(int win, const std::string &code, bool &ok, std::string &result, std::string tag)
{
    ExecJs e(this, win, tag, false);

    ok = false;
    result = e.call(code, ok);
}

void WebWireHandler::execJs(int win, const std::string &code, std::string tag)
{
    ExecJs e(this, win, tag, true);
    e.run(code);
}

//WebWireView *WebWireHandler::getView(int win)
//{
    /*TODO
     * WebUIWindow *w = getWindow(win);
    if (w != nullptr) { return w->view(); }
    else { return nullptr; }
*/
//    return nullptr;
//}

void WebWireHandler::start()
{
    msg(asprintf("protocol-version: %d", WEB_WIRE_PROTOCOL_VERSION));
}

WinInfo_t::WinInfo_t() {
    size_set = false;
    pos_set = false;
    profile = nullptr;
}

WinInfo_t::~WinInfo_t() {
    if (profile != nullptr) {
        profile->decUsage();
    }
}
