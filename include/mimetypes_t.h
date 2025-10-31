#ifndef MIMETYPES_T_H
#define MIMETYPES_T_H

#include "object_t.h"
#include <string>

typedef struct {
    std::string name;
    std::string mimetype;
    std::string ext;
    std::string info;
} MimeType_t;

class WebWireHandler;

class MimeTypes_t : public Object_t
{
private:
    static bool initialized;;
    static std::vector<MimeType_t>              _mime_types;
    static std::unordered_map<std::string, int> _ext_2_mimetype;

private:
    WebWireHandler *_handler;

private:
    void addMimeType(std::string name, std::string mimetype, std::string ext, std::string info);
    void init();

public:
    std::string mimetypeByExt(std::string ext);

public:
    MimeTypes_t(WebWireHandler *h, Object_t *parent = nullptr);
};

#endif // MIMETYPES_T_H
