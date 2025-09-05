#include "application_t.h"
#include <iostream>

#include "object_t.h"

Application_t *Application_t::_current_app = nullptr;

std::string Application_t::sourceKey(Object_t *source, std::string event)
{
    char buffer[1024];
    sprintf_s(buffer, 1024, "%p:%s", source, event.c_str());
    std::string key(buffer);
    return key;
}

void Application_t::setHandler(WebWireHandler *h)
{
    _handler = h;
}

WebWireHandler *Application_t::handler()
{
    return _handler;
}

EventQueue_t &Application_t::evtQueue()
{
    return _evt_queue;
}

void Application_t::addRoute(Object_t *source, Object_t *destination, std::string for_event_kind)
{
    std::string key = sourceKey(source, for_event_kind);
    ObjectRoutes_t r;
    if (_routes.contains(key)) {
        r = _routes[key];
    } else {
        r.source = source;
        r.event = for_event_kind;
    }
    std::list<Object_t *> dests = r.destinations;
    std::list<Object_t *>::iterator dests_it = dests.begin();
    bool not_found = true;
    while(dests_it != dests.end() && not_found) {
        if (*dests_it == destination) { not_found = false; }
        dests_it++;
    }
    if (not_found) { dests.push_back(destination); }
    r.destinations = dests;
    _routes[key] = r;

    std::list<std::string> dkeys;
    if (_keys_for_destinations.contains(destination)) {
        dkeys = _keys_for_destinations[destination];
    }
    dkeys.push_back(key);
    _keys_for_destinations[destination] = dkeys;

    std::list<std::string> keys;
    if (_keys_for_sources.contains(source)) {
        keys = _keys_for_sources[source];
    }
    keys.push_back(key);
    _keys_for_sources[source] = keys;
}

void Application_t::delRoute(Object_t *source, Object_t *destination, std::string for_event_kind)
{
    std::string key = sourceKey(source, for_event_kind);
    if (_routes.contains(key)) {
        ObjectRoutes_t r = _routes[key];
        std::list<Object_t *> dests = r.destinations;
        std::list<Object_t *>::iterator dests_it = dests.begin();
        std::list<Object_t *> ndests;
        while(dests_it != dests.end()) {
            if (*dests_it != destination) {
                ndests.push_back(destination);
            }
            dests_it++;
        }
        r.destinations = ndests;
        if (ndests.size() == 0) {
            _routes.erase(key);
        } else {
            _routes[key] = r;
        }

        std::list<std::string> dkeys = _keys_for_destinations[destination];
        std::list<std::string> ndkeys;
        std::list<std::string>::iterator dit = dkeys.begin();
        while(dit != dkeys.end()) {
            std::string k = *dit;
            if (k != key) {
                ndkeys.push_back(k);
            }
            dit++;
        }
        _keys_for_destinations[destination] = ndkeys;

        std::list<std::string> keys = _keys_for_sources[source];
        std::list<std::string> nkeys;
        std::list<std::string>::iterator it = keys.begin();
        while(it != keys.end()) {
            std::string k = *it;
            if (k != key) {
                nkeys.push_back(k);
            }
            it++;
        }
        _keys_for_sources[source] = nkeys;

    } else {
        std::cerr << "ERR:Unexpected! no route for given source object " << key;
    }
}

void Application_t::delObject(Object_t *obj)
{
    std::list<std::string> keys;
    if (_keys_for_sources.contains(obj)) {
        keys = _keys_for_sources[obj];
    } else if (_keys_for_destinations.contains(obj)) {
        keys = _keys_for_destinations[obj];
    } else {
        std::cerr << "ERR:Unexpected! no source object found for obj " << obj;
        return;
    }

    std::list<std::string>::iterator it = keys.begin();
    while(it != keys.end()) {
        std::string k = *it;
        it++;
        if (_routes.contains(k)) {
            ObjectRoutes_t r = _routes[k];

            Object_t *source = r.source;
            std::string event = r.event;

            std::list<Object_t *> dests = r.destinations;
            std::list<Object_t *>::iterator dests_it = dests.begin();
            std::list<Object_t *> tmp_dests;
            while(dests_it != dests.end()) {
                tmp_dests.push_back(*dests_it);
                dests_it++;
            }

            dests_it = tmp_dests.begin();
            while(dests_it != tmp_dests.end()) {
                delRoute(source, *dests_it, event);
                dests_it++;
            }
        } else {
            std::cerr << "ERR:Unexcpected! no route found for key " << k;
        }
    }
}

Application_t *Application_t::current()
{
    return _current_app;
}

void Application_t::quit()
{
    Event_t e(id_app_quit, nullptr);
    current()->evtQueue().enqueue(e);
}

void Application_t::exec()
{
    _evt_queue.setWait(500);
    Event_t msg = _evt_queue.dequeue();
    while(!msg.is_a(id_app_quit)) {
        if (!msg.is_a(id_evt_null)) {
            Object_t *sender = msg.sender();
            std::string event = msg.event();
            std::string key = sourceKey(sender, event);

            if (_routes.contains(key)) {
                ObjectRoutes_t r = _routes[key];
                std::list<Object_t *>::iterator dests_it = r.destinations.begin();
                while(dests_it != r.destinations.end()) {
                    Object_t *receiver = *dests_it;
                    receiver->event(msg);
                    dests_it++;
                }
            } else {
                std::cerr << "ERR:Unexpected! no route found for key " << key << "\n";
            }
        }

        msg = _evt_queue.dequeue();
    }

    // TODO: Maye cleanup all connections and objects?
}

Application_t::Application_t()
{
    if (_current_app != nullptr) {
        std::cerr << "There can be only one instantiated application object.\n";
        exit(1);
    }
    _current_app = this;
}

Application_t::~Application_t()
{
    _current_app = nullptr;
}
