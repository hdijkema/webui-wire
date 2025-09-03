#include "object_t.h"

#include "application_t.h"

void Object_t::_emit(Event_t evt)
{
    Application_t::current()->evtQueue().enqueue(evt);
}

void Object_t::deleteLater()
{
    emit(evt_delete_later);
}

void Object_t::setProperty(const std::string &key, Variant_t v)
{
    _properties[key] = v;
}

bool Object_t::hasProperty(const std::string &key)
{
    return _properties.contains(key);
}

const Variant_t &Object_t::property(const std::string &key)
{
    return _properties[key];
}

void Object_t::event(Event_t evt)
{
    if (evt.is_a(id_delete_later)) {
        delete this;
    }
}

Object_t::Object_t(Object_t *parent)
{
    if (parent != nullptr) {
        parent->_childs.remove(this);
        parent->_childs.push_back(this);
    }
    Application_t::current()->addRoute(this, this, id_delete_later);
}

Object_t::~Object_t()
{
    while(!_childs.empty()) {
        Object_t *child = _childs.front();
        _childs.pop_front();
        delete child;
    }
    Application_t::current()->delObject(this);
}

void connect(Object_t *sender, std::string event, Object_t *receiver)
{
    Application_t::current()->addRoute(sender, receiver, event);
}

void disconnect(Object_t *sender, std::string event, Object_t *receiver)
{
    Application_t::current()->delRoute(sender, receiver, event);
}
