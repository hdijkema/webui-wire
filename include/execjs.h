#ifndef EXECJS_H
#define EXECJS_H

#include <string>

class WebUIWindow;
class WebWireHandler;

class ExecJs
{
private:
    WebWireHandler *_handler;
    int             _win;
    int             _webui_win;
    WebUIWindow    *_window;
    std::string     _name;
    bool            _is_void;
    bool            _result_set;
    bool            _result_ok;
    std::string     _result;
    std::string     _result_msg;

public:
    static std::string esc_quote(const std::string &s);
    static std::string esc_dquote(const std::string &s);

public:
    void run(const std::string &code);
    std::string call(const std::string &code, bool &ok);

public:
    void setResult(std::string result, bool result_ok, std::string msg);

public:
    ExecJs(WebWireHandler *handler, int win, std::string name = "exec-js", bool is_void = false);
};

#endif // EXECJS_H
