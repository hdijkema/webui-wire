#ifndef UTF8_STRINGS_H
#define UTF8_STRINGS_H

#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
}
#endif


#endif // UTF8_STRINGS_H
