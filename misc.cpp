#include "misc.h"
#include <stdarg.h>

std::string asprintf(const char *fmt_str, ...)
{
    char *buffer = static_cast<char *>(malloc(102400));
    va_list args;
    va_start (args, fmt_str);
    vsprintf_s(buffer, 102400, fmt_str, args);
    va_end(args);
    std::string s(buffer);
    free(buffer);
    return s;
}
