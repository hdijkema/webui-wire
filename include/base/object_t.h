#ifndef OBJECT_T_H
#define OBJECT_T_H

#include "webui_wire_defs.h"
#include "application_t.h"
#include <list>
#include <unordered_map>
#include "variant_t.h"

class WEBUI_WIRE_EXPORT Object_t
{
private:
    //Application_t                              *_app;
    std::list<Object_t *>                       _childs;
    Object_t *                                  _parent;
    std::unordered_map<std::string, Variant_t>  _properties;

public:
    void _emit(Event_t msg);

public:
    void deleteLater();

public:
    void setProperty(const std::string &key, Variant_t v);
    const Variant_t &property(const std::string & key);
    bool hasProperty(const std::string &key);

public:
    virtual void event(Event_t msg);

public:
    Object_t(Object_t *parent = nullptr);
    virtual ~Object_t();
};

WEBUI_WIRE_EXPORT void connect(Object_t *sender, std::string event, Object_t *receiver);
WEBUI_WIRE_EXPORT void disconnect(Object_t *sender, std::string event, Object_t *receiver);

#define emit(expr) _emit((expr))

#endif // OBJECT_T_H
