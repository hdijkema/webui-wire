#ifndef HTTPRESPONSE_T_H
#define HTTPRESPONSE_T_H

#include <String>
#include "misc.h"

class WebWireHandler;

class HttpResponse_t
{
private:
    WebWireHandler                  *_handler;

private:
    int                              _code;
    wwhash<std::string, std::string> _headers;
    int                              _content_size;
    std::string                      _content;
    std::string                      _file;

private:
    int                              _http_response_size;
    char                            *_http_response;

public:
    void setResponseCode(int code);
    void setContentType(std::string content_type);
    void addHeader(std::string name, std::string content);
    void setContent(const std::string &content);
    void setFile(const std::string &file);

public:
    std::string contentTypeForFileExt(std::string ext);
    static std::string codeText(int code);

public:
    const char *response(int &content_length);

public:
    HttpResponse_t(WebWireHandler *h, int code = 200);
};

#endif // HTTPRESPONSE_T_H
