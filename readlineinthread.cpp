#include "readlineinthread.h"
#include "misc.h"

#include <thread>

#ifdef _LINUX
#include <sys/select.h>
#endif

ReadLineInThread::ReadLineInThread()
{
    _buffer_len = 10 * 1024 * 1024;      // max bufferlen, i.e. max line len = 10MB
    _buffer = static_cast<char *>(malloc(_buffer_len + 1));
    _go_on = true;
    _thread = new std::thread([this]() { this->run(); });
    setThreadName(_thread, "ReadLineInThread");
    _wait_ms = 25;
}

ReadLineInThread::~ReadLineInThread()
{
    quit();
    free(_buffer);
    delete _thread;
}

void ReadLineInThread::quit()
{
    _go_on = false;
#ifdef _WINDOWS
    terminateThread(_thread);
#endif
    if (_thread->joinable()) {
        _thread->join();
    }
}

void ReadLineInThread::haveALine(std::string l)
{
    emit(evt_readline_have_line << l);
}

void ReadLineInThread::haveEof()
{
    emit(evt_readline_eof);
}

#if defined(__linux__) || defined(__APPLE__)
#define strerror_s(buf, size, nr) strerror_r(nr, buf, size)
#endif

void ReadLineInThread::haveError(int error_number)
{
    char buf[10240];
    strerror_s(buf, 10239, error_number);
    std::string s = buf;
    emit(evt_readline_error << error_number << s);
}

#ifdef _WINDOWS
#include <windows.h>
#endif

void ReadLineInThread::run()
{
    _wait_ms = 1500;
    while(_go_on) {
        char *s = nullptr;
        char buffer[10240];
        fgets(buffer, 10240, stdin);
        if (feof(stdin)) {
            _go_on = false;
        } else {
            std::string line = buffer;
            trim(line);
            if (line != "") {
                haveALine(line);
                if (line == "exit") {
                    _go_on = false;
                }
            }
        }
    }
}
