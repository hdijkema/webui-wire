#include "timer_t.h"

void Timer_t::start(int ms)
{
    setInterval(ms);
    start();
}

void Timer_t::start()
{
    _stopped = false;
    _timer_thread = new std::thread([this]() {
        bool go_on = true;
        bool single_shot = this->_single_shot;
        while(go_on) {
            int ms_count = _ms;
            while (!this->_stopped && ms_count > 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                ms_count -= 1;
            }
            if (single_shot) { go_on = false; }
            if (this->_stopped) {
                go_on = false;
            } else {
                timeout();
            }
        }
        _stopped = false;
    });
}

void Timer_t::stop()
{
    _stopped = true;
    _timer_thread->join();
}

void Timer_t::setInterval(int ms)
{
    _ms = ms;
}

void Timer_t::setSingleShot(bool y)
{
    _single_shot = y;
}

void Timer_t::timeout()
{
    emit(evt_timeout << _name);
}

Timer_t::Timer_t(const std::string &name)
    : _timer_thread(nullptr), _stopped(false), _single_shot(false)
{
    _name = name;
}

