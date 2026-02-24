#ifndef WEBWIRESTANDARDDIALOGS_H
#define WEBWIRESTANDARDDIALOGS_H

#include "object_t.h"
#include "misc.h"

#define STDDLG_WIN_USE_THREADS (1==1)

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
    int openFileDialog(WebWireHandler *h, WebUIWindow *win, std::string title, std::string base_dir, PathFilterList filters);
    int saveFileDialog(WebWireHandler *h, WebUIWindow *win, std::string title, std::string base_dir, PathFilterList filters);
    int getDirectoryDialog(WebWireHandler *h, WebUIWindow *win, std::string title, std::string base_dir, int *result, std::string &dir_out);

public:
    WebWireStandardDialogs(Object_t *parent = nullptr);
};

#endif // WEBWIRESTANDARDDIALOGS_H
