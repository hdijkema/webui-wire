#include "readlineinthread.h"

#ifdef _LINUX
#include <sys/select.h>
#endif

ReadLineInThread::ReadLineInThread()
{
    _buffer_len = 10 * 1024 * 1024;      // max bufferlen, i.e. max line len = 10MB
    _buffer = static_cast<char *>(malloc(_buffer_len + 1));
    _go_on = true;
    _thread = new std::thread([this]() { this->run(); });
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
#ifdef _WINDOWS
    HANDLE hStdInput = GetStdHandle(STD_INPUT_HANDLE);
#endif

    while(_go_on) {
        char *s = nullptr;
        bool have_line = false;
#ifdef _WINDOWS
        DWORD result = WaitForSingleObject(hStdInput, _wait_ms); // Time in milliseconds to wait
        if (result == WAIT_OBJECT_0) {
            s = fgets(_buffer, _buffer_len, stdin);
            // Check for null and eof and errors
            if (s == NULL) {
                // something has happened
                if (feof(stdin)) {
                    haveEof();
                    _go_on = false;
                } else {
                    haveError(errno);
                    _go_on = false;
                }
            }
        }
        have_line = strlen(_buffer) > 0;

#else
        fd_set          read_set;
        struct timeval  tv;
        FD_ZERO(&read_set);
        FD_SET(fileno(stdin), &read_set);
        tv.tv_sec = 0;
        tv.tv_usec = _wait_ms * 1000;    // _wait_ms ms
        int retval = select(1, &read_set, NULL, NULL, &tv);
        if (retval == -1) {
            perror("select()");
        } else if (retval) {
            s = fgets(_buffer, _buffer_len, stdin);
            have_line = true;
        }
#endif
        if (have_line && s != nullptr) {
            std::string l(s);
            haveALine(l);
        }
    }
}
