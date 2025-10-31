#ifndef UTF8_STRINGS_H
#define UTF8_STRINGS_H

#define Utf8Char unsigned char	// must be 1 byte, 8 bits, can be char, the UTF consortium specify unsigned

// Utf 8
size_t StrLenUtf8(const Utf8Char* str);
int StrnCmpUtf8(const Utf8Char* Utf8s1, const Utf8Char* Utf8s2, size_t ztCount);
int StrCmpUtf8(const Utf8Char* Utf8s1, const Utf8Char* Utf8s2);
size_t CharLenUtf8(const Utf8Char* pUtf8);
Utf8Char* ForwardUtf8Chars(const Utf8Char* pUtf8, size_t ztForwardUtf8Chars);
Utf8Char* Utf8StrMakeUprUtf8Str(const Utf8Char* pUtf8);
Utf8Char* Utf8StrMakeLwrUtf8Str(const Utf8Char* pUtf8);
int StrnCiCmpUtf8(const Utf8Char* pUtf8s1, const Utf8Char* pUtf8s2, size_t ztCount);
int StrCiCmpUtf8(const Utf8Char* pUtf8s1, const Utf8Char* pUtf8s2);
Utf8Char* StrCiStrUtf8(const Utf8Char* pUtf8s1, const Utf8Char* pUtf8s2);


/// std::string interaction
#include <string>
#include <iostream>

inline std::string lcase(std::string in)
{
    const Utf8Char *s_utf8 = reinterpret_cast<const Utf8Char *>(in.c_str());
    Utf8Char *new_s = Utf8StrMakeLwrUtf8Str(s_utf8);
    if (new_s == nullptr) {
        std::cerr << "ERR: cannot allocate enough memory for new string, returning original std::string";
        return in;
    } else {
        std::string out(reinterpret_cast<char *>(new_s));
        return out;
    }
}

inline std::string ucase(std::string in)
{
    const Utf8Char *s_utf8 = reinterpret_cast<const Utf8Char *>(in.c_str());
    Utf8Char *new_s = Utf8StrMakeUprUtf8Str(s_utf8);
    if (new_s == nullptr) {
        std::cerr << "ERR: cannot allocate enough memory for new string, returning original std::string";
        return in;
    } else {
        std::string out(reinterpret_cast<char *>(new_s));
        return out;
    }
}


#endif // UTF8_STRINGS_H
