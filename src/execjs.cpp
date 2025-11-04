#include "execjs.h"

#include "webuiwindow.h"
#include "webwirehandler.h"
#include "misc.h"
#include "webui_utils.h"
#include "json.h"

#ifdef __APPLE__
#include "apple_utils.h"
#endif

using namespace json;

static std::string makeResult(WebWireHandler *h, const Variant_t &v)
{
    std::string in = v.toString();

    h->message(std::string("makeResult:") + in);
    JSON obj;

    if (in.rfind("json:", 0) == 0) {
        JSON j;
        bool ok = true;
        auto on_error = [&ok, h, in](const std::string &msg) {
            h->error(std::string("exec-js, makeResult from '") + in + "', parse error:" + msg);
            ok = false;
        };

        j = JSON::Load(in.substr(5), on_error);
        if (ok) {
            obj["result"] = j;
        } else {
            obj["result"] = false;
        }

    } else if (in.rfind("int:", 0) == 0) {
        obj["result"] = toInt(in.substr(4));
    } else if (in.rfind("bool:", 0) == 0) {
        std::string b = lcase(in.substr(5));
        obj["result"] = (b == "true") ? true : false;
    } else if (in.rfind("float:", 0) == 0) {
        obj["result"] = toDouble(in.substr(6));
    } else {
        obj["result"] = in;
    }

    std::string d = obj.dump();
    h->message(d);
    return d;
}


std::string ExecJs::esc_quote(const std::string &s)
{
    return replace(s, "'", "\\'");
}

std::string ExecJs::esc_dquote(const std::string & s)
{
    return replace(s, "\"", "\\\"");
}

void ExecJs::run(const std::string &code)
{
    if (!_is_void) {
        _handler->warning("ExecJs:Running code that is declared non void");
    }

    if (_webui_win == 0) {
        _handler->error(asprintf("ExecJs:No WebUIWindow available for window %d to run this code in", _win));
        return;
    }

    webui_run(_webui_win, code.c_str());
}

#define MAX_JS_BUF (100 *1024)      // Max 100Kb Buffer
#define MAX_EXEC_TIME 30 //600           // 10 minutes maximum execution time

std::string ExecJs::call(const std::string &code, bool &ok)
{
    //char *buf = static_cast<char *>(malloc(MAX_JS_BUF));

    _handler->message("calling: " + code);

    // Due to gtk, we need to execute javascript asynchronous.

    std::string script = "{ "
                         "  let webui_wire_09282_f = function(r, ok, m) { window._web_wire_put_evt({ evt: 'script-result', result: r, result_ok: ok, result_msg: m }); };\n"
                         "  try {\n"
                         "    let webui_wire_93732_g = function() { " + code + "};\n"
                         "    let webui_wire_83223_r = webui_wire_93732_g();\n"
                         "    webui_wire_09282_f(webui_wire_83223_r + '', true, '');\n"
                         "  } catch (webui_wire_82282_e) {\n"
                         "    webui_wire_09282_f('', false, webui_wire_82282_e.message);\n"
                         "  }\n"
                         "}";

    //ok = webui_script(_webui_win, code.c_str(), MAX_EXEC_TIME, buf, MAX_JS_BUF);
    webui_run(_webui_win, script.c_str());

    if (_is_void) {
        _handler->error("ExecJs:Calling code that has been declared void");
    }

    if (_webui_win == 0) {
        _handler->error(asprintf("ExecJs:No WebUIWindow available for window %d to run this code in", _win));
        std::string s = asprintf("NOK:%d:%s:%s", _win, _name.c_str(), "Window not available (WebUIWindow for window gives nullptr)");
        return s;
    }

    _window->setExecJs(this);

    _result_set = false;
    WebUI_Utils u;

    WebUI_Utils::WaitResult r = u.waitUntil([this](){ return _result_set; }, MAX_EXEC_TIME * 1000);

    if (r == WebUI_Utils::wu_timeout) {
        _handler->error("ExecJs: Timeout for code " + code);
        _window->setExecJs(nullptr);
        std::string s = "";
        return s;
    }

    if (_result_ok) {
        ok = true;
        return makeResult(_handler, _result);
    } else {
        ok = false;
        _handler->error("ExecJs: Error executing " + code);
        _handler->error("ExecJs: Error message: " + _result_msg);
        std::string s = "";
        return s;
    }

    std::string s = "";
    return s;
}

void ExecJs::setResult(std::string result, bool ok, std::string msg)
{
    _result = result;
    _result_ok = ok;
    _result_msg = msg;
    _result_set = true;
}

ExecJs::ExecJs(WebWireHandler *handler, int win, std::string name, bool is_void)
{
    _is_void = is_void;
    _handler = handler;
    _win = win;
    _name = name;

    WebUIWindow *w = _handler->getWindow(_win);
    _window = w;
    if (w == nullptr) {
        _webui_win = 0;
    } else {
        _webui_win = w->webuiWin();
    }
}
\
