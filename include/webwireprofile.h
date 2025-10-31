#ifndef WEBWIREPROFILE_H
#define WEBWIREPROFILE_H

#include "object_t.h"
#include <string>

class WebWireHandler;

typedef enum {
    before_load = 1,
    after_load
} ScriptWhen_t;


class Script_t
{
private:
    ScriptWhen_t _when;
    std::string  _script;
    std::string  _name;
public:
    void setName(const std::string &n) { _name = n; }
    void setSourceCode(const std::string &c) { _script = c; }

public:
    std::string name() { return _name; }
    std::string code() { return _script; }

public:
    Script_t() { _when = after_load; }
};

#define WEB_WIRE_PROFILE_WORLD_ID 425542

class WebWireProfile : public Object_t
{
private:
    std::string _profile_name;

private:
    std::string _set_html_name;
    std::string _get_html_name;
    std::string _set_attr_name;
    std::string _get_attr_name;
    std::string _get_attrs_name;
    std::string _del_attr_name;
    std::string _add_style_name;
    std::string _set_style_name;
    std::string _get_style_name;
    std::string _set_css_name;
    std::string _get_elements_name;

private:
    int _world_id;
    std::string _css;
    Script_t _css_script;

    std::list<Script_t> _scripts;

private:
    void exec(WebWireHandler *h, int win, const std::string &name, const std::string &js);
    void exec(WebWireHandler *h, int win, const std::string &name, const std::string &js, bool &ok, std::string &result);

public:
    explicit WebWireProfile(const std::string &name, const std::string &default_css, Object_t *parent = nullptr);

public:
    void decUsage();
    void incUsage();
    int  usage();
    std::string profileName();
    std::string scriptsTag();

public:
    void set_html(WebWireHandler *h, int win, const std::string &element_id, const std::string &html, bool fetch);
    std::string get_html(WebWireHandler *h, int win, const std::string &element_id, bool &ok);

    void set_attr(WebWireHandler *h, int win, const std::string &element_id, const std::string &attr, const std::string &val);
    std::string get_attr(WebWireHandler *h, int win, const std::string &element_id, const std::string &attr, bool &ok);
    std::string get_attrs(WebWireHandler *h, int win, const std::string &element_id, bool &ok);
    void del_attr(WebWireHandler *h, int win, const std::string &element_id, const std::string &attr);

    void add_style(WebWireHandler *h, int win, const std::string &element_id, const std::string &style);
    void set_style(WebWireHandler *h, int win, const std::string &element_id, const std::string &style);
    std::string get_style(WebWireHandler *h, int win, const std::string &element_id, bool &ok);

    void set_css(WebWireHandler *h, int win, const std::string &css);

    std::string get_elements(WebWireHandler *h, int win, const std::string &selector, bool &ok);
};

#endif // WEBWIREPROFILE_H
