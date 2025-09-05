#include "event_t.h"

static unsigned int seq_nr = 0;

Event_t::Event_t(const Event_t &e)
{
    _seq_nr = e._seq_nr;
    _event = e._event;
    _sender = e._sender;

    std::list<Variant_t> n;
    std::list<Variant_t>::const_iterator it = e._payload.begin();
    while(it != e._payload.end()) {
        n.push_back(*it);
        it++;
    }
    _payload = n;
}


Event_t::Event_t(std::string event, Object_t *sender)
{
    _seq_nr = ++seq_nr;
    _event = event;
    _sender = sender;
}

Variant_t Event_t::rotate()
{
    assert(!_payload.empty());

    Variant_t v(_payload.front());
    _payload.pop_front();
    _payload.push_back(v);
    return v;
}

const std::string &Event_t::event() const
{
    return _event;
}

int Event_t::seqNr() const
{
    return _seq_nr;
}

Object_t *Event_t::sender() const
{
    return _sender;
}

bool Event_t::is_a(const Event_t &other)
{
    return _event == other._event;
}

bool Event_t::is_a(const char *other)
{
    return _event == other;
}

bool Event_t::isNull()
{
    return _event == id_evt_null;
}

Event_t &Event_t::operator <<(int m)
{
    _payload.push_back(Variant_t(m));
    return *this;
}

Event_t &Event_t::operator <<(bool m)
{
    _payload.push_back(Variant_t(m));
    return *this;
}

Event_t &Event_t::operator <<(double m)
{
    _payload.push_back(Variant_t(m));
    return *this;
}

Event_t &Event_t::operator <<(std::string m)
{
    _payload.push_back(Variant_t(m));
    return *this;
}

Event_t &Event_t::operator <<(FILE *f)
{
    _payload.push_back(Variant_t(f));
    return *this;
}

Event_t &Event_t::operator <<(const char *s)
{
    _payload.push_back(Variant_t(s));
    return *this;
}

Event_t &Event_t::operator >>(int &m)
{
    m = rotate().toInt();
    return *this;
}

Event_t &Event_t::operator >>(bool &m)
{
    m = rotate().toBool();
    return *this;
}

Event_t &Event_t::operator >>(double &m)
{
    m = rotate().toDouble();
    return *this;
}

Event_t &Event_t::operator >>(std::string &m)
{
    m = rotate().toString();
    return *this;
}

Event_t &Event_t::operator >>(FILE *&f)
{
    f = rotate().toFILE();
    return *this;
}

Event_t &Event_t::operator >>(const char *&c_str)
{
    c_str = rotate().toCStr();
    return *this;
}


