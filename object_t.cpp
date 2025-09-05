#include "object_t.h"

#include "application_t.h"

void Object_t::_emit(Event_t evt)
{
    Application_t *a = Application_t::current();
    if (a) a->evtQueue().enqueue(evt);
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
    _parent = parent;
    Application_t *a = Application_t::current();
    if (a) a->addRoute(this, this, id_delete_later);
}

Object_t::~Object_t()
{
    while(!_childs.empty()) {
        Object_t *child = _childs.front();
        _childs.pop_front();
        child->_parent = nullptr;                   // release the parent of this child as we are deleting 'downwards'
        delete child;
    }

    // Check if we're somewhere in the Object tree, remove ourselve from the parent if not null
    if (_parent != nullptr) {
        _parent->_childs.remove(this);
    }

    Application_t *a = Application_t::current();
    if (a) a->delObject(this);
}

void connect(Object_t *sender, std::string event, Object_t *receiver)
{
    Application_t *a = Application_t::current();
    if (a) a->addRoute(sender, receiver, event);
}

void disconnect(Object_t *sender, std::string event, Object_t *receiver)
{
    Application_t *a = Application_t::current();
    if (a) a->delRoute(sender, receiver, event);
}
