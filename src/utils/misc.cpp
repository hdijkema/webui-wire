#include "misc.h"
#include <stdarg.h>
#include <stdio.h>

#ifdef __linux
#define vsprintf_s vsnprintf
#endif

std::string asprintf(const char *fmt_str, ...)
{
    char buf[1024];
    char *buffer = static_cast<char *>(buf);

    va_list args;
    va_start (args, fmt_str);
#ifdef __APPLE__
    int length = vsnprintf(buffer, 1024, fmt_str, args);
#else
    int length = vsnprintf(buffer, 1024, fmt_str, args);
#endif
    va_end(args);
    if (length > 1024) {
        buffer = static_cast<char *>(malloc(length + 1));
        va_start(args, fmt_str);
#ifdef __APPLE__
        vsnprintf(buffer, length + 1, fmt_str, args);
#else
        vsprintf_s(buffer, length + 1, fmt_str, args);
#endif
        va_end(args);
    }

    std::string s(buffer);
    if (buffer != buf) free(buffer);

    return s;
}

#ifdef _WIN32
#include <windows.h>
const DWORD MS_VC_EXCEPTION=0x406D1388;

#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
    DWORD dwType; // Must be 0x1000.
    LPCSTR szName; // Pointer to name (in user addr space).
    DWORD dwThreadID; // Thread ID (-1=caller thread).
    DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)


void SetThreadName(uint32_t dwThreadID, const char* threadName)
{

    // DWORD dwThreadID = ::GetThreadId( static_cast<HANDLE>( t.native_handle() ) );

    THREADNAME_INFO info;
    info.dwType = 0x1000;
    info.szName = threadName;
    info.dwThreadID = dwThreadID;
    info.dwFlags = 0;

    __try
    {
        RaiseException( MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(ULONG_PTR), (ULONG_PTR*)&info );
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
    }
}
void SetThreadName( const char* threadName)
{
    SetThreadName(GetCurrentThreadId(),threadName);
}

void SetThreadName( std::thread* thread, const char* threadName)
{
    DWORD threadId = ::GetThreadId( static_cast<HANDLE>( thread->native_handle() ) );
    SetThreadName(threadId,threadName);
}

#elif defined(__linux__)
#include <sys/prctl.h>
void SetThreadName(std::thread *thread, const char* threadName)
{
    pthread_setname_np(thread->native_handle(), threadName);
    //prctl(PR_SET_NAME,threadName,0,0,0);
}

#else
void SetThreadName(std::thread* thread, const char* threadName)
{
    auto handle = thread->native_handle();
    //pthread_setname_np(handle, threadName);
}
#endif


void setThreadName(std::thread *thr, std::string name)
{
    SetThreadName(thr, name.c_str());
}


void terminateThread(std::thread *thr)
{
#ifdef _WINDOWS
  TerminateThread(thr->native_handle(), 0);
#endif
}
