#ifndef IS_UTF8

#ifdef __cplusplus
extern "C" {
#endif

int is_utf8_c(const char * s);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

#include <string>

inline bool is_utf8(const char *s)
{
    return is_utf8_c(s);
}

inline bool is_utf8(const std::string & s)
{
    return is_utf8_c(s.c_str());
}

#endif

#endif // IS_UTF8
