#ifndef WEBWIREHANDLER_H
#define WEBWIREHANDLER_H

#include "misc.h"
#include "timer_t.h"
#include "object_t.h"
#include "variant_t.h"
#include "event_t.h"

#include <filesystem>

#undef max
#undef min

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
    int          d_i;
    bool         d_b;
    std::string  d_s;
    bool         optional;
public:
    Var(VarType t, int &v, const char *vname, bool o = false, int d = 0) { i = &v; type = t; name = vname; optional = o;d_i = d; }
    Var(VarType t, bool &v, const char *vname, bool o = false, bool d = false) { b = &v; type = t; name = vname; optional = o;d_b = d; }
    Var(VarType t, std::string &v, const char *vname, bool o = false, std::string d = std::string("")) {
        s = &v; type = t; name = vname; optional = o;d_s = d;
    }
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

typedef enum {
    debug_detail = 1,
    debug,
    info,
    warning,
    error,
    fatal
} WebWireLogLevel_t;


typedef struct {

} WebWireCommunication_t;

class WebWireHandler : public Object_t
{
private:
    wwhash<int, WebUIWindow *>        _windows;
    wwhash<int, Timer_t *>            _timers;
    wwhash<int, WinInfo_t *>          _infos;
    wwhash<std::string, WebWireProfile *> _profiles;
    std::list<AtDelete_t *>           _to_inform_at_delete;

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

    WebWireLogLevel_t                    _min_log_level;

    void (*_log_handler)(const char *kind, const char *msg, void *user_data);
    void (*_evt_handler)(const char *msg, void *user_data);

    void                                *_user_data;

private:
    void log(FILE *fh, FILE *log_fh, const char *format, const char *msg);
    std::stringlist splitArgs(std::string l);

public:
    void setLogLevel(WebWireLogLevel_t l);
    WebWireLogLevel_t logLevel();

    // Object_t interface
public:
    void event(Event_t msg);

private:  // Eventing
    void inputStopped(const Event_t &e);
    void handleTimer(Event_t e);

public:  // Eventing
    void processInput(const std::string &line, std::string *ok_msg = nullptr);

public:
    WebWireHandler(Application_t *app, int argc, char *argv[],
                   void (*log_handler)(const char *kind, const char *msg, void *user_data) = nullptr,
                   void (*evt_handler)(const char *msg, void *user_data) = nullptr,
                   void *user_data = nullptr
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
    int newWindow(const std::string &app_name, bool in_browser, int parent_win_id = -1);
    bool closeWindow(int win);
    void debugWin(int win);
    bool moveWindow(int win, int x, int y);
    bool resizeWindow(int win, int w, int h);
    bool setWindowTitle(int win, const std::string &title);
    bool setWindowIcon(int win, const std::string &icn_file);
    bool setMenu(int win, const std::string &menu);
    bool popupMenu(int win, const std::string &menu, int x, int y);
    void setShowState(int win, const std::string &state);
    std::string showState(int win);

    int fileOpen(int win, const std::string &title, const std::string &dir, const std::string &file_types);
    int fileSave(int win, const std::string &title, const std::string &dir, const std::string &file_types, bool overwrite);
    int chooseDir(int win, const std::string &title, const std::string &dir, int *result, std::string &out_dir);

    void execJs(int win, const std::string &code, std::string tag = "exec-js");
    void execJs(int win, const std::string &code, bool &ok, std::string &result, std::string tag = "exec-js");

    // WebWire internal
public:
    WebUIWindow *getWindow(int win);
    WinInfo_t *getWinInfo(int win);
    Timer_t *getTimer(int win);

    std::string serverUrl();
    int serverPort();
    int webuiPort();

    void addErr(const std::string &msg);
    void addOk(const std::string &msg);
    void addNOk(const std::string &msg);
    void msg(const std::string &msg);
    void message(const std::string &msg);
    void warning(const std::string &msg);

    void debugDetail(const std::string &msg);
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

public:
    void removeAtDelete(AtDelete_t *obj);
    void addAtDelete(AtDelete_t *obj);

protected:
    void processCommand(const std::string &cmd, const std::stringlist &args);

};

#endif // WEBWIREHANDLER_H
