#include "fileinfo_t.h"

#include <iostream>

bool FileInfo_t::exists()
{
    std::error_code ec;
    bool e = std::filesystem::exists(_p, ec);
    if (ec.value() != 0) {
        std::cerr << "ERR:file exists" << _p << " error code " << ec.value() << " - " << ec.message() << "\n";
        return false;
    }
    return e;
}

bool FileInfo_t::isReadable()
{
    std::filesystem::file_status st(std::filesystem::status(_p));
    std::filesystem::perms p = st.permissions();
    return (p & std::filesystem::perms::owner_read) != std::filesystem::perms::none;
}

size_t FileInfo_t::size()
{
    std::error_code ec;
    size_t s = std::filesystem::file_size(_p, ec);
    return s;
}

bool FileInfo_t::mkPath()
{
    std::error_code ec;
    std::filesystem::create_directories(_p, ec);
    return ec.value() != 0;
}

unsigned int FileInfo_t::removeRecursively()
{
    std::error_code ec;
    std::uintmax_t count = std::filesystem::remove_all(_p, ec);
    return count;
}

std::string FileInfo_t::ext()
{
    std::string f = _p.filename().string();
    int pos = f.rfind(".");
    if (pos != std::string::npos) {
        return f.substr(pos + 1);
    } else {
        return "";
    }
}

std::string FileInfo_t::toString()
{
    return _p.string();
}

std::filesystem::path FileInfo_t::toPath()
{
    return _p;
}

FileInfo_t &FileInfo_t::operator =(const FileInfo_t &other)
{
    if (this != &other) {
        this->_p = other._p;
    }
    return *this;
}

FileInfo_t::FileInfo_t(std::string file) : _p(file)
{
}

FileInfo_t::FileInfo_t(std::filesystem::path p) : _p(p)
{
}
