#include "webwirestandarddialogs.h"
#include "webuiwindow.h"
#include "webwirehandler.h"
#include "json.h"

#include <regex>

extern "C" {
#include <nfd.h>
}

#include <stdlib.h>

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

typedef struct {
    const char *evt_name;
    WebWireHandler *h;
    int id;
    int win_id;
} OF_Struct;

static int next_of_handle = 0;

static int nextHandle()
{
    int handle = ++next_of_handle;
    if (handle > 10000000) {
        handle = 1;
        next_of_handle = 1;
    }
    return handle;
}

static void open_dialogs_cb(nfdresult_t r, const char *data, void *user_data)
{
    OF_Struct *ofs = reinterpret_cast<OF_Struct *>(user_data);
    WebWireHandler *h = ofs->h;
    const char *evt_name = ofs->evt_name;
    int handle = ofs->id;
    int win_id = ofs->win_id;
    free(ofs);
    const char *fs = data;
    if (data == NULL) { fs = ""; }
    std::string dir = fs;
    dir = json_escape(dir);
    std::string e = asprintf("%s:%d:{ \"evt\": \"%s\", \"handle\":%d, \"choosen\": %s, \"dir\":\"%s\" }",
                             evt_name, win_id,
                             evt_name, handle, (r == NFD_OKAY) ? "true" : "false", dir.c_str()
                             );
    if (data != NULL) { free(reinterpret_cast<void *>(const_cast<char *>(data))); }
    h->evt(e);
    h->msg(NFD_GetError());
}

int WebWireStandardDialogs::openFileDialog(WebWireHandler *h, WebUIWindow *win,
                                           std::string title, std::string base_dir, PathFilterList filters)
{
    nfdchar_t *outPath = NULL;
    nfd_parent_window_data_ptr_t parent_win = static_cast<nfd_parent_window_data_ptr_t>(win->nativeHandle());

    std::string filter_list = makeFilters(filters);
    base_dir = correctPath(base_dir);
    const char *bd = (base_dir == "") ? nullptr : base_dir.c_str();

    OF_Struct *ofs = reinterpret_cast<OF_Struct *>(malloc(sizeof(OF_Struct)));
    if (ofs == NULL) { return 0; }

    int handle = nextHandle();

    ofs->h = h;
    ofs->id = handle;
    ofs->evt_name = "file-open";

    nfdresult_t result = NFD_OpenDialogWithParent( title.c_str(), filter_list.c_str(), bd, &outPath, parent_win,
                                                  open_dialogs_cb,
                                                  reinterpret_cast<void *>(ofs)
                                                  );

    if (result == NFD_RUNS_ASYNC) {
        return handle;
    } else {
        free(ofs);
        return 0;
    }
}

int WebWireStandardDialogs::saveFileDialog(WebWireHandler *h, WebUIWindow *win,
                                           std::string title, std::string base_dir, PathFilterList filters)
{
    nfdchar_t *outPath = NULL;
    nfd_parent_window_data_ptr_t parent_win = static_cast<nfd_parent_window_data_ptr_t>(win->nativeHandle());

    std::string filter_list = makeFilters(filters);
    base_dir = correctPath(base_dir);
    const char *bd = (base_dir == "") ? nullptr : base_dir.c_str();

    OF_Struct *ofs = reinterpret_cast<OF_Struct *>(malloc(sizeof(OF_Struct)));
    if (ofs == NULL) { return 0; }

    int handle = nextHandle();

    ofs->h = h;
    ofs->id = handle;
    ofs->evt_name = "file-save";

    nfdresult_t result = NFD_SaveDialogWithParent( title.c_str(), filter_list.c_str(), bd, &outPath, parent_win,
                                                   open_dialogs_cb,
                                                   reinterpret_cast<void *>(ofs)
                                                  );

    if (result == NFD_RUNS_ASYNC) {
        return handle;
    } else {
        free(ofs);
        return 0;
    }
}

int WebWireStandardDialogs::getDirectoryDialog(WebWireHandler *h, WebUIWindow *win,
                                                       std::string title, std::string base_dir,
                                                        int *a_result, std::string &dir_out
                                                       )
{
    nfdchar_t *outPath = NULL;
    nfd_parent_window_data_ptr_t parent_win = static_cast<nfd_parent_window_data_ptr_t>(win->nativeHandle());

    *a_result = -1;

    base_dir = correctPath(base_dir);
    const char *bd = (base_dir == "") ? nullptr : base_dir.c_str();

    OF_Struct *ofs = reinterpret_cast<OF_Struct *>(malloc(sizeof(OF_Struct)));
    if (ofs == NULL) { return 0; }

    int handle = nextHandle();

    ofs->h = h;
    ofs->id = handle;
    ofs->win_id = win->id();
    ofs->evt_name = "choose-dir";

#ifdef WIN32
    if (STDDLG_WIN_USE_THREADS) {
        nfdresult_t result = NFD_PickFolderWithParent( title.c_str(), bd, &outPath, parent_win,
                                                      open_dialogs_cb,
                                                      reinterpret_cast<void *>(ofs)
                                                      );
        return handle;
    } else {
        nfdresult_t result = NFD_PickFolderWithParent( title.c_str(), bd, &outPath, parent_win,
                                                      NULL,
                                                      reinterpret_cast<void *>(ofs)
                                                      );
        *a_result = static_cast<int>(result);
        if (result == NFD_OKAY) {
            dir_out = *outPath;
            free(outPath);
        }
        free(ofs);
        return handle;
    }
#else
    nfdresult_t result = NFD_PickFolderWithParent( title.c_str(), bd, &outPath, parent_win,
                                                   open_dialogs_cb,
                                                   reinterpret_cast<void *>(ofs)
                                                 );

    if (result == NFD_RUNS_ASYNC) {
        return handle;
    } else {
        free(ofs);
        return 0;
    }
#endif
}

WebWireStandardDialogs::WebWireStandardDialogs(Object_t *parent)
    : Object_t(parent)
{

}
