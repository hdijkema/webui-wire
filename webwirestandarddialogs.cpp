#include "webwirestandarddialogs.h"
#include "webuiwindow.h"
#include "webwirehandler.h"

#include <regex>

extern "C" {
#include <nfd.h>
}

static PathFilter getFilter(std::string filter)
{
    trim(filter);
    std::regex re("([^(]+)[(]([^)]+)[)]");
    std::smatch m;

    std::regex_search(filter, m, re);
    if (!m.empty()) {
        std::string name = trim_copy(m[1]);
        PathFilter f(name, {});
        std::string exts = trim_copy(m[2]);
        if (exts.length() > 0) {
            if (exts[exts.length() - 1] != ',') { exts += ","; }
        } else {
            return f;
        }
        std::regex re_e("([^,]+)[,]");
        std::smatch m_e;

        std::regex_search(exts, m_e, re_e);
        while (exts != "" && !m_e.empty()) {
            std::string ext = trim_copy(m_e[1]);
            f << ext;
            exts = trim_copy(exts.substr(m_e.length()));
            std::regex_search(exts, m_e, re_e);
        }
        return f;

    } else {
        return PathFilter("", {});
    }
}

PathFilterList WebWireStandardDialogs::filtersFromString(std::string filters)
{
    trim(filters);

    PathFilterList f;
    std::regex re("([^;]+)[;]");
    std::smatch m;

    if (filters.length() > 0) {
        if (filters[filters.length() - 1] != ';') {
            filters += ";";
        }
    } else {
        return f;
    }

    std::regex_search(filters, m ,re);
    while(filters != "" && !m.empty()) {
        PathFilter pf = getFilter(trim_copy(m[1]));
        if (!pf.emptyFilter()) {
            f.push_back(pf);
        }
        int l = m.length();
        filters = trim_copy(filters.substr(l));
        std::regex_search(filters, m ,re);
    }

    return f;
}

static std::string makeFilters(PathFilterList filters)
{
    std::string filter_list;
    std::list<PathFilter>::iterator it = filters.begin();
    std::string sep = "";
    while(it != filters.end()) {
        PathFilter &f = *it;
        filter_list += sep;
        filter_list += f.filter_exts.join(",");             // This is the windows way?
        sep = "; ";
        it++;
    }
    return filter_list;
}

static std::string correctPath(std::string p)
{
#ifdef _WINDOWS
    p = replace(p, "/", "\\");
    p = replace(p, "\\\\", "\\");
#else
    p = replace(p, "\\", "/");
    p = replace(p, "//", "/");
#endif
    std::filesystem::path _p(p);
    return _p.string();
}

std::string WebWireStandardDialogs::openFileDialog(WebWireHandler *h, WebUIWindow *win,
                                                   std::string title, std::string base_dir, PathFilterList filters,
                                                   bool &cancelled)
{
    nfdchar_t *outPath = NULL;
    nfd_parent_window_data_ptr_t parent_win = static_cast<nfd_parent_window_data_ptr_t>(win->nativeHandle());

    std::string filter_list = makeFilters(filters);
    base_dir = correctPath(base_dir);
    const char *bd = (base_dir == "") ? nullptr : base_dir.c_str();

    nfdresult_t result = NFD_OpenDialogWithParent( title.c_str(), filter_list.c_str(), bd, &outPath, parent_win );

    std::string fn;
    if ( result == NFD_OKAY ) {
        fn = outPath;
        free(outPath);
        cancelled = false;
    }
    else if ( result == NFD_CANCEL ) {
        cancelled = true;
    } else {
        h->error(asprintf("NFD: %s\n", NFD_GetError()));
        cancelled = true;
    }

    return fn;
}

std::string WebWireStandardDialogs::saveFileDialog(WebWireHandler *h, WebUIWindow *win,
                                                   std::string title, std::string base_dir, PathFilterList filters,
                                                   bool &cancelled)
{
    nfdchar_t *outPath = NULL;
    nfd_parent_window_data_ptr_t parent_win = static_cast<nfd_parent_window_data_ptr_t>(win->nativeHandle());

    std::string filter_list = makeFilters(filters);
    base_dir = correctPath(base_dir);
    const char *bd = (base_dir == "") ? nullptr : base_dir.c_str();

    nfdresult_t result = NFD_SaveDialogWithParent( title.c_str(), filter_list.c_str(), bd, &outPath, parent_win );

    std::string fn;
    if ( result == NFD_OKAY ) {
        fn = outPath;
        free(outPath);
        cancelled = false;
    }
    else if ( result == NFD_CANCEL ) {
        cancelled = true;
    } else {
        h->error(asprintf("NFD: %s\n", NFD_GetError()));
        cancelled = true;
    }

    return fn;
}

std::string WebWireStandardDialogs::getDirectoryDialog(WebWireHandler *h, WebUIWindow *win,
                                                       std::string title, std::string base_dir,
                                                       bool &cancelled)
{
    nfdchar_t *outPath = NULL;
    nfd_parent_window_data_ptr_t parent_win = static_cast<nfd_parent_window_data_ptr_t>(win->nativeHandle());

    base_dir = correctPath(base_dir);
    const char *bd = (base_dir == "") ? nullptr : base_dir.c_str();

    nfdresult_t result = NFD_PickFolderWithParent( title.c_str(), bd, &outPath, parent_win );

    std::string fn;
    if ( result == NFD_OKAY ) {
        fn = outPath;
        free(outPath);
        cancelled = false;
    }
    else if ( result == NFD_CANCEL ) {
        cancelled = true;
    } else {
        h->error(asprintf("NFD: %s\n", NFD_GetError()));
        cancelled = true;
    }

    return fn;}

WebWireStandardDialogs::WebWireStandardDialogs(Object_t *parent)
    : Object_t(parent)
{

}
