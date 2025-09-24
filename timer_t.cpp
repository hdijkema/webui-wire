#include "timer_t.h"
#include "misc.h"

#include "webwirehandler.h"

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

        WebWireHandler *h = Application_t::current()->handler();
        h->message("Starting timeout-thread");

        while(go_on) {
            int ms_count = _ms;
            int c = 0;
            while (!this->_stopped && ms_count > 0) {
                if (_reset) {
                    _reset = false;
                    ms_count = _ms;
                }
                int m = (ms_count < 10) ? ms_count : 10;
                int sleep_len = (ms_count < m) ? ms_count : m;
                if (sleep_len == 0) { sleep_len = 1; }
                std::this_thread::sleep_for(std::chrono::milliseconds(sleep_len));
                ms_count -= sleep_len;
                c += sleep_len;
                if (c >= 10) {
                    h->message(asprintf("tick: %d %d\n", ms_count, sleep_len));
                    c = 0;
                }
            }

            if (this->_stopped) {
                go_on = false;
            } else {
                h->message("emitting timeout event");
                timeout();
                if (single_shot) { go_on = false; }
            }

        }

        fprintf(stderr, "Ending thread function\n");
        _stopped = false;
    });
    setThreadName(_timer_thread, "timer-thread-" + _name);
}

void Timer_t::stop()
{
    _stopped = true;
    if (_timer_thread != nullptr) {
        if (_timer_thread->joinable()) {
            _timer_thread->join();
        }
        delete _timer_thread;
        _timer_thread = nullptr;
    }
}

void Timer_t::setInterval(int ms)
{
    _ms = ms;
}

void Timer_t::setTimeout(int ms)
{
    _ms = ms;
}

void Timer_t::setSingleShot(bool y)
{
    _single_shot = y;
}

void Timer_t::reset()
{
    _reset = true;
}

void Timer_t::timeout()
{
    emit(evt_timeout << _name);
}

int Timer_t::interval()
{
    return _ms;
}

Timer_t::Timer_t(const std::string &name)
    : _timer_thread(nullptr), _stopped(false), _single_shot(false)
{
    _name = name;
    _timer_thread = nullptr;
}

