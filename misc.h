#ifndef MISC_H
#define MISC_H

#include <string>
#include <list>
#include <thread>
#include "json.hpp"
#include "utf8_strings.h"
#include "webui_wire_defs.h"

// Trim from the start (in place)
inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
                return !std::isspace(ch);
            }));
}

// Trim from the end (in place)
inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
                return !std::isspace(ch);
            }).base(), s.end());
}

// Trim from both ends (in place)
inline void trim(std::string &s) {
    rtrim(s);
    ltrim(s);
}

// Trim from the start (copying)
inline std::string ltrim_copy(std::string s) {
    ltrim(s);
    return s;
}

// Trim from the end (copying)
inline std::string rtrim_copy(std::string s) {
    rtrim(s);
    return s;
}

// Trim from both ends (copying)
inline std::string trim_copy(std::string s) {
    trim(s);
    return s;
}

// for convenience
using json = nlohmann::json;

class Size_t
{
private:
    int _w, _h;

public:
    int width() { return _w; }
    int height() { return _h; }
    Size_t &setWidth(int w) { _w = w; return *this; }
    Size_t &setHeight(int h) { _h = h; return *this; }
    Size_t &set(int w, int h) { _w = w; _h = h; return *this; }
    Size_t &set(const Size_t &s) { _w = s._w; _h = s._h;return *this; }
public:
    Size_t &operator=(const Size_t &s) { return set(s); }
public:
    Size_t(int w, int h) { set(w, h); }
    Size_t(const Size_t &s) { set(s); }
    Size_t() { set(0, 0); }
};

class Point_t
{
private:
    int _x, _y;

public:
    int x() { return _x; }
    int y() { return _y; }
    Point_t &setX(int x) { _x = x; return *this; }
    Point_t &setY(int y) { _y = y; return *this; }
    Point_t &set(int x, int y) { _x = x; _y = y; return *this; }
    Point_t &set(const Point_t &p) { _x = p._x; _y = p._y; return *this; }
public:
    Point_t &operator = (const Point_t &p) { return set(p); }
public:
    Point_t(int x, int y) { set(x, y); }
    Point_t(const Point_t & p) { set(p); }
    Point_t() { set(0, 0); }
};

template <typename T> class wwlist : public std::list<T>
{
public:
    wwlist<T> &operator <<(const T &item) {
        this->push_back(item);
        return *this;
    }

    wwlist<T> append(const T &item) {
        this->push_back(item);
        return *this;
    }

};

namespace std {
class stringlist;
}

class std::stringlist : public wwlist<std::string>
{
public:
    std::string join(std::string sep) {
        std::stringlist::iterator it = this->begin();
        std::string r;
        if (it != this->end()) {
            r = *it++;
            while(it != this->end()) {
                r += sep;
                r += *it++;
            }
        }
        return r;
    }
};

template <typename Tkey, typename Tval> class wwhash : public std::unordered_map<Tkey, Tval>
{
public:
    wwlist<Tkey> keys() {
        wwlist<Tkey> keys;
        for(auto &[key, value] : *this) {
            keys.push_back(key);
        }
        return keys;
    }
};


inline std::string replace(std::string str, std::string find_str, std::string replace_str)
{
    std::size_t idx = str.find(find_str);
    int l = find_str.length();

    while(idx != std::string::npos) {
        str.replace(idx, l, replace_str);
        idx = str.find(find_str, idx + replace_str.length());
    }

    return str;
}

inline int toInt(const std::string &s, bool *ok = nullptr)
{
    bool _ok = true;
    const char *d = s.c_str();
    int num = 0;
    for (int i = 0;_ok && d[i] != '\0'; i++) {
        if (d[i] >= '0' && d[i] <= '9') {
            num = num * 10 + (d[i] - '0');
        } else {
            _ok = false;
        }
    }
    if (ok != nullptr) { *ok = _ok; }
    return num;
}

inline bool toBool(const std::string &s, bool *ok = nullptr)
{
    std::string b = lcase(s);
    bool _ok = true;
    bool r = false;
    if (b == "true" || b == "#t" || b == "1" || b == "t") {
        r = true;
    } else if (b == "false" || b == "#f" || b == "0" || b == "f") {
        r = false;
    } else {
        r = false;
        _ok = false;
    }
    if (ok != nullptr) { *ok = _ok; }
    return r;
}

inline double toDouble(const std::string &s, bool *ok = nullptr)
{
    bool _ok = true;
    const char *d = s.c_str();
    bool after_dot = false;
    double num = 0;
    double frac = 10;
    for (int i = 0; _ok && d[i] != '\0'; i++) {
        if (d[i] >= '0' && d[i] <= '9') {
            if (after_dot) {
                num += (d[i] - '0') / frac;
                frac *= 10;
            } else {
                num = num * 10 + (d[i] - '0');
            }
        } else if (d[i] == '.') {
            after_dot = true;
        } else {
            _ok = false;
        }
    }
    if (ok != nullptr) { *ok = _ok; }
    return num;
}


std::string asprintf(const char *fmt_str, ...);

WEBUI_WIRE_EXPORT void setThreadName(std::thread *thr, std::string name);
WEBUI_WIRE_EXPORT void terminateThread(std::thread *thr);

class AtDelete_t
{
public:
    virtual void deleteInProgress(std::string from) = 0;
};


#endif // MISC_H
