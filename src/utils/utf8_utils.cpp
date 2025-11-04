#include "utf8_utils.h"
#include "utf8_strings.h"

#include <string>
#include <stdexcept>

std::string lcase(const std::string &s)
{
    // First the naive approach
    std::string new_s = s;
    std::string::iterator it = new_s.begin();

    bool all_ascii = true;
    while(it != new_s.end()) {
        char c = *it;
        if (c >= 'A' && c <= 'Z') {
            *it += ('a' - 'A');
        } else if (c < 0) {
            all_ascii = false;
            break;
        }
        ++it;
    }

    if (all_ascii) {
        return new_s;
    } else {
        // If that does not work, do some heavy lifting
        unsigned char *l = Utf8StrMakeLwrUtf8Str(reinterpret_cast<const unsigned char *>(s.c_str()));
        if (l == nullptr) { // Memory allocation problem.
            throw std::runtime_error(std::string("lcase: Not enough memory to allocate output string"));
        }
        std::string r(reinterpret_cast<char *>(l));
        return r;
    }
}

std::string ucase(const std::string &s)
{
    // First the naive approach
    std::string new_s = s;
    std::string::iterator it = new_s.begin();

    bool all_ascii = true;
    while(it != new_s.end()) {
        char c = *it;
        if (c >= 'a' && c <= 'z') {
            *it -= ('a' - 'A');
        } else if (c < 0) {
            all_ascii = false;
            break;
        }
        ++it;
    }

    if (all_ascii) {
        return new_s;
    } else {
        // If that does not work, do some heavy lifting
        unsigned char *l = Utf8StrMakeUprUtf8Str(reinterpret_cast<const unsigned char *>(s.c_str()));
        if (l == nullptr) { // Memory allocation problem.
            throw std::runtime_error(std::string("ucase: Not enough memory to allocate output string"));
        }
        std::string r(reinterpret_cast<char *>(l));
        return r;
    }
}

