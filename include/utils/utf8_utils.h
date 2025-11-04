#ifndef UTF8_UTILS
#define UTF8_UTILS

#ifdef __cplusplus
extern "C" {
#endif

int valid_utf8_c(const char * s);

inline int is_pure_ascii_c(const char *s)
{
    for(; *s > 0; s++);         // i.e. > 0 && <= 127
    return *s == 0;
}

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

#include <string>

std::string lcase(const std::string &s);
std::string ucase(const std::string &s);

inline bool valid_utf8(const char *s)
{
    return valid_utf8_c(s);
}

inline bool valid_utf8(const std::string & s)
{
    return valid_utf8_c(s.c_str());
}

inline bool is_pure_ascii(const char *s)
{
    return is_pure_ascii_c(s);
}

inline bool is_pure_ascii(const std::string & s)
{
    return is_pure_ascii_c(s.c_str());
}

inline bool is_space(char c)
{
    return (c == ' ') || (c == '\t');
}

#endif // __cplusplus

#endif
