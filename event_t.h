#ifndef EVENT_T_H
#define EVENT_T_H

#include "webui_wire_defs.h"
#include <string>
#include <list>

#include "variant_t.h"

#define id_evt_null     "null-event"

#define id_delete_later "delete-later"
#define evt_delete_later Event_t(id_delete_later, this)

class Object_t;

class WEBUI_WIRE_EXPORT Event_t
{
private:
    std::string    _event;
    Object_t      *_sender;
    unsigned int   _seq_nr;

private:
    std::list<Variant_t> _payload;

private:
    Variant_t rotate();

public:
    const std::string &event() const;
    int seqNr() const;
    Object_t *sender() const;

public:
    bool is_a(const Event_t & other);
    bool is_a(const char *other);
    bool isNull();

public:
    Event_t &operator <<(int m);
    Event_t &operator <<(bool m);
    Event_t &operator <<(double m);
    Event_t &operator <<(std::string s);
    Event_t &operator <<(FILE *f);
    Event_t &operator <<(const char *s);

    Event_t &operator >>(int &m);
    Event_t &operator >>(bool &m);
    Event_t &operator >>(double &m);
    Event_t &operator >>(std::string &s);
    Event_t &operator >>(FILE *&f);
    Event_t &operator >>(const char *&ptr);

public:
    Event_t(std::string event, Object_t *sender);
    Event_t(const Event_t &e);
};

#endif // EVENT_T_H
