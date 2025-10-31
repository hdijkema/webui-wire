#ifndef TIMER_T_H
#define TIMER_T_H

#include <thread>
#include "object_t.h"

#define id_timeout      "timeout"
#define evt_timeout      Event_t(id_timeout, this)

class Timer_t : public Object_t
{
private:
    int              _ms;
    std::string      _name;
    bool             _single_shot;
    bool             _stopped;
    bool             _reset;
    std::thread     *_timer_thread;

private:
    void timeout();

public:
    int interval();

public:
    void start();
    void start(int ms);
    void stop();
    void reset();
    void setInterval(int ms);
    void setTimeout(int ms);
    void setSingleShot(bool y);

public:
    Timer_t(const std::string &name);
};

#endif // TIMER_T_H
