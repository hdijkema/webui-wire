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
    std::string     _name;
    bool            _is_void;

public:
    static std::string esc_quote(const std::string &s);
    static std::string esc_dquote(const std::string &s);

public:
    void run(const std::string &code);
    std::string call(const std::string &code, bool &ok);

public:
    ExecJs(WebWireHandler *handler, int win, std::string name = "exec-js", bool is_void = false);
};

#endif // EXECJS_H
