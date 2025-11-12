#ifndef FILEINFO_T_H
#define FILEINFO_T_H

#include <string>
#include <filesystem>

class FileInfo_t
{
private:
    std::filesystem::path _p;
public:
    bool exists();
    bool isReadable();
    size_t size();

public:
    bool mkPath();
    unsigned int removeRecursively();
    std::string ext();

public:
    std::string toString();
    std::filesystem::path toPath();

public:
    FileInfo_t &operator =(const FileInfo_t & other);

public:
    FileInfo_t(std::string file);
    FileInfo_t(std::filesystem::path p);
};

#endif // FILEINFO_T_H
