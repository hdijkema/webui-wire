#ifndef WEBWIRESTANDARDDIALOGS_H
#define WEBWIRESTANDARDDIALOGS_H

#include "object_t.h"
#include "misc.h"

class WebWireHandler;
class WebUIWindow;

class PathFilter {
public:
    std::string filter_name;                    // e.g. filtername = "HTML Files"
    std::stringlist filter_exts;         // e.g. filter_exts.push_back("html"); filter_exts.push_back("htm")
public:
    bool emptyFilter() { return filter_exts.empty(); }
public:
    PathFilter &operator <<(std::string ext) {
        filter_exts.push_back(ext);
        return *this;
    }
public:
    PathFilter(std::string fn, std::stringlist exts) {
        filter_name = fn;
        filter_exts = exts;
    }
};

typedef wwlist<PathFilter> PathFilterList;

class WebWireStandardDialogs : public Object_t
{
public:
    PathFilterList filtersFromString(std::string filters);
public:
    std::string openFileDialog(WebWireHandler *h, WebUIWindow *win, std::string title, std::string base_dir, PathFilterList filters, bool &cancelled);
    std::string saveFileDialog(WebWireHandler *h, WebUIWindow *win, std::string title, std::string base_dir, PathFilterList filters, bool &cancelled);
    std::string getDirectoryDialog(WebWireHandler *h, WebUIWindow *win, std::string title, std::string base_dir, bool &cancelled);

public:
    WebWireStandardDialogs(Object_t *parent = nullptr);
};

#endif // WEBWIRESTANDARDDIALOGS_H
