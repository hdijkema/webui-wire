#ifndef HTTPSERVER_T_H
#define HTTPSERVER_T_H

#include "object_t.h"
#include "event_t.h"
#include <thread>

class WebWireHandler;
class CustomLogger;

#define id_httpserver_log       "httpserver-serverlog"
#define evt_httpserver_log      Event_t(id_httpserver_log, this)

class HttpServer_t : public Object_t
{
private:
    int               _port;
    std::thread      *_crow_thread;
    WebWireHandler   *_handler;
    CustomLogger     *_logger;
    void             *_crow_app;

private:
    void run_http_server();

public:
    void serverLog(std::string msg);

public:
    bool isValidEndpoint(void * crow_req);

public:
    void start();
    bool listens();
    int port();
    int webuiPort();

public:
    void crowStopped();

public:
    HttpServer_t(WebWireHandler *h, Object_t *parent = nullptr);
    ~HttpServer_t();
};

#endif // HTTPSERVER_T_H
