#ifndef WEBWIREHANDLER_H
#define WEBWIREHANDLER_H

#include "misc.h"
#include "timer_t.h"
#include "menubar_t.h"
#include "menu_t.h"
#include "object_t.h"
#include "variant_t.h"
#include "readlineinthread.h"
#include "fileinfo_t.h"
#include "event_t.h"

#undef max
#undef min
#include "url.h"
using url = upa::url;

#define id_handler_log  "handler-log-event"
#define evt_handler_log Event_t(id_handler_log, this)

class WebUIWindow;
class WebWireProfile;
class HttpServer_t;
class Application_t;

typedef VariantType_t VarType;

class Var
{
public:
    std::string  name;
    VarType      type;
    int         *i;
    bool        *b;
    std::string *s;
    url         *u;
    int          d_i;
    bool         d_b;
    std::string  d_s;
    url          d_u;
    bool         optional;
public:
    Var(VarType t, int &v, const char *vname, bool o = false, int d = 0) { i = &v; type = t; name = vname; optional = o;d_i = d; }
    Var(VarType t, bool &v, const char *vname, bool o = false, bool d = false) { b = &v; type = t; name = vname; optional = o;d_b = d; }
    Var(VarType t, std::string &v, const char *vname, bool o = false, std::string d = std::string("")) { s = &v; type = t; name = vname; optional = o;d_s = d; }
    Var(VarType t, upa::url &v, const char *vname, bool o = false, upa::url d = upa::url()) { u = &v; type = t; name = vname; optional = o;d_u = d; }
};

class WinInfo_t
{
public:
    Size_t               size;
    bool                 size_set;
    Point_t              pos;
    bool                 pos_set;
    std::string          app_name;
    WebWireProfile      *profile;
public:
    WinInfo_t();
    ~WinInfo_t();
};


typedef struct {

} WebWireCommunication_t;

class WebWireHandler : public Object_t
{
private:
    wwhash<int, WebUIWindow *>        _windows;
    wwhash<int, Timer_t *>            _timers;
    wwhash<int, WinInfo_t *>          _infos;
    wwhash<std::string, WebWireProfile *> _profiles;

    int                                 _window_nr;
    int                                 _code_handle;
    std::stringlist                     _reasons;
    std::stringlist                     _responses;
    Application_t                      *_app;
    FILE                               *_log_fh;
    std::filesystem::path               _my_dir;

    HttpServer_t                        *_server;
    int                                  _port;
    int                                  _webui_port;

    void (*_log_handler)(const char *kind, const char *msg);
    void (*_evt_handler)(const char *msg);

private:
    void log(FILE *fh, FILE *log_fh, const char *format, const char *msg);
    std::stringlist splitArgs(std::string l);

    // Object_t interface
public:
    void event(Event_t msg);

private:  // Eventing
    void inputStopped(const Event_t &e);
    void handleTimer(const Event_t &e);

public:  // Eventing
    void processInput(const std::string &line, std::string *ok_msg = nullptr);

public:
    WebWireHandler(Application_t *app, int argc, char *argv[],
                   void (*log_handler)(const char*kind, const char*msg) = nullptr,
                   void (*evt_handler)(const char*msg) = nullptr
                   );
    ~WebWireHandler();

    // WebWireWindow callbacks
public:
    void windowCloses(int win, bool do_close = false);
    void windowResized(int win, int w, int h);
    void windowMoved(int win, int x, int y);
    void requestClose(int win);

    // WebWire Command handling
public:
    int newWindow(const std::string &app_name, int parent_win_id = -1);
    bool closeWindow(int win);
    void debugWin(int win);
    bool moveWindow(int win, int x, int y);
    bool resizeWindow(int win, int w, int h);
    bool setWindowTitle(int win, const std::string &title);
    bool setWindowIcon(int win, const FileInfo_t &icn_file);
    bool setMenu(int win, const std::string &menu);
    void setShowState(int win, const std::string &state);
    std::string showState(int win);

    std::string fileOpen(int win, const std::string &title, const std::string &dir, const std::string &file_types, bool &ok);
    std::string fileSave(int win, const std::string &title, const std::string &dir, const std::string &file_types, bool overwrite, bool &ok);
    std::string chooseDir(int win, const std::string &title, const std::string &dir, bool &ok);

    void execJs(int win, const std::string &code, std::string tag = "exec-js");
    void execJs(int win, const std::string &code, bool &ok, std::string &result, std::string tag = "exec-js");

    // WebWire internal
public:
    //WebWireView *getView(int win);
    WebUIWindow *getWindow(int win);
    WinInfo_t *getWinInfo(int win);
    std::string serverUrl();
    int serverPort();
    int webuiPort();

    void addErr(const std::string &msg);
    void addOk(const std::string &msg);
    void addNOk(const std::string &msg);
    void msg(const std::string &msg);
    void message(const std::string &msg);
    void warning(const std::string &msg);

    void debug(const std::string &msg);
    void error(const std::string &msg);
    void ok(const std::string &msg);
    void evt(const std::string &msg);

    void closeListener();
    void doQuit();
    void setStylesheet(const std::string &css);
    std::string getStylesheet();

    bool getArgs(std::string cmd, int win, wwlist<Var> types, std::stringlist args);

public:
    void start();

protected:
    void processCommand(const std::string &cmd, const std::stringlist &args);

};

#endif // WEBWIREHANDLER_H
