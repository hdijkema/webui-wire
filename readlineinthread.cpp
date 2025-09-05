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
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD fdwMode = (ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT | ENABLE_EXTENDED_FLAGS) & ~ENABLE_QUICK_EDIT_MODE;
    SetConsoleMode(hStdin, fdwMode);
    INPUT_RECORD inp[10240];
    int buf_idx = 0;
#endif

    _wait_ms = 1500;
    while(_go_on) {
        char *s = nullptr;
        bool have_line = false;
#ifdef _WINDOWS
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        DWORD nr_events = 0;
        bool ok = GetNumberOfConsoleInputEvents(hStdin, &nr_events);
        if (nr_events > 0) {
            bool ok = ReadConsoleInput(hStdin, inp, 10240, &nr_events);

            if (ok) {
                int i;
                for(i = 0; i < nr_events; i++) {
                    if (inp[i].EventType == KEY_EVENT) {
                        if (inp[i].Event.KeyEvent.bKeyDown) {
                            char c = inp[i].Event.KeyEvent.uChar.AsciiChar;
                            if (c == '\r') { c = '\n'; }
                            if (c == 8) {
                                if (buf_idx > 0) {
                                    fputc(c, stdout);
                                    fputc(' ', stdout);
                                    fputc(c, stdout);
                                    buf_idx--;
                                }
                            } else {
                                fputc(c, stdout);
                                _buffer[buf_idx++] = c;
                            }
                            _buffer[buf_idx] = '\0';
                            if (c == '\n') {
                                _buffer[buf_idx] = '\0';
                                std::string l(_buffer);
                                haveALine(l);
                                buf_idx = 0;
                            }
                        }
                    }
                }
            }
        }
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
        if (have_line && s != nullptr) {
            std::string l(s);
            haveALine(l);
        }
#endif
    }
    fprintf(stderr, "left readline thread\n");
}
