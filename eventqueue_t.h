#ifndef EVENTQUEUE_T_H
#define EVENTQUEUE_T_H

#include "webui_wire_defs.h"
#include <queue>
#include <mutex>
#include <semaphore>
#include <chrono>

#include "event_t.h"

#define MAX_QUEUE_DEPTH 100000

class WEBUI_WIRE_EXPORT EventQueue_t
{
private:
    std::mutex                                  _mutex;
    std::counting_semaphore<MAX_QUEUE_DEPTH>    _sem;
    std::queue<Event_t>                         _queue;
    int                                         _wait_ms;
    std::chrono::milliseconds                   _d;
    int                                         _id;

public:
    int count();
    Event_t dequeue();
    void enqueue(const Event_t &e);
    Event_t dequeue_if(const char *kind);
    int empty();

public:
    static bool isNull(const Event_t &e);

public:
    EventQueue_t(int wait_ms = 5);

public:
    void setWait(int ms);
};

#endif // EVERNTQUEUE_T_H
