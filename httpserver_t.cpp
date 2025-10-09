#include "httpserver_t.h"

#include "crow.h"
#include "mimetypes_t.h"
#include "webwirehandler.h"
#include <regex>
#include "webui_wire.h"
#include "webwireprofile.h"
#include "webuiwindow.h"
#include "asio/io_context.hpp"
#include "asio/ip/tcp.hpp"

static std::regex re_serve("^[/]([^/]+)[/](.*)$");
//static std::regex re_serve("^[/](.*)$");


class CustomLogger : public crow::ILogHandler {
private:
    HttpServer_t *_server;
public:
    CustomLogger(HttpServer_t *s) {
        _server = s;
    }

    void log(const std::string &message, crow::LogLevel level) {
        std::string lvl;
        switch (level) {
        case crow::LogLevel::Debug:    lvl = "debug   ";
            break;
        case crow::LogLevel::Info:     lvl = "info    ";
            break;
        case crow::LogLevel::Warning:  lvl = "warning ";
            break;
        case crow::LogLevel::Error:    lvl = "error   ";
            break;
        case crow::LogLevel::Critical: lvl = "critical";
            break;
        }

        std::string lmsg = "crow:" + lvl + ":" + message;
        _server->serverLog(lmsg);
    }

};

#ifdef _WINDOWS
#define strnicmp _strnicmp
#else
#ifdef __linux
#define strnicmp strncasecmp
#endif
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

using asio::ip::tcp;

void HttpServer_t::run_http_server()
{
    crow::SimpleApp *_app = new crow::SimpleApp();
    crow::SimpleApp &app = *_app;
    _crow_app = static_cast<void *>(_app);

    CROW_CATCHALL_ROUTE(app)([this, &app](const crow::request &r) {
        this->serverLog("Serving: " + r.url);
        //this->isValidEndpoint(reinterpret_cast<void *>(const_cast<crow::request *>(&r)));

        //auto a = r.socket_adaptor;
        //int port = a->port();
        //int pp = a->remote_endpoint().port();

        //this->serverLog("remote address:" + r.remote_ip_address + " - " + asprintf("%d", port));

        //tcp::socket s(*r.io_context);

        //asio::error_code ec;
        //auto re = s.remote_endpoint(ec);

//        auto adr = r.remote_endpoint.address();
//        auto p = r.remote_endpoint.port();
 //       auto pr = r.remote_endpoint.protocol();

        std::string u = trim_copy(r.url);

        if (u == "/") {
            std::string webui_js = asprintf("http://127.0.0.1:%d/webui.js", webuiPort());
            std::string standard_msg = "<!DOCTYPE html>\n"
                                       "<html><head><script src=\"" + webui_js + "\"></script></head>"
                                       "<body><p>Web UI Wire:" WEB_WIRE_VERSION "</p>"
                                       "<p>" WEB_WIRE_COPYRIGHT "</p>"
                                       "<p> License: " WEB_WIRE_LICENSE "</p>"
                                       "</body>"
                                       "</html>";
            return crow::response(200, standard_msg);
        }

        std::smatch m;
        std::regex_match(r.url, m, re_serve);

        //auto cr = crow::response();
        //bool b = cr.is_alive();

        if (m.empty()) {
            this->serverLog("r.url does not match re_serve");
            return crow::response(404, "invalid");
        } else {
            std::string window = m[1];
            std::string file = m[2];
            this->serverLog("serving " + file + " for window " + window);

            int win = toInt(window);
            WinInfo_t *i = _handler->getWinInfo(win);
            WebUIWindow *w = _handler->getWindow(win);
            std::string profile_scripts;
            if (i != nullptr) {
                profile_scripts = i->profile->scriptsTag();
            }

            std::string webui_js = asprintf("http://127.0.0.1:%d/webui.js", w->webuiPort());
            trim(file);
            if (file == "") {
                std::string standard_msg = "<!DOCTYPE html>\n"
                                           "<html>"
                                           "<head>"
                                           "<script src=\"" + webui_js + "\"></script>" +
                                           profile_scripts +
                                           "<title>Web UI Wire: " WEB_WIRE_VERSION "</title>"
                                           "</head>"
                                           "<body><p>Web UI Wire:" WEB_WIRE_VERSION "</p>"
                                           "<p>" WEB_WIRE_COPYRIGHT "</p>"
                                           "<p> License: " WEB_WIRE_LICENSE "</p>"
                                           "<p> Window: " + window + "</p>"
                                           "</body>"
                                           "</html>";
                return crow::response(200, standard_msg);
            } else {
                FileInfo_t fi(file);
                if (fi.exists() && fi.isReadable()) {
                    size_t s = fi.size();
                    int max_search = (s > 1024) ? 1024 : s;
    #ifdef _WINDOWS
                    FILE *f;
                    fopen_s(&f, file.c_str(), "rb");
    #else
                    FILE *f = fopen(file.c_str(), "rb");
    #endif
                    char *buffer = static_cast<char *>(malloc(max_search + 1));
                    fread(buffer, max_search, 1, f);
                    buffer[max_search] = '\0';
                    std::string content_type;
                    crow::response R;
                    if (fi.ext() == "html" || fi.ext() == "htm" || isHTML(buffer, max_search)) {
                        buffer = static_cast<char *>(realloc(buffer, s + 1));
                        fread(buffer + max_search, s - max_search, 1, f);
                        fclose(f);
                        buffer[s] = '\0';

                        std::string resp(buffer);
                        free(buffer);
                        resp = replace(resp, "<head>", "<head><script src=\"" + webui_js + "\"></script>" + profile_scripts);
                        content_type = "text/html";
                        R.code = 200;
                        R.set_header("Content-Type", content_type);
                        R.body = resp;
                    } else {
                        fclose(f);
                        free(buffer);
                        std::string ext = fi.ext();
                        MimeTypes_t types(_handler);
                        content_type = types.mimetypeByExt(ext);
                        R.code = 200;
                        R.set_header("Content-Type", content_type);
                        R.set_static_file_info_unsafe(file);
                    }
                    return R;
                } else {
                    return crow::response(404, "<!DOCTYPE html>\n"
                                               "<html>"
                                               "<head>"
                                               "<script src=\"" + webui_js + "\"></script>" +
                                               profile_scripts +
                                               "</head>"
                                               "<body><p>Web UI Wire:" WEB_WIRE_VERSION "</p>"
                                               "<p>Not found: " + file + "</p>"
                                               "</body>"
                                               "</html>"
                                          );

                }
            }
        }
    });

    app.bindaddr("127.0.0.1").port(_port).run();

    this->crowStopped();
}

void HttpServer_t::serverLog(std::string msg)
{
    emit(evt_httpserver_log << msg);
}

bool HttpServer_t::isValidEndpoint(void *crow_req)
{
    crow::request *req = static_cast<crow::request *>(crow_req);

    //req->remote_ip_address
    //    req->
    //const asio::io_context *c = req->io_context;
    //asio::io_context &context = *(req->io_context);
    //asio::ip::tcp::socket s(context);
    //asio::ip::tcp::endpoint p = s.remote_endpoint();
    //asio::ip::address adr = p.address();
    //asio::ip::port_type port = p.port();
    //serverLog(std::string("request from address: " + adr.to_string() + ":" + asprintf("%d", port)));
    return true;
}

void HttpServer_t::start()
{
    _crow_app = nullptr;
    _handler->message(asprintf("Starting web server on localhost, port %d", _port));
    _crow_thread = new std::thread([this]() {
        run_http_server();
    });
    setThreadName(_crow_thread, "Crow");
    while(_crow_app == nullptr) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    crow::SimpleApp *_app = static_cast<crow::SimpleApp *>(_crow_app);
    crow::SimpleApp &app = *_app;
    app.wait_for_server_start(std::chrono::milliseconds(1000));
    _port = app.port();
}

bool HttpServer_t::listens()
{
    return _crow_thread != nullptr;
}

int HttpServer_t::port()
{
    return _port;
}

int HttpServer_t::webuiPort()
{
    return _port + 1;
}

void HttpServer_t::crowStopped()
{
    delete _crow_thread;
    _crow_thread = nullptr;
}

HttpServer_t::HttpServer_t(WebWireHandler *h, Object_t *parent)
    : Object_t(parent)
{
    _logger = new CustomLogger(this);
    crow::logger::setHandler(_logger);
    _crow_thread = nullptr;
    _port = 0; //_next_port++;
    _handler = h;
}

HttpServer_t::~HttpServer_t()
{
    delete _logger;
}
