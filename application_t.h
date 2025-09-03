#ifndef APPLICATION_T_H
#define APPLICATION_T_H

#include "eventqueue_t.h"
#include <map>
#include <string>
#include <list>

class Object_t;
class WebWireHandler;

#define id_app_quit     "application-quit"
#define evt_app_quit    Event_t(id_app_quit, this)

typedef struct {
    Object_t                *source;
    std::list<Object_t *>    destinations;
    std::string              event;
} ObjectRoutes_t;

class Application_t
{
private:
    static Application_t *_current_app;

private:
    EventQueue_t _evt_queue;

private:
    std::map<std::string, ObjectRoutes_t>         _routes;
    std::map<Object_t *, std::list<std::string>>  _keys_for_destinations;
    std::map<Object_t *, std::list<std::string>>  _keys_for_sources;
    std::string sourceKey(Object_t *source, std::string kind);

    WebWireHandler *_handler;

public:
    void setHandler(WebWireHandler *h);
    WebWireHandler *handler();

public:
    EventQueue_t &evtQueue();

public:
    void addRoute(Object_t *source, Object_t *destination, std::string for_event_kind);
    void delRoute(Object_t *source, Object_t *destination, std::string for_event_kind);
    void delObject(Object_t *obj);

public:
    static Application_t *current();

public:
    void quit();

public:
    void exec();

public:
    Application_t();
};

#endif // APPLICATION_T_H
