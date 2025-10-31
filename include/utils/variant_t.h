#ifndef VARIANT_T_H
#define VARIANT_T_H

#include <cassert>
#include <string>

#ifdef __linux
#include <string.h>
#include <malloc.h>
#endif

typedef enum {
    t_unknown = 0,
    t_string,
    t_int,
    t_bool,
    t_double,
    t_url,
    t_json_string,
    t_std_file_handle,
    t_const_char_ptr
} VariantType_t;


class Variant_t
{
private:
    VariantType_t  _type;
    union {
        int         i_v;
        bool        b_v;
        double      d_v;
        char       *s_v;
        FILE       *f_v;
        const char *cs_v;
    } val;

public:
    int toInt() const { assert(_type == t_int);return val.i_v; }
    bool toBool() const { assert(_type == t_bool);return val.b_v; }
    double toDouble() const { assert(_type == t_double);return val.d_v; }
    std::string toString() const { assert(_type == t_string);std::string s(val.s_v);return s; }
    FILE *toFILE() const { assert(_type == t_std_file_handle);return val.f_v; }
    const char *toCStr() const { assert(_type == t_const_char_ptr);return val.cs_v; }

public:
    Variant_t the_type() {
        return _type;
    }

    void operator =(const Variant_t &t)
    {
        _type = t._type;
        val = t.val;
        if (_type == t_string) {
            size_t l = strlen(t.val.s_v) + 1;
            val.s_v = static_cast<char *>(malloc(l));
            memcpy(val.s_v, t.val.s_v, l);
        }
    }

public:
    Variant_t()
    {
        _type = t_unknown;
    }

    Variant_t(const Variant_t &t) {
        _type = t._type;
        val = t.val;
        if (_type == t_string) {
            size_t l = strlen(t.val.s_v) + 1;
            val.s_v = static_cast<char *>(malloc(l));
            memcpy(val.s_v, t.val.s_v, l);
        }
    }

    Variant_t(int v) {
        _type = t_int;
        val.i_v = v;
    }

    Variant_t(bool v) {
        _type = t_bool;
        val.b_v = v;
    }

    Variant_t(double v) {
        _type = t_double;
        val.d_v = v;
    }

    Variant_t(std::string s) {
        _type = t_string;
        size_t l = strlen(s.c_str()) + 1;
        val.s_v = static_cast<char *>(malloc(l));
        memcpy(val.s_v, s.c_str(), l);
    }

    Variant_t(FILE *f) {
        _type = t_std_file_handle;
        val.f_v = f;
    }

    Variant_t(const char *c_str) {
        _type = t_const_char_ptr;
        val.cs_v = c_str;
    }

    ~Variant_t()
    {
        if (_type == t_string) {
            free(val.s_v);
        }
    }
};

#endif // VARIANT_T_H
