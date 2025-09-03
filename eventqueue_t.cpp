#include "eventqueue_t.h"

EventQueue_t::EventQueue_t(int wait_ms) : _sem(0)
{
    _wait_ms = wait_ms;
    _id = 0;
    _d = std::chrono::milliseconds(_wait_ms);
}

int EventQueue_t::count()
{
    int c;
    _mutex.lock();
    c = _queue.size();
    _mutex.unlock();
    return c;
}

int EventQueue_t::empty()
{
    _mutex.lock();
    bool b = _queue.empty();
    _mutex.unlock();
    return b;
}

bool EventQueue_t::isNull(const Event_t &e)
{
    return e.event() == id_evt_null;
}

Event_t EventQueue_t::dequeue()
{
    if (_sem.try_acquire_for(_d)) {
        _mutex.lock();
        Event_t e = _queue.front();
        _queue.pop();
        _mutex.unlock();
        return e;
    }

    Event_t null_e(id_evt_null, nullptr);
    return null_e;
}

Event_t EventQueue_t::dequeue_if(const char *kind)
{
    if (_sem.try_acquire_for(_d)) {
        _mutex.lock();
        Event_t e = _queue.front();
        if (e.event() == kind) {
            _queue.pop();
            _mutex.unlock();
            return e;
        }
    }

    Event_t null_e(id_evt_null, nullptr);
    return null_e;
}

void EventQueue_t::setWait(int ms)
{
    _wait_ms = ms;
    _d = std::chrono::milliseconds(_wait_ms);
}


void EventQueue_t::enqueue(const Event_t &e)
{
    _mutex.lock();
    _queue.push(e);
    _mutex.unlock();
    _sem.release();
}
